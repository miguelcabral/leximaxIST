#include <Leximax_encoder.h>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <zlib.h>
#include <fmtutils.hh>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring> // for strerror()

void Leximax_encoder::read_gurobi_output(const std::string &output_filename)
{
    // TODO
}

void Leximax_encoder::read_glpk_output(const std::string &output_filename)
{
    // TODO
}

void Leximax_encoder::read_lpsolve_output(const std::string &output_filename)
{
    // TODO
}

void Leximax_encoder::read_scip_output(const std::string &output_filename)
{
    // TODO
}

void Leximax_encoder::read_cbc_output(const std::string &output_filename)
{
    // TODO
}

void Leximax_encoder::read_cplex_output(const std::string &output_filename)
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

void Leximax_encoder::read_solver_output(const std::string &output_filename)
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
        if (m_lp_solver == "cplex")
            read_cplex_output(output_filename);
        else if (m_lp_solver == "gurobi")
            read_gurobi_output(output_filename);
        else if (m_lp_solver == "glpk")
            read_glpk_output(output_filename);
        else if (m_lp_solver == "scip")
            read_scip_output(output_filename);
        else if (m_lp_solver == "cbc")
            read_cbc_output(output_filename);
        else if (m_lp_solver == "lpsolve")
            read_lpsolve_output(output_filename);
    }
}

int Leximax_encoder::split_solver_command(const std::string &command, std::vector<std::string> &command_split)
{
    size_t pos (0);
    while (pos < command.length()) {
        // skip whitespace until next piece of text
        while (command[pos] == ' ' && pos < command.length())
            pos++;
        if (pos == command.length())
            break;
        // pos is where the piece of text starts
        size_t found (command.find_first_of(" \"\'", pos));
        while (found != std::string::npos && command[found] != ' ') {
            char quote (command[found]);
            size_t quote_pos (found);
            found++;
            while (command[found] != quote && found < command.length())
                found++;
            if (command[found] != quote) {
                std::string msg ("Can not parse external solver command - missing closing quotation mark\n");
                msg += command + "\n";
                for (int i (0); i < quote_pos; ++i) {
                    msg += " ";
                }
                msg += "^";
                print_error_msg(msg);
                return -1;
            }
            else
                found++;
            found = command.find_first_of(" \"\'", found);
        }
        // I just found the end of the piece of text
        command_split.push_back(command.substr(pos, found - pos)/*.c_str()*/);
        pos = found;
    }
    if (command_split.empty()) {
        print_error_msg("Empty external solver command");
        return -1;
    }
    return 0;
}

