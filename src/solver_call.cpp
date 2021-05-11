#include <leximaxIST_Encoder.h>
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
#include <sstream>
#include <cctype>

namespace leximaxIST {

    bool descending_order (int i, int j);
    
    void Encoder::read_gurobi_output(std::vector<int> &model, bool &sat, StreamBuffer &r)
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
    int Encoder::read_glpk_output(std::vector<int> &model, bool &sat, StreamBuffer &r)
    {
        // TODO
    }

    int Encoder::read_lpsolve_output(std::vector<int> &model, bool &sat, StreamBuffer &r)
    {
        // TODO
    }

    void Encoder::read_scip_output(std::vector<int> &model, bool &sat, StreamBuffer &r)
    {
        // TODO
    }

    void Encoder::read_cbc_output(std::vector<int> &model, bool &sat, StreamBuffer &r)
    {
        // TODO
    }
*/
    void Encoder::read_cplex_output(std::vector<int> &model, bool &sat, StreamBuffer &r)
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
    void Encoder::read_sat_output(std::vector<int> &model, bool &sat, StreamBuffer &r)
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
    
    void Encoder::read_solver_output(std::vector<int> &model)
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

    void Encoder::split_command(const std::string &command, std::vector<std::string> &command_split)
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
    void Encoder::call_ext_solver(const std::string &command)
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

    void Encoder::write_cnf_file(int i)
    {
        m_file_name += "_" + std::to_string(i) + ".cnf";
        std::ofstream out (m_file_name);
        if (!out) {
            print_error_msg("Can't open " + m_file_name + " for writing");
            exit(EXIT_FAILURE);
        }
        // print header
        out << "p cnf " << m_id_count << " " << m_hard_clauses.size() << '\n';
        print_hard_clauses(out);
        out.close();
    }
    
    void Encoder::write_wcnf_file(int i)
    {
        m_file_name += "_" + std::to_string(i) + ".wcnf";
        std::ofstream out (m_file_name);
        if (!out) {
            print_error_msg("Can't open " + m_file_name + " for writing");
            exit(EXIT_FAILURE);
        }
        // prepare input for the solver
        size_t weight = m_soft_clauses.size() + 1;
        out << "p wcnf " << m_id_count << " " << m_hard_clauses.size() << " " << weight << '\n';
        print_hard_clauses(out);
        print_soft_clauses(out);
        out.close();
    }

    void Encoder::write_opb_file(int i)
    {
        m_file_name += "_" + std::to_string(i) + ".opb";
        std::ofstream out (m_file_name);
        if (!out.is_open()) {
            print_error_msg("Can't open " + m_file_name + " for writing");
            exit(EXIT_FAILURE);
        }
        // prepare input for the solver
        out << "* #variable= " << m_id_count;
        out << " #constraint= " << m_hard_clauses.size() << '\n';
        if (m_soft_clauses.size() > 0) {// print minimization function
            out << "min:";
            for (int neg_var : m_soft_clauses)
                out << " " << "+1" << m_multiplication_string << "x" << -neg_var;
            out << ";\n";
        }
        for (const Clause *cl : m_hard_clauses)
            print_pb_constraint(cl, out);
        out.close();
    }

    void Encoder::write_solver_input(int i)
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

