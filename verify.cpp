#include "Leximax_encoder.h"
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include "old_packup/types.hh"
#include <zlib.h>
#include "old_packup/fmtutils.hh"
#include <algorithm>

bool comp(LINT i, LINT j) { return (i > j) };

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
                std::cout << l << std::endl;
                assert(model.size()>(size_t)l);
                model[l] = (sign ? l : -l);
            }
            assert(*r=='\n')
            ++r; // skip '\n'
            // determine objective vector for current model
            for (int j(0); j < m_num_objectives; ++j) {
                size_t score(0);
                std::vector<LINT> *objective = m_objectives[j];
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

void Leximax_encoder::verify()
{
    if (!m_sat)
        return;
    // call pienum
    stringstream command_stream;
    size_t pos = m_pienum_file_name.find_first_of('.');
    std::string output_filename = "all_models_" + m_pienum_file_name.substr(0, pos) + ".out";
    command_stream << "./pienum -p ";
    command_stream << m_pienum_file_name << " > " << output_filename;
    const std::string command = command_stream.str();
    const int retv = system (command.c_str());
    std::cerr << "# " <<  "pienum finished with exit value " << retv << '\n';
    // open output file of pienum and compute optimum objective vector sorted in non-increasing order.
    std::forward_list<std::vector<LINT>> sorted_obj_vectors();
    std::vector<LINT> pienum_opt(m_num_objectives, 0);
    collect_sorted_obj_vecs(output_filename, sorted_obj_vectors);
    brute_force_optimum(pienum_opt, sorted_obj_vectors);
    
    // clean up
    if (!m_leave_temporary_files) {
        remove(m_pienum_file_name.c_str());
        remove(output_filename.c_str());
    }
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
    else
        std::cerr << "Problems!\n";
}