int Leximax_encoder::call_solver(const std::string &input_filename)
{
    const std::string output_filename = input_filename + ".out";
    const std::string error_filename = input_filename + ".err";
    std::string command (m_solver_command + " ");
    if (m_solver_format == "lp") { // TODO: set CPLEX parameters : number of threads, tolerance, etc.
        if (m_lp_solver == "cplex")
            command += "-c \"read " + input_filename + "\" \"optimize\" \"display solution variables -\"";
        if (m_lp_solver == "cbc")
            command += input_filename + " solve solution $";
    }
    else
        command += input_filename;
    pid_t pid (fork());
    if (pid == -1) {
        std::string errmsg (strerror(errno));
        print_error_msg("Can't fork process: " + errmsg);
        return -1;
    }
    if (pid == 0) { // child process
        // open output_filename and error_filename
        std::ofstream output_stream(output_filename);
        std::ofstream error_stream(error_filename);
        // redirect std output to output_filename and std error to error_filename
        std::cout.rdbuf(output_stream.rdbuf());
        std::cerr.rdbuf(error_stream.rdbuf());
        // convert command to vector of strings (split by whitespace)
        std::vector<std::string> command_split;
        if (split_solver_command(command, command_split) == -1)
            return -1;
        command_split.clear();
        command_split.push_back("grep");
        command_split.push_back("'packup'");
        command_split.push_back("/home/miguelcabral/thesis/old_packup/Makefile");
        // convert to array for execv function
        std::vector<char*> args(command_split.size() + 1, new char[30]{}); // memory leak - no problems because child process dies afterward?
        //char **args (new char*[command_split.size()]);
        for (size_t i (0); i < command_split.size(); ++i) {
            size_t length (command_split[i].copy(args[i], command_split[i].size()));
            args[i][length] = '\0';
            std::cerr << "i="<< i << std::endl;
            std::cerr << args[i] << std::endl;
        }
        args[command_split.size()] = nullptr;
            /*std::cout << "command_split: ";
    for (std::string &s : command_split)
        std::cout << s << ", ";
    std::cout << std::endl;*/
        // call solver
        std::string ola ("/bin/grep");
        if (execv(/*args[0]*/ola.c_str(), args.data()) == -1) {
            std::string errmsg (strerror(errno));
            print_error_msg("Something went wrong with the external solver: " + errmsg);
            return -1;
        }
    }
    // parent process - store child pid for signal handling
    m_child_pid = pid;
    // waitpid for child process
    int pid_status;
    if (waitpid(pid, &pid_status, 0) == -1) {
        std::string errmsg (strerror(errno));
        print_error_msg("Error waiting for child process: " + errmsg);
        return -1;
    }
    if (WEXITSTATUS(pid_status)) {
        std::string errmsg (strerror(errno));
        print_error_msg("The external solver finished with non-zero error status: " + errmsg);
        return -1;
    }    
    read_solver_output(output_filename);
    if (!m_leave_temporary_files) {
        remove(input_filename.c_str());
        remove(output_filename.c_str());
        remove(error_filename.c_str());
    }
    // set to zero, i.e. no external solver is currently running
    m_child_pid = 0;
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

void Leximax_encoder::write_atmost_lp(int i, std::ostream &output)
{
    bool first_iteration (true);
    for (LINT var : m_relax_vars) {
        if (first_iteration) {
            output << 'x' << var;
            first_iteration = false;
        }
        else
            output << " + " << 'x' << var;
    }
    output << " <= " << i << '\n';
}

void Leximax_encoder::write_sum_equals_pb(int i, std::ostream &output)
{
    for (LINT var : m_relax_vars) {
        output << "+1" << m_multiplication_string << "x" << var << " ";
    }
    output << " = " << i << ";\n";
}

void Leximax_encoder::write_sum_equals_lp(int i, std::ostream &output)
{
    bool first_iteration (true);
    for (LINT var : m_relax_vars) {
        if (first_iteration) {
            output << 'x' << var;
            first_iteration = false;
        }
        else
            output << " + " << 'x' << var;
    }
    output << " = " << i << '\n';
}

int Leximax_encoder::solve_pbo(int i)
{
    std::string input_name (m_input_name);
    input_name += "_" + std::to_string(i) + ".opb";
    std::ofstream output(input_name.c_str());
    // prepare input for the solver
    output << "* #variable= " << m_id_count;
    if (i == 0)
        output << " #constraint= " << m_constraints.size() << '\n';
    else
        output << " #constraint= " << m_constraints.size() + 1 << '\n'; // + 1 because of card. const.
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

int Leximax_encoder:: solve_lp(int i)
{
    std::string input_name (m_input_name);
    input_name += "_" + std::to_string(i) + ".lp";
    std::ofstream output(input_name.c_str());
    // prepare input for the solver
    output << "Minimize\n";
    output << " obj: ";
    if (m_soft_clauses.size() > 0) {// print minimization function
        size_t nb_vars_in_line (0);
        for (size_t j (0); j < m_soft_clauses.size(); ++j) {
            Clause *cl (m_soft_clauses[j]);
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

int Leximax_encoder::external_solve(int i)
{
    if(m_solver_format == "wcnf")
        return solve_maxsat(i);
    else if(m_solver_format == "opb")
        return solve_pbo(i);
    else if(m_solver_format == "lp")
        return solve_lp(i);
    else {
        std::string msg ("The external solver format entered: '" + m_solver_format + "' is not valid\n");
        msg += "Valid external solver formats: 'wcnf' 'opb' 'lp'";
        print_error_msg(msg);
        return -1;
    }
}