    void Encoder::external_solve(int i)
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
        set_solution(model, true); // update solution and print obj vector
        if (!m_leave_tmp_files)
            remove_tmp_files();
        // set m_file_name back to pid
        reset_file_name();
    }

    void Encoder::write_lp_file(int i)
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
        for (const Clause *cl : m_hard_clauses)
            print_lp_constraint(cl, output);
        // print all variables after Binaries
        output << "Binaries\n";
        for (int j (1); j <= m_id_count; ++j)
            output << "x" << j << '\n';
        output << "End";
        output.close();
    }
    
    void Encoder::remove_tmp_files() const
    {
        std::string output_filename (m_file_name + ".sol");
        std::string error_filename (m_file_name + ".err");
        remove(m_file_name.c_str());
        remove(output_filename.c_str());
        remove(error_filename.c_str());
    }
    
    void Encoder::sat_solve()
    {
        if (m_sat_solver->solve()) {
            m_status = 's';
            m_solution = std::move(m_sat_solver->model());
            m_sat_solver->model().clear();
            if (m_verbosity >= 1)
                print_obj_vector();
        }
        else
            m_status = 'u';
    }
    
    int mss_choose_obj_seq (std::vector<std::vector<int>> &todo_vec)
    {
        int obj_index;
        // obj_index points to the first objective whose todo is not empty
        for (obj_index = 0; obj_index < todo_vec.size(); ++obj_index) {
            if (!todo_vec.at(obj_index).empty())
                break;
        }
        if (obj_index == todo_vec.size())
            obj_index = -1;
        return obj_index;
    }
    
    void erase_and_decrement (std::vector<std::vector<int>> &todo_vec, std::vector<int> &obj_vector, int &obj_index)
    {
        // erase next_var from todo by moving last element to its position
        todo_vec.at(obj_index).at(0) = todo_vec.at(obj_index).back();
        todo_vec.at(obj_index).pop_back();
        // decrement obj_vector
        --obj_vector[obj_index];
    }
    
    int mss_choose_obj_max (std::vector<std::vector<int>> &todo_vec, std::vector<int> &upper_bounds)
    {
        int max (*std::max_element(upper_bounds.begin(), upper_bounds.end()));
        for (int i (0); i < todo_vec.size(); ++i) {
            if (upper_bounds.at(i) == max)
                return i;
        }
    }
    
    /* choose next obj in todo using an heuristic - choose the maximum if
     * the ratio between the min and the max of upper_bounds is greater than m_mss_tolerance
     * returns the index of the objective that we will try to minimise next
     * if the maximum can not be improved, then stop, that is, return -1
     * check how much each objective can decrease: check todo and upper_bounds
     * if there is an objective that can not, in the best case, be less than best_max
     * then stop - return -1
     */
    int Encoder::mss_choose_obj (std::vector<std::vector<int>> &todo_vec, const std::vector<int> &upper_bounds, const int best_max) const
    {
        // first check if the maximum can not be improved
        for (int j (0); j < m_num_objectives; ++j) {
            const int ub (upper_bounds.at(j));
            const int todo_size (todo_vec.at(j).size());
            if (ub - todo_size >= best_max)
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
     * Which clauses are added to the mss is defined by parameter m_mss_add_cls
     * remove those variables from todo_vec
     * update assumps
     * update upper_bounds
     */
    void Encoder::mss_add_falsified (IpasirWrap *solver, const std::vector<int> &model, std::vector<int> &upper_bounds, std::vector<std::vector<int>> &todo_vec, std::vector<int> &assumps)
    {
        std::vector<std::vector<int>> vars_to_add (m_num_objectives); // by objective
        // find all satisfied soft clauses
        for (int j (0); j < m_num_objectives; ++j) {
            std::vector<int> &todo (todo_vec[j]);
            for (size_t i (0); i < todo.size(); ++i) {
                int var (todo.at(i));
                if (model[var] < 0)
                    vars_to_add.at(j).push_back(i);
            }
        }
        // add the clauses to the mss according to m_mss_add_cls
        if (m_mss_add_cls == 0 || m_mss_add_cls == 1) { // 0 - add all; 1 - add only some
            int min_size (vars_to_add.at(0).size());
            for (int j (1); j < m_num_objectives; ++j) {
                const int new_size (vars_to_add.at(j).size());
                if (new_size < min_size)
                    min_size = new_size;
            }
            for (int j (0); j < m_num_objectives; ++j) {
                const std::vector<int> add (vars_to_add.at(j));
                std::vector<int> &todo (todo_vec[j]);
                const int limit_to_add (m_mss_add_cls == 0 ? add.size() : min_size);
                for (const int k (0); k < limit_to_add; ++k) {
                    const int i (add.at(k));
                    int var (todo.at(i));
                    // add clause to the mss
                    if (m_mss_incremental)
                        assumps.push_back(-var);
                    else
                        solver->addClause(-var);
                    // erase element in todo by moving last element to current position
                    todo.at(i) = todo.back();
                    todo.pop_back();
                    // update upper bounds
                    --upper_bounds.at(j);
                }
            }
        }
        else if (m_mss_add_cls == 2) { // do not add
            // nothing to do here
        }
    }
    
    
    /* MSS enumeration, using basic/extended linear search
     * MSS search: if the maximum can not be improved, stop
     * Thus, we will actually get subsets of the MSSes
     * Enumeration stops if a limit of time or number of MSSes is reached
     * Returns the number of MSSes found
     */
    int Encoder::mss_enumerate()
    {
        if (m_verbosity >= 1)
            print_mss_enum_info();
        int nb_msses (0);
        std::vector<int> obj_vec (get_objective_vector());
        int best_max (*std::max_element(obj_vec.begin(), obj_vec.end()));
        std::vector<Clause> blocking_cls;
        IpasirWrap *solver (nullptr);
        while (true) {
            if (m_mss_nb_limit > 0 && nb_msses >= m_mss_nb_limit)
                break;
            IpasirWrap new_solver;
            if (m_mss_incremental)
                solver = m_sat_solver;
            else {
                solver = &new_solver;
                for (const Clause *hard_cl : m_hard_clauses)
                    solver->addClause(hard_cl);
                for (const Clause &c : blocking_cls)
                    solver->addClause(c);
            }
            std::vector<int> model;
            double initial_time (solver->set_timeout(m_mss_timeout));
            int rv (mss_linear_search(model, solver, best_max));
            // remove timeout
            solver->set_timeout(std::numeric_limits<double>::max());
            if (rv != 10)
                break; // SAT call was interrupted or UNSAT (all msses were found)
            // generate blocking clause
            // since we only have unit soft clauses, we can use both MCS or MSS
            // so I will use the set that produces the smallest blocking clause
            Clause block_mcs; // at least one clause of the MCS must be true
            Clause block_mss; // at least one clause of the MSS must be false
            for (const std::vector<int> *obj : m_objectives) {
                for (int var : *obj) {
                    if (model.at(var) > 0) // var is true -> part of MCS
                        block_mcs.push_back(-var);
                    else
                        block_mss.push_back(var);
                }
            }
            const bool mcs_is_smaller (block_mcs.size() < block_mss.size());
            if (m_mss_incremental)
                add_hard_clause(mcs_is_smaller ? block_mcs : block_mss);
            else
                blocking_cls.push_back(mcs_is_smaller ? block_mcs : block_mss);
            ++nb_msses;
            if (m_verbosity >= 1)
                print_time(read_cpu_time() - initial_time, "c Single MSS CPU time: ");
        }
        if (m_verbosity >= 1)
            std::cout << "c Number of MSS subsets found: " << nb_msses << '\n';
    }
    
    /* Compute an MSS of the problem with sum of obj funcs 
     * Stop if the upper bound can't be improved
     * returns the return value of the sat solver
     */
    int Encoder::mss_linear_search(std::vector<int> &model, IpasirWrap *solver, int &best_max)
    {
        // is there another MSS?
        std::vector<int> assumps;
        int rv (solver->solve());
        if (rv != 10)
            return rv; // UNSAT or interrupted
        model = solver->model();
        const std::vector<int> &obj_vec (set_solution(model, false));
        best_max = *std::max_element(obj_vec.begin(), obj_vec.end()); 
        std::vector<std::vector<int>> todo_vec (m_num_objectives);
        std::vector<int> upper_bounds (m_num_objectives);
        for (int i (0); i < m_num_objectives; ++i) {
            const std::vector<int> *objective (m_objectives[i]);
            todo_vec[i] = *objective; // copy assignment
            obj_vector.at(i) = objective->size();
        }
        mss_add_falsified (solver, model, upper_bounds, todo_vec, assumps);
        int nb_calls (1);
        while (true /*stops when obj_index == -1 or if SAT call is interrupted*/) {
            if (m_verbosity == 2)
                print_mss_todo(todo_vec);
            int obj_index (mss_choose_obj (todo_vec, upper_bounds, best_max));
            if (obj_index == -1)
                break;
            const int next_var (todo_vec.at(obj_index).at(0));
            std::vector<int> assumps.push_back(-next_var);
            rv = solver->solve(assumps);
            if (rv == 0)
                break;
            if (rv == 10) { // SAT
                model = solver->model();
                const std::vector<int> &obj_vec (set_solution(model, false));
                best_max = *std::max_element(obj_vec.begin(), obj_vec.end());
                // add clause to the mss
                if (!m_mss_incremental) {
                    assumps.pop_back();
                    solver->addClause(-next_var);
                }
                // update upper bounds
                --upper_bounds.at(j);
                mss_add_falsified (solver, model, upper_bounds, todo_vec, assumps);
            }
            else { // UNSAT
                // add clause to the mcs (backbone literals)
                assumps.pop_back();
                if (!m_mss_incremental)
                    solver->addClause(next_var);
            }
            // erase element in todo by moving last element to current position
            todo_vec.at(obj_index).at(0) = todo_vec.at(obj_index).back();
            todo_vec.at(obj_index).pop_back();
            ++nb_calls;
        }
        if (m_verbosity >= 1) {
            print_mss_info(nb_calls);
            print_obj_vector(get_objective_vector(model));
        }
        return rv;
    }
    
    /* model is an optimal solution of the sum of objective functions problem
     * The ceiling of the sum divided by the nb of obj functions is a lower bound
     */
    int Encoder::get_lower_bound(const std::vector<int> &model)
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
    int Encoder::maxsat_presolve()
    {
        if (m_maxsat_psol_cmd.empty()) {
            print_error_msg("Empty MaxSAT presolve command");
            exit(EXIT_FAILURE);
        }
        // soft clauses are negations of all objective variables
        m_soft_clauses.clear();
        for (const std::vector<int> *obj : m_objectives) {
            for (int x : *obj)
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
        set_solution(model, true); // update m_solution if this model is better and print obj_vec
        if (!m_leave_tmp_files)
            remove_tmp_files();
        // set m_file_name back to pid
        reset_file_name();
        return sum;
    }
    
    /* Approximate the Leximax Optimisation Problem with a greedy technique that is:
     * 1) less greedy than MCS enumeration
     * 2) works towards getting intermediate Pareto-optimal solutions
     * the MCSes are not Pareto-optimal necessarily
     * Disadvantage: finding one Pareto-optimal solution is slower than finding an MCS
     */
    void Encoder::pareto_presolve()
    {
        std::vector<int> 
        for (int i (0); i < m_num_objectives; ++i) {
            // Minimise the ith maximum (in a greedy way)
            // I have some objectives fixed.
            // try to minimise the maximum of the remaining objectives
            // do the pareto search. In the end either I get interrupted or UNSAT
            // if UNSAT I tried to minimise the ith maximum too much
            // so then I check if I can minimise it without fixing the objs
            // if I can I 
            
        }
        
        if (m_verbosity >= 1)
            std::cout << "c Searching for Pareto optimal solutions...\n";
        double initial_time (m_sat_solver->set_timeout(m_pareto_timeout));
        while (true) {
            int rv (pareto_search());
            if (rv == 0)
                break;
            // check if the maximum can be improved (if not proceed to the other maxima)
            // assume only that the maximum can be decremented
            rv = m_sat_solver->solve();
            if (rv == 0) // interrupted
                break;
            if (rv == 10) { // sat
                set_solution(m_sat_solver->model(), false);
                
            }
            else { // unsat - the ith maximum can not be improved
                if (m_truly_pareto) {
                    // try to minimise the remaining objectives by decreasing order
                }
                // fix value of the maximum objective?
            }
        }
        // unset timeout
        double final_time (m_sat_solver->set_timeout(std::numeric_limits<double>::max()));
        if (m_verbosity >= 1)
            print_time(final_time - initial_time, "c Pareto Presolving CPU time: ");
    }
    
    // TODO: possibly add lower bounds if maxsat presolving is enabled
    // TODO: add option to continue to minimise
    /* Find one Pareto-optimal solution
     * Assumptions:
     * Each obj can only decrease with respect to its current value of m_solution
     * The maximum is decreased by one, that is, each obj is less than or equal to
     * the current max - 1
     * Hard Clauses:
     * Each time we find a solution we turn the previous attempt to decrement the
     * maximum into a constraint of the problem
     */
    int Encoder::pareto_search(int max_index, IpasirWrap *solver)
    {
        double initial_time (read_cpu_time());
        int nb_calls (0);
        int rv;
        while (true/* ends when unsat or interrupted */) {
            const std::vector<int> obj_vec = get_objective_vector();
            std::vector<int> s_obj_vec (obj_vec);
            std::sort(s_obj_vec.begin(), s_obj_vec.end(), descending_order);
            max = s_obj_vec.at(max_index);
            if (max == 0)
                return 0; // optimal
            if (max_index == 0) { // the first maximum bounds all objectives
                if (m_pareto_incremental)
                    encode_ub_sorted(max); // bound all objectives
                else {
                    /*const std::vector<Clause*> &hard_cls (encode_ub_sorted(max - 1));
                    for (const Clause *c : hard_cls)
                        solver->addClause(c);*/
                }
            }
            // try to get a pareto-better solution by trying to decrease the maximum
            // fix values of other objectives in assumps
            std::vector<int> assumps;
            for (int i (0); i < m_num_objectives; ++i) {
                const std::vector<int> *sorted_vec (m_sorted_vecs.at(i));
                const int obj_val (obj_vec.at(i));
                if (obj_val > max)
                    continue; // the previous maxima are fixed
                const int ub (obj_val == max ? max - 1 : obj_val);
                int size (sorted_vec->size());
                int pos (size - 1 - ub); // pos might be < 0
                if (pos >= 0) {
                    if (m_pareto_incremental)
                        assumps.push_back(-sorted_vec->at(pos)); // neg sorted vec
                    else
                        solver->addClause(-sorted_vec->at(pos));
                }
            }
            int rv (solver->solve(assumps));
            ++nb_calls;
            if (rv == 10) {
                set_solution(solver->model(), false);
                
            }
            if (rv == 20) {
                if (m_truly_pareto) {
                    // continue until a Pareto-optimal solution is obtained
                    // fix current maximum - in assumps because it might change
                    // find out which functions are maximum and fix them
                    for (int i : objs_max) {
                        const std::vector<int> *s_vec (m_sorted_vecs.at(i));
                        // v counts the number of ones; 
                        for (int j (s_vec->size() - 1), v (0); j >= 0; ++v; --j) {
                            if (v < max)
                                assumps.push_back(s_vec->at(j)); // one
                            else
                                assumps.push_back(-s_vec->at(j)); // zero
                        }
                    }
                    ++max_index;
                }
                else // stop and try to minimise further the maximum
                    break;
            }
            if (rv == 0)
                break;
        }
        if (m_verbosity >= 1) {
            print_obj_vector();
            print_time(read_cpu_time() - initial_time, "c Single Pareto Optimal Solution CPU time: ");
            std::cout << "c Number of SAT calls: " << nb_calls << '\n';
        }
    }
    
    /* Presolve: Find solutions to get bounds on the optimal first maximum
     * Returns the minimum value of the sum of the obj functions, if m_maxsat_presolve
     * Otherwise returns 0
     * Sets m_solution which can be used to retrieve the upper bound
     */
    int Encoder::presolve()
    {   
        int sum (0);
        if (m_verbosity >= 1) {
            std::cout << "c Presolving...\n";
            std::cout << "c Checking satisfiability - calling SAT solver...\n";
        }
        sat_solve();
        if (m_status == 'u')
            return 0;
        if (m_mss_presolve)
            mss_enumerate();
        if (m_maxsat_presolve) {
            if (m_verbosity >= 1) {
                std::cout << "c Minimising sum of objective functions...\n";
                double initial_time = read_cpu_time();
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
    
    void Encoder::get_sol_and_bound(int i, int &ub)
    {
        m_solution = std::move(m_sat_solver->model());
        m_sat_solver->model().clear();
        std::vector<int> obj_vec (get_objective_vector());
        if (m_verbosity >= 1)
            print_obj_vector(obj_vec);
        // check if obj value is, by chance, better than ub
        std::sort (obj_vec.begin(), obj_vec.end(), descending_order);
        int obj_val = obj_vec.at(i);
        if (ub > obj_val)
            ub = obj_val;
        int size (m_soft_clauses.size());
        if (ub != size) { // bound only if upper bound is not trivial
            int sc = m_soft_clauses.at(size - ub - 1);
            m_sat_solver->addClause(sc); // sum of soft vars <= ub
        }
    }
    
    void Encoder::search(int i, int lb, int ub)
    {
        int nb_calls (0);
        if (m_verbosity >= 1)
            print_bounds(lb, ub);
        int size (m_soft_clauses.size());
        while (ub != lb) {
            int k;
            if (m_opt_mode == "bin")
                k = lb + (ub - lb)/2; // floor of half of the interval
            else if (m_opt_mode == "linear-su")
                k = ub - 1;
            // y <= k means size - k zeros
            // last position = size - k - 1
            int sc (m_soft_clauses.at(size - k - 1));
            std::vector<int> assumps {sc};
            if (m_verbosity >= 1)
                std::cout << "c Calling SAT solver...\n";
            double initial_time (read_cpu_time());
            bool is_sat (m_sat_solver->solve(assumps));
            if (m_verbosity >= 1)
                print_time(read_cpu_time() - initial_time, "c SAT call CPU time: ");
            if (is_sat) { // sum of soft vars <= k
                ub = k;
                // get solution and refine upper bound
                get_sol_and_bound(i, ub);
            }
            else { // sum of soft vars >= k + 1
                lb = k + 1;
                // y >= lb means at least lb ones
                // size - 1 means 1 one, size - 2 means 2 ones, ...
                sc = m_soft_clauses.at(size - lb);
                m_sat_solver->addClause(-sc);
            }
            ++nb_calls;
            if (m_verbosity >= 1)
                print_bounds(lb, ub);
        }
        if (m_verbosity >= 1)
            print_nb_sat_calls(nb_calls);
    }
    
    void Encoder::internal_solve(int i, int lb, int ub)
    {
        if (m_verbosity >= 1) {
            if (m_opt_mode == "bin")
                std::cout << "c Binary ";
            else if (m_opt_mode == "linear-su")
                std::cout << "c Linear SAT-UNSAT ";
            std::cout << "search of optimum with incremental SAT solver...\n";
        }
        double initial_time;
        if (m_verbosity >= 1)
            initial_time = read_cpu_time();
        search(i, lb, ub);
        if (m_verbosity >= 1)
            print_time(read_cpu_time() - initial_time, "c Minimisation CPU time: ");
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
    
    /*void Encoder::add_solving_time(double t)
    {
        // set to t the first position of m_times that has 0.0
        for (double &time : m_times) {
            if (std::abs(time - 0.0) < 0.00001)
                time = t;
                return;
        }
    }*/
    
} /* namespace leximaxIST */
