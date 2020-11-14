#include <Leximax_encoder.h>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <zlib.h>
#include <fmtutils.hh>
#include <errno.h>
#include <sys/wait.h>

void Leximax_encoder::read_cplex_output(std::string &output_filename)
{
    gzFile of = gzopen(output_filename.c_str(), "rb");
    assert(of!=NULL);
    StreamBuffer r(of);
    bool sat = false;
    m_solution.resize(static_cast<size_t>(m_id_count + 1), 0);
    // set all variables to false, because we only get the variables that are true
    for (size_t v (1); v < m_id_count + 1; ++v)
        m_solution[v] = -v;
    while (*r != EOF) {
        if (*r != 'C') {// ignore all the other lines
            skipLine(r);
        } else {
            // check if the line is 'CPLEX> Incumbent solution'
            std::string line;
            for (int i (0); i < 25 && (*r != '\n') && (*r != EOF) && (*r != '\r'); ++i) {
                line.push_back(*r);
                ++r;
            }
            if (line == "CPLEX> Incumbent solution") {
                sat=true;
                while (*r != EOF) {
                    if (*r != 'x')
                        skipLine(r);
                    else {
                        ++r;
                        const LINT l = parseInt(r);
                        assert(m_solution.size()>(size_t)l);
                        m_solution[l] = l;
                    }
                }
            }
        }
    }
    if (!sat) m_solution.clear();    
}

void Leximax_encoder::read_solver_output(std::string &output_filename)
{
    if (m_solver_format == "wcnf" || m_solver_format == "opb") {
        gzFile of = gzopen(output_filename.c_str(), "rb");
        assert(of!=NULL);
        StreamBuffer r(of);
        bool sat = false;
        m_solution.resize(static_cast<size_t>(m_id_count + 1), 0);
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
                    assert(m_solution.size()>(size_t)l);
                    m_solution[l] = (sign ? l : -l);
                }
                assert (*r=='\n');
                ++r; // skip '\n'
            }
        }
        if (!sat) m_solution.clear();
    }
    else if (m_solver_format == "lp") {
        switch (m_lp_solver) {
            case "cplex":
                read_cplex_output(output_filename);
                break;
            case "gurobi":
                read_gurobi_output(output_filename);
                break;
            case "glpk":
                read_glpk_output(output_filename);
                break;
            case "scip":
                read_scip_output(output_filename);
                break;
            case "cbc":
                read_cbc_output(output_filename);
                break;
            case "lpsolve":
                read_lpsolve_output(output_filename);
                break;
        }
    }
}

int Leximax_encoder::call_solver(std::string &file_name)
{
    // TODO: change this to fork and exec and waitpid; Store the child pid -> member variable
    // , to send signal to the child in function terminate
    std::stringstream scommand;
    const std::string output_filename = file_name + ".out";
    scommand << m_solver_command << " ";
    if (m_solver_format == "lp" && m_lp_solver == "cplex") {
        scommand << "-c \"read " << file_name << "\" \"optimize\" \"display solution variables -\" \"quit\"";
    }
    else
        scommand << file_name;
    //scommand << " > " << output_filename << " 2> solver_error.txt";
    const std::string command = scommand.str();
    pid_t pid (fork());
    if (pid == -1) {
        print_error_msg("Can't fork process: " + strerror(errno));
        return -1;
    }
    if (pid == 0) {
        // child process - run external solver
        // redirect std output to output_filename and std error to solver_error.txt
        
        // convert command to vector of strings (split by whitespace)
        
        // execute execl
        execl();
    }
    // parent process - store child pid for signal handling
    m_child_pid = pid;
    // waitpid for child process
    int pid_status;
    if (waitpid(pid, &pid_status, 0) == -1) {
        print_error_msg("Error waiting for child process: " + strerror(errno));
        return -1;
    }
    if (WEXITSTATUS(pid_status)) {
        print_error_msg("The external solver finished with non-zero error status");
        return -1;
    }    
    read_solver_output(output_filename);
    if (!m_leave_temporary_files) {
        remove(file_name.c_str());
        remove(output_filename.c_str());
        remove("solver_error.txt");
    }
    return 0;
}

void write_clauses(std::ostream &output, std::vector<Clause*> &clauses, size_t weight)
{
    for (Clause *cl : clauses) {
        output << weight << " ";
        for (LINT lit : *cl)
            output << lit << " "; 
        output << "0\n";
    }
}

int Leximax_encoder::solve_maxsat(int i)
{
    std::string input_name (m_input_name);
    input_name += "_" + std::to_string(i) + ".wcnf";
    std::ofstream output(input_name.c_str());
    // prepare input for the solver
    size_t weight = m_soft_clauses.size() + 1;
    output << "p wcnf " << m_id_count << " " << m_constraints.size() << " " << weight << '\n';
    // print hard clauses
    write_clauses(output, m_constraints, weight);
    // print soft clauses
    write_clauses(output, m_soft_clauses, 1);
    output.close();
    // call the solver
    return call_solver(input_name);
}

