#include "Leximax_encoder.h"
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include "types.hh"
#include <zlib.h>
#include "fmtutils.hh"

int Leximax_encoder::call_solver(IntVector &tmp_model, std::string &file_name)
{
    stringstream scommand;
    const string output_filename = file_name + ".out";
    scommand << m_solver_command << " ";
    if (m_pbo)
        scommand << "-formula=1 ";
    scommand << file_name << " > " << output_filename << " 2> solver_error.txt";
    const string command = scommand.str();
    const int retv = system (command.c_str());
    //std::cerr << "# " <<  "external command finished with exit value " << retv << '\n';
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
        remove(file_name.c_str());
        remove(output_filename.c_str());
        remove("solver_error.txt");
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

int Leximax_encoder::solve_maxsat(int i, IntVector &tmp_model)
{
    std::string input_name = m_input_files;
    input_name += "_" + to_string(i) + ".wcnf";
    ofstream output(input_name.c_str());
    // prepare input for the solver
    size_t weight = m_soft_clauses.size() + 1;
    output << "p wcnf " << m_id_count << " " << m_constraints.size() << " " << weight << '\n';
    // print hard clauses
    write_clauses(output, m_constraints, weight);
    // print soft clauses
    write_clauses(output, m_soft_clauses, 1);
    output.close();
    // call the solver
    return call_solver(tmp_model, input_name);
}

void Leximax_encoder::write_pbconstraint(BasicClause *cl, ostream& output) {
    LINT num_negatives(0);
    for (LINT literal : *cl) {
        bool sign = literal > 0;
        if (!sign)
            ++num_negatives;
        output << (sign ? "+1" : "-1") << m_multiplication_string << "x" << (sign ? literal : -literal) << " ";
    }
    output << " >= " << 1 - num_negatives << ";\n";
}

void Leximax_encoder::write_atmost_pb(int i, ostream &output)
{
    for (LINT var : m_relax_vars) {
        output << "+1" << m_multiplication_string << "x" << var << " ";
    }
    output << " <= " << i << ";\n";
}

void Leximax_encoder::write_sum_equals_pb(int i, ostream &output)
{
    for (LINT var : m_relax_vars) {
        output << "+1" << m_multiplication_string << "x" << var << " ";
    }
    output << " = " << i << ";\n";
}

int Leximax_encoder::solve_pbo(int i, IntVector&  tmp_model)
{
    std::string input_name = m_input_files;
    input_name += "_" + to_string(i) + ".opb";
    ofstream output(input_name.c_str());
    // prepare input for the solver
    output << "* #variable= " << m_id_count << " #constraint= " << m_constraints.size() + 1 << '\n'; // + 1 because of card. const.
    if (m_soft_clauses.size() > 0) {// print minimization function
        output << "min:";
        for (BasicClause *cl : m_soft_clauses) {
            LINT soft_var = -(*(cl->begin())); // cl is unitary clause
            output << " " << "+1" << m_multiplication_string << "x" << soft_var;
        }
        output << ";\n";
    }
    // print all constraints except for cardinality constraint
    for (BasicClause *cl : m_constraints) {
        write_pbconstraint(cl, output);
    }
    if (i == m_num_objectives - 1)
        write_sum_equals_pb(1, output); // in the last iteration print =1 cardinality constraint
    else if (i != 0)
        write_atmost_pb(i, output); // in other iterations print at most i constraint  
    output.close();
    // call the solver
    return call_solver(tmp_model, input_name);
}

