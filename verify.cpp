#include "Leximax_encoder.h"
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include "old_packup/types.hh"
#include <zlib.h>
#include "old_packup/fmtutils.hh"

std::vector<LINT>* Leximax_encoder::compute_optimum(std::string &output_filename)
{
    gzFile of = gzopen(output_filename.c_str(), "rb");
    assert(of!=NULL);
    StreamBuffer r(of);
    bool sat = false;
    // find the max id in m_objectives
    std::vector<LINT>* last_objective = m_objectives[m_num_objectives - 1];
    size_t max_id = last_objective->at(last_objective->size() - 1);
    IntVector model(max_id + 1, 0);
    size_t optimum(0);
    while (*r != EOF) {
        if (*r != 'v') {// ignore all the other lines
            skipLine(r);
        } else {
            sat=true;
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
        }
    }
    if (!sat) tmp_model.clear();
    if (!m_leave_temporary_files) {
        remove(file_name.c_str());
        remove(output_filename.c_str());
    }
}

void Leximax_encoder::verify()
{
    // call pienum
    stringstream command_stream;
    size_t pos = m_input_files.find_first_of('.'); // CHANGE THIS!!! -> pienum must receive constraints with encoding of obj funcs
    const std::string input_to_pienum = m_input_files.substr(0, pos - 1);
    input_to_pienum += "_pienum.cnf";
    // write input file to pienum
    
    std::string output_filename = "all_models_" + m_input_to_pienum;
    command_stream << "pienum -p ";
    command_stream << input_to_pienum << " > " << output_filename;
    const std::string command = command_stream.str();
    const int retv = system (command.c_str());
    std::cerr << "# " <<  "pienum finished with exit value " << retv << '\n';
    // open output file of pienum and compute optimum objective vector sorted in non-increasing order.
    std::vector<LINT> *pienum_opt = compute_optimum(output_filename);
    // call leximax with input file hard.cnf and objective functions given in the cmd line args
    
    // open output file of leximax and read optimum obj vector sorted in non-increasing order.
    std::vector<LINT> *leximax_opt = read_optimum(output_filename)
    // compare the two results and return OK if optimum coincides and false Problems!
    compare(pienum_opt, leximax_opt);
    return 0;
}
