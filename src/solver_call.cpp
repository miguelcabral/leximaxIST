#include <leximaxIST_Solver.h>
#include <leximaxIST_rusage.h>
#include <leximaxIST_parsing_utils.h>
#include <leximaxIST_printing.h>
#include <IpasirWrap.h>
#include <zlib.h>
#include <sys/wait.h>
#include <sys/resource.h> // for getrusage()
#include <unistd.h>
#include <assert.h>
#include <errno.h> // for errno
#include <stdlib.h> // exit, system
#include <stdio.h> // for fopen()
#include <fstream>
#include <cstring> // for strerror()
#include <iostream>
#include <limits> // std::numeric_limits<double>::max()
#include <vector>
#include <algorithm> // std::max_element, std::sort
#include <cmath> // std::abs()
#include <sstream>
#include <cctype>

namespace leximaxIST {

    bool descending_order (int i, int j);
    
    void Solver::read_gurobi_output(std::vector<int> &model, bool &sat, StreamBuffer &r)
    {
        while (*r != EOF) {
            if (*r != 'x')
                skipLine(r);
            else {
                sat = true;
                ++r;
                const int var = parseInt(r);
                assert(model.size()>(size_t)var);
                ++r; // skip whitespace
                if (*r == '1')
                    model[var] = var;
                else if (*r == '0')
                    model[var] = -var;
                else {
                    std::string errmsg ("Can't read gurobi output '" + m_file_name + ".sol");
                    char current_char (*r);
                    std::string current_char_str (1, current_char);
                    errmsg += "' - expecting '1' or '0' but instead got '" + current_char_str + "'";
                    print_error_msg(errmsg);
                    if (!m_leave_tmp_files)
                        remove_tmp_files();
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
/*
    int Solver::read_glpk_output(std::vector<int> &model, bool &sat, StreamBuffer &r)
    {
        // TODO
    }

    int Solver::read_lpsolve_output(std::vector<int> &model, bool &sat, StreamBuffer &r)
    {
        // TODO
    }

    void Solver::read_scip_output(std::vector<int> &model, bool &sat, StreamBuffer &r)
    {
        // TODO
    }

    void Solver::read_cbc_output(std::vector<int> &model, bool &sat, StreamBuffer &r)
    {
        // TODO
    }
*/
    void Solver::read_cplex_output(std::vector<int> &model, bool &sat, StreamBuffer &r)
    {
        // set all variables to false, because we only get the variables that are true
        for (size_t v (1); v < m_id_count + 1; ++v)
            model.at(v) = -v;
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
                            const int l = parseInt(r);
                            assert(model.size()>(size_t)l);
                            model.at(l) = l;
                        }
                    }
                }
            }
        }
    }

    // if model.empty() in the end then unsat, else sat
    void Solver::read_sat_output(std::vector<int> &model, bool &sat, StreamBuffer &r)
    {
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
                    const int l = parseInt(r);
                    assert(model.size()>(size_t)l);
                    model[l] = (sign ? l : -l);
                }
                assert (*r=='\n');
                ++r; // skip '\n'
            }
        }
    }
    
    void Solver::read_solver_output(std::vector<int> &model)
    {
        std::string output_filename (m_file_name + ".sol");
        gzFile of = gzopen(output_filename.c_str(), "rb");
        if (of == Z_NULL) {
            print_error_msg("Can't open file '" + output_filename + "' for reading");
            if (!m_leave_tmp_files)
                remove_tmp_files();
            exit(EXIT_FAILURE);
        }
        StreamBuffer r(of);
        bool sat = false;
        model.resize(static_cast<size_t>(m_id_count + 1), 0);
        if (m_formalism == "wcnf" || m_formalism == "opb")
            read_sat_output(model, sat, r);
        else if (m_formalism == "lp") {
            if (m_lp_solver == "cplex")
                read_cplex_output(model, sat, r);
            else if (m_lp_solver == "gurobi")
                read_gurobi_output(model, sat, r);
            /*else if (m_lp_solver == "glpk")
                read_glpk_output(model, sat, r);
            else if (m_lp_solver == "scip")
                read_scip_output(model, sat, r);
            else if (m_lp_solver == "cbc")
                read_cbc_output(model, sat, r);
            else if (m_lp_solver == "lpsolve")
                read_lpsolve_output(model, sat, r);*/
        }
        if (!sat)
            model.clear();
        gzclose(of);
    }

    void Solver::split_command(const std::string &command, std::vector<std::string> &command_split)
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
                    exit(EXIT_FAILURE);
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
            exit(EXIT_FAILURE);
        }
    }

    // TODO: move the part of setting the command to the caller
    void Solver::call_ext_solver(const std::string &command)
    {
        if (m_verbosity >= 1)
            std::cout << "c Calling external solver..." << '\n';
       /* pid_t pid (fork()); // TODO: maybe change this part to m_child_pid
        if (pid == -1) {
            std::string errmsg (strerror(errno));
            print_error_msg("Can't fork process: " + errmsg);
            return -1;
        }
        if (pid == 0) { // child process -> external solver
            // open output_filename and error_filename
            FILE *my_out_stream (nullptr);
            if (m_formalism != "lp" || m_lp_solver != "gurobi") {
                my_out_stream = fopen(output_filename.c_str(), "w");
                if (my_out_stream == nullptr) {
                    print_error_msg("Can't open '" + output_filename + "' for writing");
                    exit(-1);
                }
                
            }
            FILE *my_err_stream (fopen(error_filename.c_str(), "w"));
            if (my_err_stream == nullptr) {
                print_error_msg("Can't open '" + error_filename + "' for writing");
                exit(-1);
            }
            // redirect std output to output_filename and std error to error_filename (if not gurobi)
            if (m_formalism != "lp" || m_lp_solver != "gurobi") {
                if (dup2(fileno(my_out_stream), 1) == -1) {
                    print_error_msg("Can't redirect Standard Output of external solver");
                    exit(-1);
                }
            }
            // copy screen (in file descriptor 2) to a new file descriptor
            // why? to redirect std error back to screen later on
            int newfd (dup(2));
            if (newfd == -1) {
                print_error_msg("Can't redirect Standard Error of external solver");
                exit(-1);
            }
            if (dup2(fileno(my_err_stream), 2) == -1) {
                print_error_msg("Can't redirect Standard Error of external solver");
                exit(-1);
            }
            // convert command to vector of strings (split by whitespace)
            std::vector<std::string> command_split;
            if (split_command(command, command_split) == -1)
                exit(-1);
            /*command_split.clear();
            command_split.push_back("grep");
            command_split.push_back("'packup'");
            command_split.push_back("/home/miguelcabral/thesis/old_packup/Makefile");*/
            // convert to array for execv function
            /*std::vector<char*> args(command_split.size() + 1, new char[30]{}); // memory leak - no problems because child process dies afterward?
            //char **args (new char*[command_split.size()]);
            for (size_t i (0); i < command_split.size(); ++i) {
                size_t length (command_split[i].copy(args[i], command_split[i].size()));
                args[i][length] = '\0';
                std::cerr << "i="<< i << std::endl;
                std::cerr << args[i] << std::endl;
            }
            args[command_split.size()] = nullptr;*/
            /*std::cout << "command_split: ";
            for (std::string &s : command_split)
                std::cout << s << ", ";
            std::cout << std::endl;*/
            // call solver
            //std::string ola ("/bin/grep");
            /*if (execv(args[0]/*ola.c_str()*//*, args.data()) == -1) {
                std::string errmsg (strerror(errno));
                // redirect std error back to screen
                dup2(newfd, 2); // if this fails does not matter, terminate child process anyway
                print_error_msg("Something went wrong when calling the external solver: " + errmsg);
                exit(-1);
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
        /*if (WEXITSTATUS(pid_status)) {
            std::string errmsg (strerror(errno));
            print_error_msg("The external solver finished with non-zero error status: " + errmsg);
            return -1;
        }*/
        system(command.c_str());
        // set to zero, i.e. no external solver is currently running
        m_child_pid = 0;
    }

    void Solver::write_cnf_file(int i)
    {
        m_file_name += "_" + std::to_string(i) + ".cnf";
        std::ofstream out (m_file_name);
        if (!out) {
            print_error_msg("Can't open " + m_file_name + " for writing");
            exit(EXIT_FAILURE);
        }
        // print header
        out << "p cnf " << m_id_count << " " << m_input_hard.size() << '\n';
        print_hard_clauses(out);
        out.close();
    }
    
    void Solver::write_wcnf_file(int i)
    {
        m_file_name += "_" + std::to_string(i) + ".wcnf";
        std::ofstream out (m_file_name);
        if (!out) {
            print_error_msg("Can't open " + m_file_name + " for writing");
            exit(EXIT_FAILURE);
        }
        // prepare input for the solver
        size_t weight = m_soft_clauses.size() + 1;
        out << "p wcnf " << m_id_count << " " << m_input_hard.size() + m_encoding.size() << " " << weight << '\n';
        print_hard_clauses(out);
        print_soft_clauses(out);
        out.close();
    }

    void Solver::write_opb_file(int i)
    {
        m_file_name += "_" + std::to_string(i) + ".opb";
        std::ofstream out (m_file_name);
        if (!out.is_open()) {
            print_error_msg("Can't open " + m_file_name + " for writing");
            exit(EXIT_FAILURE);
        }
        // prepare input for the solver
        out << "* #variable= " << m_id_count;
        out << " #constraint= " << m_input_hard.size() + m_encoding.size() << '\n';
        if (m_soft_clauses.size() > 0) {// print minimization function
            out << "min:";
            for (int neg_var : m_soft_clauses)
                out << " " << "+1" << m_multiplication_string << "x" << -neg_var;
            out << ";\n";
        }
        for (const Clause &cl : m_input_hard)
            print_pb_constraint(cl, out);
        for (const Clause &cl : m_encoding)
            print_pb_constraint(cl, out);
        out.close();
    }

    void Solver::write_solver_input(int i)
    {
        if (m_formalism == "wcnf")
            write_wcnf_file(i);
        else if (m_formalism == "opb")
            write_opb_file(i);
        else if (m_formalism == "lp") {
            if (m_lp_solver == "gurobi" || m_lp_solver == "scip")
                write_opb_file(i);
            if (m_lp_solver == "cplex" || m_lp_solver == "cbc")
                write_lp_file(i);
        }
    }

    void Solver::external_solve(int i)
    {
        write_solver_input(i+1);
        // call the solver
        if (m_ext_solver_cmd.empty()) {
            print_error_msg("Empty external solver command");
            if (!m_leave_tmp_files)
                remove_tmp_files();
            exit(EXIT_FAILURE);
        }
        std::string command (m_ext_solver_cmd + " ");
        if (m_formalism == "lp") { // TODO: set CPLEX parameters : number of threads, tolerance, etc.
            if (m_lp_solver == "cplex")
                command += "-c \"read " + m_file_name + "\" \"optimize\" \"display solution variables -\"";
            if (m_lp_solver == "cbc") // TODO: set solver parameters for scip and cbc as well
                command += m_file_name + " solve solution $";
            if (m_lp_solver == "scip")
                command += "-f " + m_file_name;
            if (m_lp_solver == "gurobi") {
                command = "gurobi_cl";
                command += " Threads=1 ResultFile=" + m_file_name + ".sol";
                command += " LogFile=\"\" LogToConsole=0 "; // disable logging
                command += m_file_name;
            }
        }
        else
            command += m_file_name;
        if (m_formalism != "lp" || m_lp_solver != "gurobi")
            command += " > " + m_file_name + ".sol";
        command += " 2> " + m_file_name + ".err";
        double initial_time, final_time;
        if (m_verbosity >= 1 && m_verbosity <= 2)
            initial_time = read_cpu_time();
        call_ext_solver(command);
        if (m_verbosity >= 1 && m_verbosity <= 2) {
            final_time = read_cpu_time();
            print_time(final_time - initial_time, "c Minimisation CPU time: ");
        }
        // read output of solver
        std::vector<int> model;
        read_solver_output(model);
        // if ext solver is killed before it finds a sol, the problem might not be unsat
        set_solution(model); // update solution and print obj vector
        if (!m_leave_tmp_files)
            remove_tmp_files();
        // set m_file_name back to pid
        reset_file_name();
    }

    void Solver::write_lp_file(int i)
    {
        m_file_name += "_" + std::to_string(i) + ".lp";
        std::ofstream output(m_file_name); 
        if (!output) {
            print_error_msg("Could not open " + m_file_name + " for writing");
            exit(EXIT_FAILURE);
        }
        // prepare input for the solver
        output << "Minimize\n";
        output << " obj: ";
        if (m_soft_clauses.size() > 0) {// print minimization function
            size_t nb_vars_in_line (0);
            for (size_t j (0); j < m_soft_clauses.size(); ++j) {
                int soft_var (-m_soft_clauses[j]);
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
        // print constraints
        for (const Clause &cl : m_input_hard)
            print_lp_constraint(cl, output);
        for (const Clause &cl : m_encoding)
            print_lp_constraint(cl, output);
        // print all variables after Binaries
        output << "Binaries\n";
        for (int j (1); j <= m_id_count; ++j)
            output << "x" << j << '\n';
        output << "End";
        output.close();
    }
    
    void Solver::remove_tmp_files() const
    {
        std::string output_filename (m_file_name + ".sol");
        std::string error_filename (m_file_name + ".err");
        remove(m_file_name.c_str());
        remove(output_filename.c_str());
        remove(error_filename.c_str());
    }
    
    /* model is an optimal solution of the sum of objective functions problem
     * The ceiling of the sum divided by the nb of obj functions is a lower bound
     */
    int Solver::get_lower_bound(const std::vector<int> &model)
    {
        const std::vector<int> &obj_vec (get_objective_vector(model));
        int sum (0);
        for (int obj_value : obj_vec)
            sum += obj_value;
        // ceiling of the sum divided by m_num_objectives
        int lb (sum / m_num_objectives);
        if (sum % m_num_objectives != 0)
            ++lb;
        return lb;
    }
    
    /* Calls external MaxSAT solver on the problem of the sum of all objectives
     * Updates m_solution if the new solution is leximax better
     * Returns the (optimum) value of the sum of all objectives
     */
    int Solver::maxsat_presolve()
    {
        if (m_maxsat_psol_cmd.empty()) {
            print_error_msg("Empty MaxSAT presolve command");
            exit(EXIT_FAILURE);
        }
        // soft clauses are negations of all objective variables
        m_soft_clauses.clear();
        for (const std::vector<int> &obj : m_objectives) {
            for (int x : obj)
                m_soft_clauses.push_back(-x);
        }
        write_wcnf_file(0);
        std::string command (m_maxsat_psol_cmd);
        std::string output_filename (m_file_name + ".sol");
        command += " " + m_file_name;
        command += " > " + output_filename;
        command += " 2> " + m_file_name + ".err";
        call_ext_solver(command);
        // read output
        gzFile of = gzopen(output_filename.c_str(), "rb");
        if (of == Z_NULL) {
            print_error_msg("Can't open file '" + output_filename + "' for reading");
            if (!m_leave_tmp_files)
                remove_tmp_files();
            exit(EXIT_FAILURE);
        }
        StreamBuffer r(of);
        bool sat = false;
        // choose the best solution in terms of the leximax order
        std::vector<int> model (m_id_count + 1, 0);
        read_sat_output(model, sat, r);
        const std::vector<int> &obj_vec (get_objective_vector(model));
        int sum (0);
        for (int obj_value : obj_vec)
            sum += obj_value;
        set_solution(model); // update m_solution if this model is better and print obj_vec
        if (!m_leave_tmp_files)
            remove_tmp_files();
        // set m_file_name back to pid
        reset_file_name();
        return sum;
    }

    
    /* Presolve: Find solutions to get bounds on the optimal first maximum
     * Returns the minimum value of the sum of the obj functions, if m_maxsat_presolve
     * Otherwise returns 0
     * Sets m_solution which can be used to retrieve the upper bound
     */
    int Solver::presolve()
    {   
        int sum (0);
        if (m_verbosity >= 1) {
            std::cout << "c Presolving...\n";
            std::cout << "c Checking satisfiability...\n";
        }
        if (!call_sat_solver(m_sat_solver, {})) {
            m_status = 'u';
            return 0;
        }
        m_status = 's';
        if (m_maxsat_presolve) {
            double initial_time;
            if (m_verbosity >= 1) {
                std::cout << "c Minimising sum of objective functions...\n";
                initial_time = read_cpu_time();
            }
            sum = maxsat_presolve();
            if (m_verbosity >= 1) {
                print_time(read_cpu_time() - initial_time, "c MaxSAT Presolving CPU time: ");
                std::cout << "c Minimum value of the sum: " << sum << '\n';
            }
        }
        return sum;
    }
    
    // calls sat solver with assumptions and returns true if sat and false if unsat
    bool Solver::call_sat_solver(IpasirWrap *solver, const std::vector<int> &assumps)
    {
        if (solver == nullptr) {
            print_error_msg("In call_sat_solver function: solver is a null pointer!");
            exit(EXIT_FAILURE);
        }
        if (m_verbosity >= 1) {
            std::cout << "c Calling SAT solver... ";
            std::cout << '(' <<  m_id_count << " variables and ";
            std::cout << m_encoding.size() + m_input_hard.size() << " clauses)\n";
        }
        double initial_time (read_cpu_time());
        const int rv (solver->solve(assumps));
        if (m_verbosity >= 1)
            print_time(read_cpu_time() - initial_time, "c SAT call CPU time: ");
        if (rv == 0) {
            print_error_msg("SAT Solver Interrupted!");
            exit(EXIT_FAILURE);
        }
        if (rv == 10) {
            // get solution and return
            set_solution(solver->model());
            return true;
        }
        else
            return false;
    }
    
    /*void Solver::add_solving_time(double t)
    {
        // set to t the first position of m_times that has 0.0
        for (double &time : m_times) {
            if (std::abs(time - 0.0) < 0.00001)
                time = t;
                return;
        }
    }*/
    
} /* namespace leximaxIST */
