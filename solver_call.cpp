#include "Leximax_encoder.h"
#include <fstream>
#include <sstream>
#include <time.h>

int Leximax_encoder::solve_maxsat()//TODO
{
    // write input file of the solver
    
    // call solver and write its output to a file
   
    return 0;
}

void write_pbconstraint(const BasicClause *cl, ostream& output) {
    for (const LINT literal : *cl) {
        const bool sign = literal > 0;
        output << (sign ? "+1" : "-1") << m_multiplication_string << "x" << (sign ? literal : -literal) << " ";
    }
    output << " >= " << 0 << ";\n";
}

void Leximax_encoder::write_atmost_pb(int i, ostream &output)
{
    for (const LINT var : m_relax_vars) {
        output << "+1" << m_multiplication_string << "x" << var << " ";
    }
    output << " <= " << i << ";\n";
}

int Leximax_encoder::solve_pbo(int i)//TODO
{
    time_t stamp = time(NULL);
    std::string temporary_directory("tmp");
    stringstream strstr;
    strstr << temporary_directory << "/" << "o" << stamp << "_" << i << ".opb";
    m_input_file_name = strstr.str();
    ofstream output(m_input_file_name.c_str());
    // prepare input for the solver
    output << "* #variable= " << m_id_count << " #constraint= " << m_constraints.size() + 1 << endl; // + 1 because of card. const.
    if (m_soft_clauses.size() > 0) {// print minimization function
        output << "min:";
        for (BasicClause *cl : m_soft_clauses) {
            LINT literal = *(cl->begin()); // cl is unitary clause
            const bool sign = literal > 0;
            output << " " << (sign ? "+1" : "-1") << m_multiplication_string << "x" << (sign ? literal : -literal);
        }
        output << ";\n"; // in packup there was << std::endl instead of \n
    }
    // print all constraints except for at most i
    for (BasicClause *cl : m_constraints) {
        write_pbconstraint(*cl, output);
    }
    // print at most i constraint
    write_atmost_pb(i);
    output.close();
    // call the solver
    stringstream scommand;
    const string output_filename = input_file_name + ".out";
    scommand << solver_command << " " << input_file_name << " >" << output_filename;
    const string command = scommand.str();
    const int retv = system (command.c_str());
    cerr << "# " <<  "external command finished with exit value " << retv << endl;
    gzFile of=gzopen(output_filename.c_str(), "rb");
    assert(of!=NULL);//TODO
    StreamBuffer r(of);
    bool sat=false;
    tmp_model.resize(static_cast<size_t>(_id_manager.top_id()+1),0);
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
                // if ( model.size()<=(size_t)l )
                //cerr << "# " << l << " " << (sign ? l : -l) << endl;
                assert(tmp_model.size()>(size_t)l);
                tmp_model[l] = (sign ? l : -l);
            }
            assert (*r=='\n');
            ++r; // skip '\n'
        }
    }
    if (!sat) tmp_model.clear();
    if (!leave_temporary_files) {
        remove(input_file_name.c_str());
        remove(output_filename.c_str());
    }

    return retv;
    // write input file of the solver
    
    // call solver and write its output to a file
    
    return 0;
}

