#include "Leximax_encoder.h"
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include "old_packup/types.hh"
#include <zlib.h>
#include "old_packup/fmtutils.hh"
#include <algorithm>

bool comp(LINT i, LINT j) { return (i > j); }

void Leximax_encoder::collect_sorted_obj_vecs(std::string &output_filename, std::forward_list<std::vector<LINT>> &sorted_obj_vectors)
{
    gzFile of = gzopen(output_filename.c_str(), "rb");
    assert(of!=NULL);
    StreamBuffer r(of);
    // find the max id in m_objectives
    std::vector<LINT>* last_objective = m_objectives[m_num_objectives - 1];
    size_t max_id = last_objective->at(last_objective->size() - 1);
    std::vector<LINT> model(max_id + 1, 0);
    std::vector<LINT> obj_vec(m_num_objectives, 0);
    while (*r != EOF) {
        if (*r != 'v') {// ignore all the other lines
            skipLine(r);
        } else {
            ++r; // skip 'v'
            while ( (*r != '\n')  && (*r != EOF)  && (*r != '\r') ) {
                skipTrueWhitespace(r);
                const bool sign = (*r) != '-';
                if ((*r == '+') || (*r == '-')) ++r;
                if ((*r == 'x')) ++r;
                if (*r < '0' || *r > '9')
                    break;
                const LINT l = parseInt(r);
                assert(model.size()>(size_t)l);
                model[l] = (sign ? l : -l);
            }
            assert(*r=='\n');
            ++r; // skip '\n'
            // determine objective vector for current model
            for (int j(0); j < m_num_objectives; ++j) {
                std::vector<LINT> *objective = m_objectives[j];
                size_t score(0);
                for (LINT var : *objective) {
                    if (model[var] > 0)
                        ++score;
                }
                obj_vec[j] = score;
            }
            // sort objective vector in non-increasing order
            std::sort(obj_vec.begin(), obj_vec.end(), comp);
            // add the objective vector to the collection
            sorted_obj_vectors.push_front(obj_vec);
        }
    }
}

void Leximax_encoder::brute_force_optimum(std::vector<LINT> &optimum, std::forward_list<std::vector<LINT>> &sorted_obj_vectors)
{
    assert(!sorted_obj_vectors.empty());
    std::vector<LINT> &first_vec = sorted_obj_vectors.front();
    // set up - optimum starts as the first vector
    for (int j(0); j < m_num_objectives; ++j)
        optimum[j] = first_vec[j];
    for (int pos(0); pos < m_num_objectives; ++pos) {
        for (std::vector<LINT> &vec : sorted_obj_vectors) {
            // check if vec is less than current optimum
            bool ignore = false;
            for (int previous_pos(0); previous_pos < pos; ++previous_pos) {
                if (vec[previous_pos] != optimum[previous_pos]) {
                    ignore = true;
                    break;
                }
            }
            if (!ignore && vec[pos] < optimum[pos]) {
                // update optimum
                for (int k(pos); k < m_num_objectives; ++k) {
                    optimum[k] = vec[k];
                }
            }
        }
    }
}

void Leximax_encoder::verify()
{
    if (!m_sat) {
        std::cerr << "OK, UNSAT\n";
        return;
    }
    // call pienum
    stringstream command_stream;
    std::string output_filename = m_pienum_file_name + ".out";
    command_stream << "./pienum -p ";
    command_stream << m_pienum_file_name << " > " << output_filename;
    const std::string command = command_stream.str();
    const int retv = system (command.c_str());
    //std::cerr << "# " <<  "pienum finished with exit value " << retv << '\n';
    // open output file of pienum and compute optimum objective vector sorted in non-increasing order.
    std::forward_list<std::vector<LINT>> sorted_obj_vectors;
    std::vector<LINT> pienum_opt(m_num_objectives, 0);
    collect_sorted_obj_vecs(output_filename, sorted_obj_vectors);
    brute_force_optimum(pienum_opt, sorted_obj_vectors);
    // compare the two results and return OK if optimum coincides and Problems! otherwise
    bool ok(true);
    for (int j(0); j < m_num_objectives; ++j) {
        if (pienum_opt[j] != m_optimum[j]) {
            ok = false;
            break;
        }
    }
    if (ok)
        std::cerr << "OK\n";
    else {
        std::cerr << "Problems on " << m_input_files << '\n';
        std::cerr << "Brute force: ";
        for (int j(0); j < m_num_objectives; ++j)
            std::cerr << pienum_opt[j] << ' ';
        std::cerr << "; Tool: ";
        for (int j(0); j < m_num_objectives; ++j)
            std::cerr << m_optimum[j] << ' ';
        std::cerr << '\n';
    }
}
