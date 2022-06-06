#include <leximaxIST_Solver.h>
#include <leximaxIST_rusage.h>
#include <leximaxIST_parsing_utils.h>
#include <leximaxIST_printing.h>
#include <IpasirWrap.h>
#include <zlib.h>
#include <sys/wait.h>
#include <sys/resource.h> // for getrusage()
#include <sys/types.h> // getpid(), I think
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

    // returns the model, as it is useful for the hitting set algorithm
    std::vector<int> Solver::call_ilp_solver( const std::vector<ILPConstraint> &constraints,
                                 const std::vector<int> &max_vars, 
                                 int i)
    {
        // temporary file names
        const std::string base ("/tmp/" + std::to_string(getpid()) + "_" + std::to_string(i));
        const std::string input_file_name (base + ".lp");
        const std::string sol_file_name (base + ".sol");
        m_tmp_files.push_back(input_file_name);
        m_tmp_files.push_back(sol_file_name);
        // write lp file for ilp solver
        write_lp_file(constraints, max_vars, i);
        if (m_ilp_solver == "gurobi") {
            // call gurobi
            std::string command ("gurobi_cl");
            command += " Threads=1 ResultFile=" + sol_file_name;
            //command += " Threads=1 ResultFile=/tmp/foo.sol";
            command += " LogFile= LogToConsole=0 "; // disable logging
            command += input_file_name;
            // call solver
            system(command.c_str());
        }
        else if (m_ilp_solver == "cplex") {
            std::string command ("cplex -c");
            command += " \"set logfile *\""; // disable log file cplex.log
            command += " \"set threads 1\""; // set threads to 1
            command += " \"read " + input_file_name + "\""; // read input
            command += " \"optimize\" \"display solution variables -\""; // solve and print solution to stdout
            command += " > " + sol_file_name; // redirect stdout to the solution file
            // call solver
            system(command.c_str());
        }
        else {
            print_error_msg("Invalid ILP solver '" + m_ilp_solver + "'");
            exit(EXIT_FAILURE);
        }
        // read gurobi .sol file and update m_solution
        // read output of solver
        std::vector<int> model;
        read_solver_output(model, sol_file_name);
        // if ext solver is killed before it finds a sol, the problem might not be unsat
        if (m_opt_mode != "hs") // in the hitting set algorithm, the model is just the hitting set
            set_solution(model); // update solution and print obj vector
        if (!m_leave_tmp_files)
            remove_tmp_files();
        return model;
    }
    
    void Solver::read_gurobi_output(std::vector<int> &model, bool &sat, StreamBuffer &r)
    {
        while (*r != EOF) {
            if (*r != 'x')
                skipLine(r);
            else {
                sat = true;
                ++r;
                const int var = parseInt(r);
                // skip the variables that have larger id than model.size()
                // those variables are produced by the encoding and some of them may be integer
                // we only care about the assignment to the input variables
                if (var > m_input_nb_vars)
                    skipLine(r);
                else {
                    ++r; // skip whitespace
                    if (*r == '1')
                        model.at(var) = var;
                    else if (*r == '0')
                        model.at(var) = -var;
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
        for (size_t v (1); v < m_input_nb_vars + 1; ++v)
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
                            const int var = parseInt(r);
                            // skip the variables that have larger id than model.size()
                            // those variables are produced by the encoding and some of them may be integer
                            // we only care about the assignment to the input variables
                            if (var > m_input_nb_vars)
                                skipLine(r);
                            else
                                model.at(var) = var;
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
    
    /* opens the file where the solution of the external solver is stored and
     * writes the solution in the variable 'model'
     */
    void Solver::read_solver_output(std::vector<int> &model, const std::string &filename)
    {
        gzFile of = gzopen(filename.c_str(), "rb");
        if (of == Z_NULL) {
            const std::string errmsg (strerror(errno));
            print_error_msg("Can't open file '" + filename + "' for reading - " + errmsg);
            if (!m_leave_tmp_files)
                remove_tmp_files();
            exit(EXIT_FAILURE);
        }
        StreamBuffer r(of);
        bool sat = false;
        model.resize(static_cast<size_t>(m_input_nb_vars + 1), 0);
        if (m_ilp_solver == "cplex")
            read_cplex_output(model, sat, r);
        else if (m_ilp_solver == "gurobi")
            read_gurobi_output(model, sat, r);
        else {
            print_error_msg("Invalid ILP solver '" + m_ilp_solver + "'");
            exit(EXIT_FAILURE);
        }
        /*if (m_formalism == "wcnf" || m_formalism == "opb")
            read_sat_output(model, sat, r);
        else if (m_formalism == "lp") {
            if (m_ilp_solver == "cplex")
                read_cplex_output(model, sat, r);
            else if (m_ilp_solver == "gurobi")
                read_gurobi_output(model, sat, r);*/
            /*else if (m_ilp_solver == "glpk")
                read_glpk_output(model, sat, r);
            else if (m_ilp_solver == "scip")
                read_scip_output(model, sat, r);
            else if (m_ilp_solver == "cbc")
                read_cbc_output(model, sat, r);
            else if (m_ilp_solver == "lpsolve")
                read_lpsolve_output(model, sat, r);*/
        //}
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
            if (m_formalism != "lp" || m_ilp_solver != "gurobi") {
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
            if (m_formalism != "lp" || m_ilp_solver != "gurobi") {
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
            if (m_ilp_solver == "gurobi" || m_ilp_solver == "scip")
                write_opb_file(i);
            if (m_ilp_solver == "cplex" || m_ilp_solver == "cbc")
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
            if (m_ilp_solver == "cplex")
                command += "-c \"read " + m_file_name + "\" \"optimize\" \"display solution variables -\"";
            if (m_ilp_solver == "cbc") // TODO: set solver parameters for scip and cbc as well
                command += m_file_name + " solve solution $";
            if (m_ilp_solver == "scip")
                command += "-f " + m_file_name;
            if (m_ilp_solver == "gurobi") {
                command = "gurobi_cl";
                command += " Threads=1 ResultFile=" + m_file_name + ".sol";
                command += " LogFile=\"\" LogToConsole=0 "; // disable logging
                command += m_file_name;
            }
        }
        else
            command += m_file_name;
        if (m_formalism != "lp" || m_ilp_solver != "gurobi")
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
        read_solver_output(model, m_file_name + ".sol");
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
    
    void Solver::write_lp_file(const std::vector<ILPConstraint> &constraints, const std::vector<int> &max_vars, int i) const
    {
        const std::string filename ("/tmp/" + std::to_string(getpid()) + "_" + std::to_string(i) + ".lp");
        std::ofstream os(filename); 
        if (!os) {
            print_error_msg("Could not open '" + filename + "' for writing");
            exit(EXIT_FAILURE);
        }
        // prepare input for the solver
        os << "Minimize\n";
        os << " obj: " << "x" << max_vars.at(i) << '\n';
        os << "Subject To\n";
        // print constraints
        for (const ILPConstraint &ilpc : constraints)
            ilpc.print(os);
        os << "Binaries\n";
        // print all variables except for the maximum integer variables
        for (int j (1); j <= m_id_count; ++j) {
            // find j in max_vars
            bool in_max_vars (false);
            for (int max_v : max_vars) {
                if (max_v == j)
                    in_max_vars = true;
            }
            if (!in_max_vars)
                os << "x" << j << '\n';
        }
        os << "Generals\n";
        for (size_t j (0); j < max_vars.size(); ++j)
            os << "x" << max_vars.at(j) << '\n';
        os << "End";
        os.close();
    }
    
    void Solver::remove_tmp_files()
    {
        for (const std::string &tmp_file : m_tmp_files)
            remove(tmp_file.c_str());
        m_tmp_files.clear();
    }
    
    int mss_choose_obj_seq (const std::vector<std::vector<int>> &todo_vec)
    {
        int obj_index;
        // obj_index points to the first objective whose todo is not empty
        for (obj_index = 0; obj_index < todo_vec.size(); ++obj_index) {
            if (!todo_vec.at(obj_index).empty())
                break;
        }
        if (obj_index == todo_vec.size()) // all empty
            obj_index = -1;
        return obj_index;
    }
    
    void erase_from_todo (std::vector<std::vector<int>> &todo_vec, const int obj_index, const int var_index)
    {
        // erase next_var from todo by moving last element to its position
        todo_vec.at(obj_index).at(var_index) = todo_vec.at(obj_index).back();
        todo_vec.at(obj_index).pop_back();
    }
    
    int mss_choose_obj_max (const std::vector<std::vector<int>> &todo_vec, const std::vector<int> &upper_bounds)
    {
        int max (*std::max_element(upper_bounds.begin(), upper_bounds.end()));
        for (int i (0); i < todo_vec.size(); ++i) {
            if (upper_bounds.at(i) == max)
                return i;
        }
        return -1; // can not reach this line
    }
    
    /* choose next obj in todo using an heuristic - choose the maximum if
     * the ratio between the min and the max of upper_bounds is greater than m_mss_tol
     * returns the index of the objective that we will try to minimise next
     * if the maximum can not be improved, then stop, that is, return -1
     * check how much each objective can decrease: check todo and upper_bounds
     * if there is an objective that can not, in the best case, be less than best_max
     * then stop - return -1
     */
    int Solver::mss_choose_obj (const std::vector<std::vector<int>> &todo_vec, const std::vector<std::vector<int>> &mss, const int best_max) const
    {
        // compute the upper bounds
        std::vector<int> upper_bounds (m_num_objectives);
        for (int j (0); j < m_num_objectives; ++j)
            upper_bounds.at(j) = m_objectives.at(j).size() - mss.at(j).size();
        // first check if the maximum can not be improved
        for (int j (0); j < m_num_objectives; ++j) {
            const int todo_size (todo_vec.at(j).size());
            if (upper_bounds.at(j) - todo_size >= best_max)
                return -1;
        }
        const int max (*std::max_element(upper_bounds.begin(), upper_bounds.end()));
        const int min (*std::min_element(upper_bounds.begin(), upper_bounds.end()));
        const int ratio ((max - min) * 100 / max);
        if (ratio >= m_mss_tolerance)
            return mss_choose_obj_max (todo_vec, upper_bounds);
        else
            return mss_choose_obj_seq (todo_vec);
    }
    
    /* Add satisfied soft clauses (or falsified variables) to the mss
     * Which clauses are added to the mss is defined by parameter m_mss_add_cls:
     * 0 - add all clauses
     * 1 - add as much as possible while simultaneously trying to even out the upper bounds
     * 2 - add only the satisfied clause used in the SAT test (already added)
     * remove those variables from todo_vec
     * update assumps
     * update upper_bounds
     */
    void Solver::mss_add_falsified (IpasirWrap *solver, const std::vector<int> &model, std::vector<std::vector<int>> &mss, std::vector<std::vector<int>> &todo_vec, std::vector<int> &assumps)
    {
        std::vector<std::vector<int>> vars_to_add (m_num_objectives); // by objective
        // find all satisfied soft clauses that haven't been added to the mss
        for (int j (0); j < m_num_objectives; ++j) {
            std::vector<int> &todo (todo_vec[j]);
            for (size_t i (0); i < todo.size(); ++i) {
                int var (todo.at(i));
                if (model[var] < 0)
                    vars_to_add.at(j).push_back(i);
            }
        }
        // add the clauses to the mss according to m_mss_add_cls
        if (m_mss_add_cls == 0 || m_mss_add_cls == 1) {
            // compute the maximum of the best case decreased upper bounds
            // add clauses for each objective until the upper bound is decreased to max, if possible
            std::vector<int> upper_bounds (m_num_objectives);
            for (int j (0); j < m_num_objectives; ++j)
                upper_bounds.at(j) = m_objectives.at(j).size() - mss.at(j).size();
            int max (upper_bounds.at(0) - vars_to_add.at(0).size());
            for (int j (1); j < m_num_objectives; ++j) {
                const int best_case_ub (upper_bounds.at(j) - vars_to_add.at(j).size());
                if (max < best_case_ub)
                    max = best_case_ub;
            }
            for (int j (0); j < m_num_objectives; ++j) {
                const std::vector<int> add (vars_to_add.at(j));
                std::vector<int> &todo (todo_vec[j]);
                int limit_to_add; // number of variables to add to objective j
                if (m_mss_add_cls == 0)
                    limit_to_add = add.size(); // all
                else
                    limit_to_add = upper_bounds.at(j) - max; // even out upper bounds
                // add clauses to mss and remove them from todo
                // must do so from end to begining, because of how we erase from todo
                for (int k (limit_to_add - 1); k >= 0; --k) { // Do not change the order!
                    const int i (add.at(k));
                    int var (todo.at(i));
                    // add clause to the mss
                    if (m_mss_incr)
                        assumps.push_back(-var);
                    else
                        solver->addClause(-var);
                    mss.at(j).push_back(-var);
                    // erase element in todo by moving last element to current position
                    erase_from_todo(todo_vec, j, i);
                }
            }
        }
        else if (m_mss_add_cls == 2) { // do not add
            // nothing to do here
        }
    }
    
    // TODO: what to do if the MSS subset returned is empty? 
    // No blocking clause! Use the subset of MCS?
    /* MSS enumeration, using basic/extended linear search
     * MSS search: if the maximum can not be improved, stop
     * Thus, we will actually get subsets of the MSSes
     * Enumeration stops if a limit of time or number of MSSes is reached
     */
    void Solver::mss_enumerate()
    {
        if (m_verbosity >= 1)
            print_mss_enum_info();
        int nb_msses (0);
        std::vector<int> obj_vec (get_objective_vector());
        int best_max (*std::max_element(obj_vec.begin(), obj_vec.end()));
        std::vector<Clause> blocking_cls;
        IpasirWrap *solver (nullptr);
        double initial_time (read_cpu_time());
        while (true) {
            if (m_mss_nb_limit > 0 && nb_msses >= m_mss_nb_limit)
                break;
            IpasirWrap new_solver;
            if (m_mss_incr)
                solver = m_sat_solver;
            else {
                solver = &new_solver;
                solver->addClauses(m_input_hard);
                solver->addClauses(blocking_cls);
            }
            std::vector<std::vector<int>> mss (m_num_objectives);
            solver->set_timeout(m_approx_tout, initial_time);
            const int rv (mss_linear_search(mss, solver, best_max));
            // remove timeout
            solver->set_timeout(std::numeric_limits<double>::max(), 0);
            if (rv != 10)
                break; // SAT call was interrupted or UNSAT (all msses were found)
            // blocking clause - at least one clause of the satisfiable subset is false
            int mss_size (0);
            for (const std::vector<int> &s : mss)
                mss_size += s.size();
            // if mss is empty then all msses were found
            if (mss_size == 0)
                break;
            Clause block_mss (mss_size);
            { // construct blocking clause
                int i (0);
                for (const std::vector<int> &s : mss) {
                    for (int lit : s) {
                        block_mss.at(i) = -lit;
                        ++i;
                    }
                }
            }
            if (m_mss_incr)
                add_hard_clause(block_mss);
            else
                blocking_cls.push_back(block_mss);
            ++nb_msses;
        }
        if (m_verbosity == 2) {
            std::cout << "c Blocking clauses:\n";
            for (const Clause &c : blocking_cls) {
                std::cout << "c ";
                for (const int lit : c)
                    std::cout << lit << ' ';
                std::cout << '\n';
            }
        }
        if (m_verbosity >= 1)
            std::cout << "c Number of MSS subsets found: " << nb_msses << '\n';
    }
    
    /* Compute an MSS of the problem with sum of obj funcs 
     * Stop if the upper bound can't be improved
     * returns rv, the return value of the first call to the sat solver:
     * 10 - sat means there is another mss
     * 20 - unsat means all msses were found
     * 0 - interrupted means timeout reached
     * in the end, variable mss is the sets of chosen satisfied soft clauses, by objective
     */
    int Solver::mss_linear_search(std::vector<std::vector<int>> &mss, IpasirWrap *solver, int &best_max)
    {
        // is there another MSS?
        std::vector<int> assumps;
        int rv (solver->solve());
        if (rv != 10)
            return rv; // UNSAT or interrupted
        // SAT, but the MSS may be empty. If so, all MSSes have been found
        std::vector<int> model (solver->model()); // copy
        const std::vector<int> &obj_vec (set_solution(solver->model())); // move
        best_max = *std::max_element(obj_vec.begin(), obj_vec.end()); 
        std::vector<std::vector<int>> todo_vec (m_num_objectives);
        for (int i (0); i < m_num_objectives; ++i) {
            const std::vector<int> &objective (m_objectives.at(i));
            todo_vec[i] = objective; // copy assignment
        }
        mss_add_falsified (solver, model, mss, todo_vec, assumps);
        int nb_calls (1);
        while (true /*stops when obj_index == -1 or if SAT call is interrupted*/) {
            if (m_verbosity == 2)
                print_mss_debug(todo_vec, mss);
            int obj_index (mss_choose_obj (todo_vec, mss, best_max));
            if (obj_index == -1)
                break;
            const int next_var (todo_vec.at(obj_index).at(0));
            assumps.push_back(-next_var);
            const int rv_local = solver->solve(assumps);
            if (rv_local == 0) {
                rv = 0; // interrupted
                break;
            }
            if (rv_local == 10) { // SAT
                model = solver->model(); // copy
                const std::vector<int> &obj_vec (set_solution(solver->model())); // move
                best_max = *std::max_element(obj_vec.begin(), obj_vec.end());
                // add clause to the mss
                if (!m_mss_incr) {
                    assumps.pop_back();
                    solver->addClause(-next_var);
                }
                // update mss
                mss.at(obj_index).push_back(-next_var);
                // remove next_var from todo (BEFORE mss_add_falsified)
                erase_from_todo(todo_vec, obj_index, 0);
                mss_add_falsified (solver, model, mss, todo_vec, assumps);
            }
            else { // UNSAT
                // add clause to the mcs (backbone literals)
                assumps.pop_back();
                if (!m_mss_incr)
                    solver->addClause(next_var);
                // remove next_var from todo
                erase_from_todo(todo_vec, obj_index, 0);
            }
            ++nb_calls;
        }
        if (m_verbosity >= 1)
            print_mss_info(nb_calls, todo_vec, mss);
        return rv;
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
    
    void Solver::approximate()
    {
        if (m_verbosity >= 2)
            std::cout << std::unitbuf; // debug - flushes the output stream after any output operation       
        m_input_nb_vars = m_id_count; // store number of input variables
        if (m_verbosity > 0 && m_verbosity <= 2) {
            std::cout << "c Approximating using algorithm " << m_opt_mode << "...\n";
            std::cout << "c Number of input variables: " << m_input_nb_vars << '\n';
            std::cout << "c Number of input hard clauses: " << m_input_hard.size() << '\n';
            std::cout << "c Number of objective functions: " << m_num_objectives << '\n';
        }
        double initial_time (read_cpu_time());
        // check if problem is satisfiable
        if (!call_sat_solver(m_sat_solver, {})) {
            m_status = 'u';
            return;
        }
        m_status = 's'; // update status to SATISFIABLE
        if (m_approx == "gia") {
            // encode sorted vectors with sorting network
            for (int j (0); j < m_num_objectives; ++j)
                encode_sorted(m_objectives.at(j), j);
            encode_bounds(0, 0); // upper bound all objs based on the solution given by the SAT solver
            gia();
        }
        else if (m_approx == "mss") {
            mss_enumerate();
        }
        else {
            print_error_msg("Invalid approximation algorithm");
            exit(EXIT_FAILURE);
        }
        if (m_verbosity >= 1)
            print_time(read_cpu_time() - initial_time, "c Approximation CPU time: ");
    }
    
    // TODO: Add lower bounds from maxsat presolving
    /* Approximate the Leximax Optimisation Problem with a "greedy" technique that is:
     * 1) less "greedy" than MCS enumeration
     * 2) works towards getting intermediate Pareto-optimal solutions
     * the MCSes are not Pareto-optimal necessarily
     * Disadvantage: finding one Pareto-optimal solution is slower than finding an MCS
     * This is based on the Guided Improvement Algorithm (GIA)
     */
    void Solver::gia()
    {
        int max_index (0);
        bool skip (false);
        double initial_time (read_cpu_time());
        while (true /*stops when interrupted or last max can not be improved*/) {
            IpasirWrap new_solver;
            IpasirWrap *solver (nullptr);
            if (!m_gia_incr) {
                solver = &new_solver;
                solver->addClauses(m_input_hard);
                solver->addClauses(m_encoding);
            }
            else
                solver = m_sat_solver;
            solver->set_timeout(m_approx_tout, initial_time);
            if (!skip) {// skip if we know the solution can not be improved
                const int rv (pareto_search(max_index, solver));
                if (rv == 0)
                    break;
            }
            // unsat
            if (max_index == m_num_objectives - 1 && skip) {
                /* skip being true implies:
                 * max m_num_objectives - 2 can not improve
                 * max m_num_objectives - 1 has been minimised
                 */
                break;
            }
            if (max_index == m_num_objectives)
                break;
            // check if the ith max can be improved without restrictions on smaller objs
            const std::vector<int> &obj_vec = get_objective_vector();
            if (m_verbosity == 2) {
                std::cout << "c Current objective vector: ";
                print_obj_vector(obj_vec);
            }
            std::vector<int> s_obj_vec (obj_vec);
            std::sort(s_obj_vec.begin(), s_obj_vec.end(), descending_order);
            if (s_obj_vec.at(max_index) == 0)
                break;
            std::vector<int> assumps;
            // fix values of previous maxima in assumps
            fix_previous_max(assumps, max_index, obj_vec);
            // bound the remaining objs with max - 1
            decrease_max(assumps, max_index, obj_vec);
            // solve with m_sat_solver because the new_solver is now forever unsat
            // set upper bound on all objectives as hard clauses
            if (max_index == 0) 
                encode_ub_sorted(s_obj_vec.at(max_index));
            double t (0.0);
            if (m_verbosity >= 1) {
                t = read_cpu_time();
                std::cout << "c Calling SAT solver...\n";
            }
            const int rv (m_sat_solver->solve(assumps));
            if (m_verbosity >= 1)
                print_time(read_cpu_time() - t, "c SAT call CPU time: ");
            if (rv == 0)
                break;
            if (rv == 20) {// unsat -> move on to next max
                skip = m_gia_pareto;
                ++max_index;
                if (max_index == m_num_objectives)
                    break;
            }
            else { // sat -> try to decrease further the current max
                skip = false;
                set_solution(m_sat_solver->model());
            }
        }
        // unset timeout
        m_sat_solver->set_timeout(std::numeric_limits<double>::max(), 0);
    }
    
    /* adds to unit_clauses the clauses that:
     * fix some objectives whose values are equal to previous maxima
     * CONVENTION: if there are previous maxima equal to the current maximum,
     * then fix the objectives equal to the previous maxima with the smallest index
     */
    void Solver::fix_previous_max(std::vector<int> &unit_clauses, int max_index, const std::vector<int> &obj_vec) const
    {
        std::vector<int> s_obj_vec (obj_vec);
        std::sort(s_obj_vec.begin(), s_obj_vec.end(), descending_order);
        int min_prev (0);
        if (max_index > 0) // otherwise it does not matter, no objs will be fixed
            min_prev = s_obj_vec.at(max_index - 1); // minimum of previous maxima
        // fix the first max_index objectives with values in the range [min_prev, max]
        int j (0);
        for (int i (0); i < max_index; ++j) {
            // i is the nb of fixed objs; j is the obj index
            const int obj_val (obj_vec.at(j));
            if (obj_val >= min_prev) {
                const std::vector<int> &sorted_vec (m_sorted_vecs.at(j));
                const int size (sorted_vec.size());
                const int pos (size - obj_val); // 0 <= pos <= size
                if (pos < size)
                    unit_clauses.push_back(sorted_vec.at(pos)); // lower bound
                if (pos - 1 >= 0)
                    unit_clauses.push_back(-sorted_vec.at(pos - 1)); // upper bound
                if (m_verbosity == 2)
                    std::cout << "c Objective " << j << ": = " << obj_val << '\n';
                ++i; // one more fixed
            }
        }
    }
    
    /* adds to unit_clauses the clauses that
     * upper bound the remaining not fixed objectives; bound: <= max - 1
     */
    void Solver::decrease_max(std::vector<int> &unit_clauses, int max_index, const std::vector<int> &obj_vec) const
    {
        std::vector<int> s_obj_vec (obj_vec);
        std::sort(s_obj_vec.begin(), s_obj_vec.end(), descending_order);
        int min_prev (0);
        if (max_index > 0) // otherwise it does not matter, no objs will be fixed
            min_prev = s_obj_vec.at(max_index - 1); // minimum of previous maxima
        // upper bound (<= max-1) the objectives not fixed to previous maxima
        int j (0);
        for (int i (0); i < max_index; ++j) {
            // i is the nb of fixed objs; j is the obj index
            const int obj_val (obj_vec.at(j));
            if (obj_val >= min_prev)
                ++i; // fixed obj
            else {
                const std::vector<int> &sorted_vec (m_sorted_vecs.at(j));
                const int size (sorted_vec.size());
                const int ub (s_obj_vec.at(max_index) - 1);
                const int pos (size - ub); // 0 <= pos <= size
                if (pos - 1 >= 0)
                    unit_clauses.push_back(-sorted_vec.at(pos - 1)); // upper bound
                if (m_verbosity == 2)
                    std::cout << "c Objective " << j << ": <= " << ub << '\n';
            }
        }
        // bound the remaining objs
        for (; j < m_num_objectives; ++j) {
            const std::vector<int> &sorted_vec (m_sorted_vecs.at(j));
            const int size (sorted_vec.size());
            const int ub (s_obj_vec.at(max_index) - 1); // <= max-1
            const int pos (size - ub); // 0 <= pos <= size
            if (pos - 1 >= 0)
                unit_clauses.push_back(-sorted_vec.at(pos - 1)); // upper bound
            if (m_verbosity == 2)
                std::cout << "c Objective " << j << ": <= " << ub << '\n';
        }
    }
    
    /* adds to unit_clauses the clauses that bound the objectives less than or equal to max
     */
    void Solver::bound_objs(std::vector<int> &unit_clauses, int max, const std::vector<int> &obj_vec) const
    {
        for (int i (0); i < m_num_objectives; ++i) {
            const std::vector<int> &sorted_vec (m_sorted_vecs.at(i));
            const int obj_val (obj_vec.at(i));
            const int size (sorted_vec.size());
            if (obj_val < max) {
                const int pos (size - obj_val - 1); // -1 <= pos <= size - 1
                if (pos >= 0)
                    unit_clauses.push_back(-sorted_vec.at(pos)); // upper bound
                if (m_verbosity == 2)
                    std::cout << "c Objective " << i << ": <= " << obj_val << '\n';
            }
        }
    }
    
    // TODO: Add lower bounds from maxsat presolving
    /* Find one Pareto-optimal solution
     */
    // It may happen in this function that we prove that the current max can't improve
    // then we do not need to check that again in the pareto_presolve function
    // keep track of which maxs are optimal - advance max_index
    // how do we know when the current max can't improve?
    // when max_index == max_index_local, we get unsat and the smaller objs
    // differ from the current local max by at most 1
    int Solver::pareto_search(int &max_index, IpasirWrap *solver)
    {
        double initial_time (read_cpu_time());
        int nb_calls (0);
        int rv;
        int max_index_local (max_index);
        while (true/* ends when unsat or interrupted */) {
            std::vector<int> assumps;
            const std::vector<int> &obj_vec = get_objective_vector();
            if (m_verbosity == 2) {
                std::cout << "c Current objective vector: ";
                print_obj_vector(obj_vec);
            }
            std::vector<int> s_obj_vec (obj_vec);
            std::sort(s_obj_vec.begin(), s_obj_vec.end(), descending_order);
            const int max (s_obj_vec.at(max_index_local));
            if (max == 0) {
                rv = 20; // unsat - can not improve max
                break;
            }
            std::vector<int> unit_clauses;
            // decrease current max - bound all objectives not fixed and with value <= max
            decrease_max(unit_clauses, max_index_local, obj_vec);
            // add clauses to the problem
            for (int lit : unit_clauses) {
                if (!m_gia_incr && !m_gia_pareto)
                    solver->addClause(lit);
                else
                    assumps.push_back(lit);
            }
            unit_clauses.clear(); // clear clauses already added to the problem
            // fix previous maxima
            fix_previous_max(unit_clauses, max_index_local, obj_vec); // construct clauses
            // bound (permanently if not incremental) all objs with their current values
            bound_objs(unit_clauses, max, obj_vec); // construct clauses
            // add clauses to the problem
            for (int lit : unit_clauses) {
                if (m_gia_incr)
                    assumps.push_back(lit);
                else
                    solver->addClause(lit);
            }
            double initial_time;
            if (m_verbosity >= 1) {
                initial_time = read_cpu_time();
                std::cout << "c Calling SAT solver...\n";
            }
            rv = solver->solve(assumps);
            if (m_verbosity >= 1)
                print_time(read_cpu_time() - initial_time, "c SAT call CPU time: ");
            ++nb_calls;
            if (rv == 10)
                set_solution(solver->model());
            if (rv == 20) {
                /* check if we have proved that the current max can't improve
                 * removing the constraints on the smaller objectives
                 * check if the objectives <= max only differ by at most 1
                 * and if max_index == max_index_local
                 */
                const int min (s_obj_vec.at(m_num_objectives - 1));
                if (max_index == max_index_local && max - min <= 1)
                    ++max_index;
                if (m_gia_pareto) {
                    // continue until a Pareto-optimal solution is obtained
                    ++max_index_local;
                    if (max_index_local == m_num_objectives)
                        break; // Pareto-optimal solution has been found
                }
                else // stop to check if we can improve the max_index maximum
                    break;
            }
            if (rv == 0)
                break;
        }
        if (m_verbosity >= 1) {
            print_time(read_cpu_time() - initial_time, "c Single Pareto Optimal Solution CPU time: ");
            std::cout << "c Number of SAT calls: " << nb_calls << '\n';
        }
        return rv;
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
    
    inline void print_bounds(int lb, int ub)
    {
        std::cout << "c lb = " << lb << " ub = " << ub << '\n';
    }
    
    inline void print_nb_sat_calls(int nb_calls)
    {
        std::cout << "c Number of SAT calls: " << nb_calls << '\n';
    }
    
    void Solver::update_lb(int &lb)
    {
        const std::vector<int> &core (m_sat_solver->conflict());
        // get the position in m_soft_clauses of the var with the greatest id in the core
        // NOTE: we assume m_soft_clauses is sorted in increasing order
        for (int pos (m_soft_clauses.size() - 1); pos >= 0; --pos) {
            const int var (std::abs(m_soft_clauses.at(pos)));
            // try to find var in core
            bool found (false);
            for (int lit : core) {
                if (std::abs(lit) == var) {
                    found = true;
                    break;
                }
            }
            if (found) { // update lower bound and return
                // update the lower bound pos = size - lb, hence, lb = size - pos
                const int new_lb = m_soft_clauses.size() - pos;
                if (new_lb <= lb) {
                    print_error_msg("In Solver::update_lb(), lb did not increase");
                    exit(EXIT_FAILURE);
                }
                lb = new_lb;
                break;
            }
        }
        // check core size. If core size > 1, get the relevant variable and add it as hard
        // cost >= lb means at least lb ones
        // size - 1 means 1 one, size - 2 means 2 ones, ...
        if (m_verbosity >= 1)
            std::cout << "c core size: " << core.size() << '\n';
        if (core.size() > 1) {
            const int size (m_soft_clauses.size());
            const int sc = m_soft_clauses.at(size - lb);
            add_clause(-sc);
        }
    }
    
    void Solver::search(int i, int lb, int ub)
    {
        int nb_calls (0);
        if (m_verbosity >= 1)
            print_bounds(lb, ub);
        int size (m_soft_clauses.size());
        while (ub != lb) {
            int k;
            if (m_opt_mode == "bin")
                k = lb + (ub - lb)/2; // floor of half of the interval
            else if (m_opt_mode == "lin_su")
                k = ub - 1;
            else if (m_opt_mode == "lin_us")
                k = lb;
            // y <= k means size - k zeros; last position = size - k - 1
            // adding all neg vars to assumps may allow lb to increase by more than 1
            const int limit (size - k - 1);
            std::vector<int> assumps (limit + 1);
            for (int j (0); j <= limit; ++j)
                assumps.at(j) = m_soft_clauses.at(j);
            if (m_verbosity >= 1)
                std::cout << "c Calling SAT solver...\n";
            double initial_time (read_cpu_time());
            const int rv (m_sat_solver->solve(assumps));
            if (m_verbosity >= 1)
                print_time(read_cpu_time() - initial_time, "c SAT call CPU time: ");
            if (rv == 0) {
                print_error_msg("SAT solver interrupted with no timeout!");
                exit(EXIT_FAILURE);
            }
            if (rv == 10) { // cost <= k
                // get solution and refine upper bound
                std::vector<int> s_obj_vec (set_solution(m_sat_solver->model()));
                std::sort (s_obj_vec.begin(), s_obj_vec.end(), descending_order);
                ub = s_obj_vec.at(i); // if obj value is, by chance, less than k
                encode_ub_soft(ub); // bound the cost in hard clauses, not as assumptions
            }
            else { // cost >= k + 1
                // inspect core and check if lb can be increased by more than 1
                update_lb(lb);
            }
            ++nb_calls;
            if (m_verbosity >= 1)
                print_bounds(lb, ub);
        }
        if (m_verbosity >= 1)
            print_nb_sat_calls(nb_calls);
    }
    
    void Solver::internal_solve(const int i, const int lb)
    {
        if (m_verbosity >= 1) {
            if (m_opt_mode == "bin")
                std::cout << "c Binary ";
            else if (m_opt_mode == "lin_su")
                std::cout << "c Linear SAT-UNSAT ";
            else if (m_opt_mode == "lin_us")
                std::cout << "c Linear UNSAT-SAT ";
            std::cout << "search of optimum with incremental SAT solver...\n";
        }
        std::vector<int> s_obj_vec (get_objective_vector());
        std::sort(s_obj_vec.begin(), s_obj_vec.end(), descending_order);
        const int ub (s_obj_vec.at(i));
        double initial_time;
        if (m_verbosity >= 1)
            initial_time = read_cpu_time();
        search(i, lb, ub);
        if (m_verbosity >= 1)
            print_time(read_cpu_time() - initial_time, "c Minimisation CPU time: ");
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
    
    double read_cpu_time()
    {
        struct rusage ru;
        // parent process
        getrusage(RUSAGE_SELF, &ru);
        // user time
        double user_time ((double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1000000);
        // system time - negligible ?
        double sys_time ((double)ru.ru_stime.tv_sec + (double)ru.ru_stime.tv_usec / 1000000);
        // children that have been waited for
        getrusage(RUSAGE_CHILDREN, &ru);
        // user time
        user_time += (double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1000000;
        //system time - negligible ?
        sys_time += (double)ru.ru_stime.tv_sec + (double)ru.ru_stime.tv_usec / 1000000;
        double total_time (user_time + sys_time);
        return total_time;
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
