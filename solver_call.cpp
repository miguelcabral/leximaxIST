#include "Leximax_encoder.h"
#include <fstream>
#include <sstream>
#include <time.h>
#include <stdlib.h>
#include "old_packup/types.hh"
#include <zlib.h>
#include "old_packup/fmtutils.hh"

int Leximax_encoder::call_solver(IntVector &tmp_model)
{
    stringstream scommand;
    const string output_filename = m_input_file_name + ".out";
    scommand << m_solver_command << " " << m_input_file_name << " >" << output_filename;
    const string command = scommand.str();
    const int retv = system (command.c_str());
    std::cerr << "# " <<  "external command finished with exit value " << retv << '\n';
    gzFile of = gzopen(output_filename.c_str(), "rb");
    assert(of!=NULL);
    StreamBuffer r(of);
    bool sat = false;
    tmp_model.resize(static_cast<size_t>(m_id_count + 1), 0);
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
                assert(tmp_model.size()>(size_t)l);
                tmp_model[l] = (sign ? l : -l);
            }
            assert (*r=='\n');
            ++r; // skip '\n'
        }
    }
    if (!sat) tmp_model.clear();
    if (!m_leave_temporary_files) {
        remove(m_input_file_name.c_str());
        remove(output_filename.c_str());
    }
    return retv;
}

void write_clauses(ostream &output, BasicClauseSet &clauses, size_t weight)
{
    for (BasicClause *cl : clauses) {
        output << weight << " ";
        for (LINT lit : *cl)
            output << lit << " "; 
        output << "0\n";
    }
}

int Leximax_encoder::solve_maxsat(int i, IntVector &tmp_model)//TODO
{
    time_t stamp = time(NULL);
    std::string temporary_directory("tmp");
    stringstream strstr;
    strstr << temporary_directory << "/" << "o" << stamp << "_" << i << ".wcnf";
    m_input_file_name = strstr.str();
    ofstream output(m_input_file_name.c_str());
    // prepare input for the solver
    size_t weight = m_soft_clauses.size() + 1;
    output << "p wcnf " << m_id_count << " " << m_constraints.size() << " " << weight << '\n';
    // print hard clauses
    write_clauses(output, m_constraints, weight);
    // print soft clauses
    write_clauses(output, m_soft_clauses, 1);
    output.close();
    // call the solver
    return call_solver(tmp_model);
}

void Leximax_encoder::write_pbconstraint(BasicClause *cl, ostream& output) {
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

int Leximax_encoder::solve_pbo(int i, IntVector&  tmp_model)
{
    time_t stamp = time(NULL);
    std::string temporary_directory("tmp");
    stringstream strstr;
    strstr << temporary_directory << "/" << "o" << stamp << "_" << i << ".opb";
    m_input_file_name = strstr.str();
    ofstream output(m_input_file_name.c_str());
    // prepare input for the solver
    output << "* #variable= " << m_id_count << " #constraint= " << m_constraints.size() + 1 << '\n'; // + 1 because of card. const.
    if (m_soft_clauses.size() > 0) {// print minimization function
        output << "min:";
        for (BasicClause *cl : m_soft_clauses) {
            LINT literal = *(cl->begin()); // cl is unitary clause
            const bool sign = literal > 0;
            output << " " << (sign ? "+1" : "-1") << m_multiplication_string << "x" << (sign ? literal : -literal);
        }
        output << ";\n";
    }
    // print all constraints except for at most i
    for (BasicClause *cl : m_constraints) {
        write_pbconstraint(cl, output);
    }
    // print at most i constraint
    write_atmost_pb(i, output);
    output.close();
    // call the solver
    return call_solver(tmp_model);
}