void Leximax_encoder::write_pbconstraint(Clause *cl, std::ostream &output) {
    LINT num_negatives(0);
    for (LINT literal : *cl) {
        bool sign = literal > 0;
        if (!sign)
            ++num_negatives;
        output << (sign ? "+1" : "-1") << m_multiplication_string << "x" << (sign ? literal : -literal) << " ";
    }
    output << " >= " << 1 - num_negatives << ";\n";
}

void Leximax_encoder::write_lpconstraint(Clause *cl, std::ostream &output) {
    LINT num_negatives(0);
    size_t nb_vars_in_line (0);
    for (size_t j (0); j < cl->size(); ++j) {
        LINT literal (cl->at(j));
        bool sign = literal > 0;
        if (!sign)
            ++num_negatives;
        if (j == 0)
            output << 'x' << (sign ? literal : -literal);
        else
            output << (sign ? " + " : " - ") << 'x' << (sign ? literal : -literal);
        nb_vars_in_line++;
        if (nb_vars_in_line == 5) {
            output << '\n';
            nb_vars_in_line = 0;
        }
    }
    output << " >= " << 1 - num_negatives << '\n';
}

void Leximax_encoder::write_atmost_pb(int i, std::ostream &output)
{
    for (LINT var : m_relax_vars) {
        output << "-1" << m_multiplication_string << "x" << var << " ";
    }
    output << " >= " << -i << ";\n";
}

void Leximax_encoder::write_sum_equals_pb(int i, std::ostream &output)
{
    for (LINT var : m_relax_vars) {
        output << "+1" << m_multiplication_string << "x" << var << " ";
    }
    output << " = " << i << ";\n";
}

int Leximax_encoder::solve_pbo(int i)
{
    std::string input_name (m_input_name);
    input_name += "_" + std::to_string(i) + ".opb";
    std::ofstream output(input_name.c_str());
    // prepare input for the solver
    output << "* #variable= " << m_id_count << " #constraint= " << m_constraints.size() + 1 << '\n'; // + 1 because of card. const.
    if (m_soft_clauses.size() > 0) {// print minimization function
        output << "min:";
        for (Clause *cl : m_soft_clauses) {
            LINT soft_var = -(*(cl->begin())); // cl is unitary clause
            output << " " << "+1" << m_multiplication_string << "x" << soft_var;
        }
        output << ";\n";
    }
    // print all constraints except for cardinality constraint
    for (Clause *cl : m_constraints) {
        write_pbconstraint(cl, output);
    }
    if (i == m_num_objectives - 1 && m_num_objectives != 1)
        write_sum_equals_pb(1, output); // in the last iteration print =1 cardinality constraint
    else if (i != 0)
        write_atmost_pb(i, output); // in other iterations print at most i constraint  
    output.close();
    // call the solver
    return call_solver(input_name);
}

void Leximax_encoder:: solve_lp(int i)
{
    std::string input_name (m_input_name);
    input_name += "_" + std::to_string(i) + ".lp";
    std::ofstream output(input_name.c_str());
    // prepare input for the solver
    output << "Minimize\n";
    output << " obj: "
    if (m_soft_clauses.size() > 0) {// print minimization function
        size_t nb_vars_in_line (0);
        for (size_t j (0); j < m_soft_clauses.size(); ++j) {
            Clause *cl (m_soft_clauses[j])
            LINT soft_var = -(*(cl->begin())); // cl is unitary clause
            if (j == 0)
                output << 'x' << soft_var;
            else
                output << " + " << 'x' << soft_var;
            nb_vars_in_line++;
            if (nb_vars_in_line == 5) {
                output << '\n';
                nb_vars_in_line = 0;
            }
        }
        output << '\n';
    }
    output << "Subject To\n";
    // print all constraints except for cardinality constraint
    for (Clause *cl : m_constraints) {
        write_lpconstraint(cl, output);
    }
    if (i == m_num_objectives - 1 && m_num_objectives != 1)
        write_sum_equals_lp(1, output); // in the last iteration print =1 cardinality constraint
    else if (i != 0)
        write_atmost_lp(i, output); // in other iterations print at most i constraint  
    // print all variables after Binaries
    output << "Binaries\n";
    for (size_t j (1); j <= m_id_count; ++j)
        output << "x" << j << '\n';
    output << "End";
    output.close();
    // call the solver
    return call_solver(input_name);
}

void Leximax_encoder::external_solve(int i)
{
    switch(m_solver_format) {
        case "wcnf":
            solve_maxsat(i);
            break;
        case "opb":
            solve_pbo(i);
            break;
        case "lp":
            solve_lp(i);
            break;
    }
}
