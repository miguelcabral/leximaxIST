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
        set_solution(model); // update solution if model is better in the leximax order
        if (!m_leave_tmp_files)
            remove_tmp_files();
        // set m_file_name back to pid
        reset_file_name();
        if (m_verbosity >= 1 && m_verbosity <= 2)
            print_obj_vector();
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
    
    void mss_choose_obj_seq (std::vector<std::vector<int>> &todo_vec, int &obj_index)
    {
        // obj_index points to the first objective whose todo is not empty
        for (obj_index = 0; obj_index < todo_vec.size(); ++obj_index) {
            if (!todo_vec.at(obj_index).empty())
                break;
        }
        if (obj_index == todo_vec.size())
            obj_index = -1;
    }
    
    void erase_and_decrement (std::vector<std::vector<int>> &todo_vec, std::vector<int> &obj_vector, int &obj_index)
    {
        // erase next_var from todo by moving last element to its position
        todo_vec.at(obj_index).at(0) = todo_vec.at(obj_index).back();
        todo_vec.at(obj_index).pop_back();
        // decrement obj_vector
        --obj_vector[obj_index];
    }
    
    void mss_choose_obj_max (std::vector<std::vector<int>> &todo_vec, std::vector<int> &obj_vector, int &obj_index)
    {
        std::vector<int> max_index_set;
        int max (*(std::max_element(obj_vector.begin(), obj_vector.end())));
        for (int i (0); i < todo_vec.size(); ++i) {
            if (obj_vector.at(i) == max)
                max_index_set.push_back(i);
        }
        // select a todo set with an index in max_index_set
        obj_index = max_index_set.at(0);
        // if one of the todos corresponding to max is empty, the ub can't be improved
        for (int i : max_index_set) {
            if (todo_vec.at(i).empty()) {
                obj_index = -1;
                break;
            }
        }
    }
    
    int Encoder::mss_choose_var (std::vector<std::vector<int>> &todo_vec, std::vector<int> &obj_vector, int &obj_index) const
    {
        if (m_ub_presolve == 1)
            mss_choose_obj_seq (todo_vec, obj_index);
        else if (m_ub_presolve == 2)
            mss_choose_obj_max (todo_vec, obj_vector, obj_index);
        // -1 means stop computing mss - possibly because the upper bound can't be improved
        if (obj_index == -1)
            return -1;
        int next_var (todo_vec.at(obj_index).at(0));
        erase_and_decrement (todo_vec, obj_vector, obj_index);
        return next_var;
    }
    
    // add falsified variables in todo to hard clauses of sat solver; decrement obj_vector accordingly
    void Encoder::mss_add_falsified (IpasirWrap &solver, std::vector<std::vector<int>> &todo_vec, std::vector<int> &obj_vector) const
    {
        for (int j (0); j < m_num_objectives; ++j) {
            std::vector<int> &todo (todo_vec[j]);
            for (size_t i (0); i < todo.size(); ++i) {
                int var (todo.at(i));
                if (m_solution[var] < 0) {
                    solver.addClause(-var);
                    // decrement objective value
                    --obj_vector[j];
                    // erase element in todo by moving last element to current position
                    todo.at(i) = todo.back();
                    todo.pop_back();
                }
            }
        }    
    }
    
    void Encoder::enumerate_mss()
    {
        // TODO
        // setup of timeout: terminate function SAT solver 
        // find mss, add blocking clause and repeat, until I get unsat or I reach timeout
    }
    
    /* Compute an MSS of the problem with sum of obj funcs 
     * stop if the upper bound can't be improved
     */
    /* TODO: see how this behaves without add falsified
     * without add falsified the obj_vector is not correct
     * Fix this - use get_objective_vector() <- probably not too heavy
     */
    void Encoder::find_mss(const std::vector<Clause> &blocking_cls)
    {
        // Can not use m_sat_solver, because we add the unit clauses of the MSS
        IpasirWrap solver;
        for (const Clause *hard_cl : m_hard_clauses)
            solver.addClause(hard_cl);
        m_status = solver.solve() ? 's' : 'u';
        if (m_status == 'u')
            return;
        m_solution = std::move(solver.model());
        // solver.model() is in a valid but unspecified state - so clear it
        solver.model().clear();
        // obj_vector starts as if all variables are true and is decremented along the way
        std::vector<int> obj_vector (m_num_objectives);
        std::vector<std::vector<int>> todo_vec (m_num_objectives); // fill constructor
        for (int i (0); i < m_num_objectives; ++i) {
            const std::vector<int> *objective (m_objectives[i]);
            todo_vec[i] = *objective; // copy assignment
            obj_vector[i] = objective->size();
        }
        int obj_index (0);
        mss_add_falsified (solver, todo_vec, obj_vector);
        int nb_calls (1);
        while (true /*stops when next_var == -1*/) {
            if (m_verbosity == 2)
                print_mss_todo(todo_vec);
            /* choose var in todo using an heuristic
             * var is removed from todo
             * The obj value in obj_vector is decremented
             * obj_index is set to the objective that contains var */
            int next_var (mss_choose_var (todo_vec, obj_vector, obj_index));
            if (next_var == -1)
                break;
            std::vector<int> assumps {-next_var};
            if (solver.solve(assumps)) {
                m_solution = std::move(solver.model());
                solver.model().clear();
                solver.addClause(-next_var);
                mss_add_falsified (solver, todo_vec, obj_vector);
            }
            else {
                solver.addClause(next_var);
                ++obj_vector[obj_index]; // correct obj value (it was decremented before)
            }
            ++nb_calls;
        }
        if (m_verbosity >= 1) {
            print_mss_info(nb_calls);
            print_obj_vector(obj_vector);
        }
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
        set_solution(model);
        if (!m_leave_tmp_files)
            remove_tmp_files();
        // set m_file_name back to pid
        reset_file_name();
        if (m_verbosity >= 1 && m_verbosity <= 2)
            print_obj_vector(obj_vec);
        return sum;
    }
    
    /* Presolve: Find solutions to get bounds on the optimal first maximum
     * Returns the minimum value of the sum of the obj functions, if m_maxsat_presolve
     * Otherwise returns 0
     * Sets m_solution which can be used to retrieve the upper bound
     */
    int Encoder::presolve()
    {   
        int sum (0);
        double initial_time;
        if (m_verbosity >= 1) {
            std::cout << "c Presolving...\n";
            initial_time = read_cpu_time();
        }
        if (m_ub_presolve == 0) {
            if (m_verbosity >= 1)
                std::cout << "c Calling SAT solver...\n";
            sat_solve();
        }
        else if (m_ub_presolve == 1 || m_ub_presolve == 2) {
            if (m_verbosity >= 1) {
                if (m_ub_presolve == 1)
                    std::cout << "c Greedy sequential minimisation...\n";
                if (m_ub_presolve == 2)
                    std::cout << "c Greedy maximum minimisation...\n";
            }
            mss_solve();
        }
        if (m_verbosity >= 1)
            print_time(read_cpu_time() - initial_time, "c Upper Bound Presolving CPU time: ");
        if (m_status == 's' && m_maxsat_presolve) {
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
