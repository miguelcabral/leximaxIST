#include "Leximax_encoder.h"
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include "old_packup/types.hh"
#include <zlib.h>
#include "old_packup/fmtutils.hh"

void Leximax_encoder::brute_force_optimum(std::string &output_filename, std::vector<LINT> &pienum_opt)
{
    gzFile of = gzopen(output_filename.c_str(), "rb");
    assert(of!=NULL);
    StreamBuffer r(of);
    // find the max id in m_objectives
    std::vector<LINT>* last_objective = m_objectives[m_num_objectives - 1];
    size_t max_id = last_objective->at(last_objective->size() - 1);
    IntVector model(max_id + 1, 0);
    size_t optimum(0);
    int obj_index(0);
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
                if (*r < '0' || *r > '9') break;
                const LINT l = parseInt(r);
                assert(model.size()>(size_t)l);
                model[l] = (sign ? l : -l);
            }
            assert (*r=='\n');
            ++r; // skip '\n'
            // determine score of current model
            optimum = 0;
            for (BasicClause *cl : m_soft_clauses) {
                LINT var = -(*(cl->begin()));
                if (model[var] > 0)
                    optimum++;
            }
            pienum_opt[obj_index] = optimum;
            ++obj_index;
        }
    }
}

void Leximax_encoder::verify()
{
    if (!m_sat)
        return;
    // call pienum
    stringstream command_stream;    
    std::string output_filename = "all_models_" + m_pienum_file_name;
    command_stream << "./pienum -p ";
    command_stream << m_pienum_file_name << " > " << output_filename;
    const std::string command = command_stream.str();
    const int retv = system (command.c_str());
    std::cerr << "# " <<  "pienum finished with exit value " << retv << '\n';
    // open output file of pienum and compute optimum objective vector sorted in non-increasing order.
    std::vector<LINT> pienum_opt(m_num_objectives, 0);
    brute_force_optimum(output_filename, pienum_opt);
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
