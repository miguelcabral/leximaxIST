#include <leximaxIST_Encoder.h>
#include <leximaxIST_rusage.h>
#include <leximaxIST_parsing_utils.h>
#include <leximaxIST_error.h>
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
#include <sstream>
#include <cctype>

namespace leximaxIST {

    int Encoder::read_gurobi_output(std::vector<int> &model)
    {
        std::string output_filename (m_file_name + ".sol");
        gzFile of = gzopen(output_filename.c_str(), "rb");
        if (of == Z_NULL) {
            print_error_msg("Can't open file '" + output_filename + "' for reading");
            return -1;
        }
        StreamBuffer r(of);
        bool sat = false;
        model.resize(static_cast<size_t>(m_id_count + 1), 0);
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
                    std::string errmsg ("Can't read gurobi output '" + output_filename);
                    char current_char (*r);
                    std::string current_char_str (1, current_char);
                    errmsg += "' - expecting '1' or '0' but instead got '" + current_char_str + "'";
                }
            }
        }
        if (!sat) model.clear();
        gzclose(of);
        return 0;
    }

    int Encoder::read_glpk_output(std::vector<int> &model)
    {
        // TODO
        return 0;
    }

    int Encoder::read_lpsolve_output(std::vector<int> &model)
    {
        // TODO
        return 0;
    }

    int Encoder::read_scip_output(std::vector<int> &model)
    {
        // TODO
        return 0;
    }

    int Encoder::read_cbc_output(std::vector<int> &model)
    {
        // TODO
        return 0;
    }

    int Encoder::read_cplex_output(std::vector<int> &model)
    {
        std::string output_filename (m_file_name + ".out");
        gzFile of = gzopen(output_filename.c_str(), "rb");
        if (of == Z_NULL) {
            print_error_msg("Can't open file '" + output_filename + "' for reading");
            return -1;
        }
        StreamBuffer r(of);
        bool sat = false;
        model.resize(static_cast<size_t>(m_id_count + 1), 0);
        // set all variables to false, because we only get the variables that are true
        for (size_t v (1); v < m_id_count + 1; ++v)
            model[v] = -v;
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
                            model[l] = l;
                        }
                    }
                }
            }
        }
        if (!sat) model.clear();
        gzclose(of);
        return 0;
    }

    // if model.empty() in the end then unsat, else sat
    int Encoder::read_sat_output(std::vector<int> &model)
    {
        std::string output_filename (m_file_name + ".out");
        gzFile of = gzopen(output_filename.c_str(), "rb");
        if (of == Z_NULL) {
            print_error_msg("Can't open file '" + output_filename + "' for reading");
            return -1;
        }
        StreamBuffer r(of);
        bool sat = false;
        model.resize(static_cast<size_t>(m_id_count + 1), 0);
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
        if (!sat) model.clear();
        gzclose(of);
        return 0;
    }
    
    int Encoder::read_solver_output(std::vector<int> &model)
    {
        if (m_formalism == "wcnf" || m_formalism == "opb")
            return read_sat_output(model);
        else if (m_formalism == "lp") {
            if (m_lp_solver == "cplex")
                return read_cplex_output(model);
            else if (m_lp_solver == "gurobi")
                return read_gurobi_output(model);
            else if (m_lp_solver == "glpk")
                return read_glpk_output(model);
            else if (m_lp_solver == "scip")
                return read_scip_output(model);
            else if (m_lp_solver == "cbc")
                return read_cbc_output(model);
            else if (m_lp_solver == "lpsolve")
                return read_lpsolve_output(model);
        }
        return -1; // wrong formalism 
    }

    int Encoder::split_command(const std::string &command, std::vector<std::string> &command_split)
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

    int Encoder::call_solver(const std::string &solver_type)
    {
        std::string output_filename = m_file_name + ".out";
        const std::string error_filename = m_file_name + ".err";
        std::string command;
        if (solver_type == "optimisation") {
            if (m_opt_solver_cmd.empty()) {
                print_error_msg("Empty optimisation solver command");
                return -1;
            }
            command = m_opt_solver_cmd + " ";
        }
        else if (solver_type == "decision") {
            if (m_sat_solver_cmd.empty()) {
                print_error_msg("Empty SAT solver command");
                return -1;
            }
            command = m_sat_solver_cmd + " ";
        }
        else {
            print_error_msg("In Encoder::call_solver(), wrong solver_type");
            return -1;
        }
        if (m_formalism == "lp" && solver_type == "optimisation") { // TODO: set CPLEX parameters : number of threads, tolerance, etc.
            if (m_lp_solver == "cplex")
                command += "-c \"read " + m_file_name + "\" \"optimize\" \"display solution variables -\"";
            if (m_lp_solver == "cbc")
                command += m_file_name + " solve solution $";
            if (m_lp_solver == "scip")
                command += "-f " + m_file_name;
            if (m_lp_solver == "gurobi") {
                output_filename = m_file_name + ".sol"; // gurobi only accepts this file extension
                command = "gurobi_cl";
                command += " Threads=1 ResultFile=" + output_filename;
                command += " LogFile=\"\" LogToConsole=0 "; // disable logging
                command += m_file_name;
            }
        }
        else
            command += m_file_name;
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
        if (m_formalism != "lp" || m_lp_solver != "gurobi" || solver_type == "decision") {
            command += " > " + output_filename;
        }
        command += " 2> " + error_filename;
        system(command.c_str());
        // set to zero, i.e. no external solver is currently running
        m_child_pid = 0;
        return 0;
    }

    int Encoder::write_cnf_file(int i)
    {
        m_file_name += "_" + std::to_string(i) + ".cnf";
        std::ofstream out (m_file_name);
        if (!out) {
            print_error_msg("Can't open " + m_file_name + " for writing");
            return -1;
        }
        // print header
        out << "p cnf " << m_id_count << " " << m_constraints.size() << '\n';
        // print clauses
        for (const Clause *clause : m_constraints)
            print_clause(out, clause);
        out.close();
        return 0;
    }
    
    int Encoder::write_wcnf_file(int i)
    {
        m_file_name += "_" + std::to_string(i) + ".wcnf";
        std::ofstream out (m_file_name);
        if (!out) {
            print_error_msg("Can't open " + m_file_name + " for writing");
            return -1;
        }
        // prepare input for the solver
        size_t weight = m_soft_clauses.size() + 1;
        out << "p wcnf " << m_id_count << " " << m_constraints.size() << " " << weight << '\n';
        // print hard clauses
        print_wcnf_clauses(out, m_constraints, weight);
        // print soft clauses
        print_wcnf_clauses(out, m_soft_clauses, 1);
        out.close();
        return 0;
    }

    int Encoder::write_opb_file(int i)
    {
        m_file_name += "_" + std::to_string(i) + ".opb";
        std::ofstream out (m_file_name);
        if (!out.is_open()) {
            print_error_msg("Can't open " + m_file_name + " for writing");
            return -1;
        }
        // prepare input for the solver
        out << "* #variable= " << m_id_count;
        if (i == 0)
            out << " #constraint= " << m_constraints.size() << '\n';
        else
            out << " #constraint= " << m_constraints.size() + 1 << '\n'; // + 1 because of card. const.
        if (m_soft_clauses.size() > 0) {// print minimization function
            out << "min:";
            for (const Clause *cl : m_soft_clauses) {
                int soft_var = -(*(cl->begin())); // cl is unitary clause
                out << " " << "+1" << m_multiplication_string << "x" << soft_var;
            }
            out << ";\n";
        }
        // print all constraints except for cardinality constraint
        for (const Clause *cl : m_constraints) {
            print_pb_constraint(cl, out);
        }
        int last_j (i);
        if (m_simplify_last) {
            // last_j is min(i, m_num_objectives - 2)
            last_j = i < m_num_objectives - 2 ? i : m_num_objectives - 2;
        }
        for (int j (1); j <= last_j; ++j)
            print_atmost_pb(j, out);
        // if m_simplify_last, in the last iteration print =1 cardinality constraint
        if (i == m_num_objectives - 1 && m_num_objectives != 1 && m_simplify_last)
            print_sum_equals_pb(1, out);
        out.close();
        return 0;
    }

    int Encoder::write_solver_input(int i)
    {
        if (m_formalism == "wcnf")
            return write_wcnf_file(i);
        else if (m_formalism == "opb")
            return write_opb_file(i);
        else if (m_formalism == "lp") {
            if (m_lp_solver == "gurobi" || m_lp_solver == "scip")
                return write_opb_file(i);
            if (m_lp_solver == "cplex" || m_lp_solver == "cbc")
                return write_lp_file(i);
        }
        return -1; // wrong m_formalism - error msg already in set_formalism
    }

    int Encoder::external_solve(int i)
    {
        if (write_solver_input(i) != 0)
            return -1;
        // call the solver
        double initial_time (read_cpu_time());
        if (call_solver("optimisation") != 0) {
            if (!m_leave_temporary_files)
                remove_tmp_files();
            return -1;
        }
        double final_time (read_cpu_time());
        if (m_verbosity >= 1 && m_verbosity <= 2) {
            std::cout << "c Minimisation time: " << final_time - initial_time;
            std::cout << " seconds" << std::endl;
        }
        m_solver_output = true; // there is solver output to read
        // read output of solver
        std::vector<int> model;
        if (read_solver_output(model) != 0) {
            if (!m_leave_temporary_files)
                remove_tmp_files();
            return -1;
        }
        // check if there is already a solution (from a previous iteration for example)
        // if ext solver is killed before it finds a sol, the problem might not be unsat
        if (m_solution.empty()) {
            m_solution = std::move(model);
            m_sat = !(m_solution.empty());
        }
        else {
            if (!model.empty()) // if it is empty then something went wrong with ext solver
                m_solution = std::move(model);
            m_sat = true;
        }
        m_solver_output = false; // I have read solver output
        if (!m_leave_temporary_files)
            remove_tmp_files();
        // set m_file_name back to pid
        reset_file_name();
        return 0;
    }

    int Encoder::write_lp_file(int i)
    {
        m_file_name += "_" + std::to_string(i) + ".lp";
        std::ofstream output(m_file_name); 
        if (!output) {
            print_error_msg("Could not open " + m_file_name + " for writing");
            return -1;
        }
        // prepare input for the solver
        output << "Minimize\n";
        output << " obj: ";
        if (m_soft_clauses.size() > 0) {// print minimization function
            size_t nb_vars_in_line (0);
            for (size_t j (0); j < m_soft_clauses.size(); ++j) {
                const Clause *cl (m_soft_clauses[j]);
                int soft_var = -(*(cl->begin())); // cl is unitary clause
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
        for (const Clause *cl : m_constraints) {
            print_lp_constraint(cl, output);
        }
        int last_j (i);
        if (m_simplify_last) {
            // last_j is min(i, m_num_objectives - 2)
            last_j = i < m_num_objectives - 2 ? i : m_num_objectives - 2;
        }
        for (int j (1); j <= last_j; ++j)
            print_atmost_lp(j, output);
        // if m_simplify_last, in the last iteration print =1 cardinality constraint
        if (i == m_num_objectives - 1 && m_num_objectives != 1 && m_simplify_last)
            print_sum_equals_lp(1, output);
        // print all variables after Binaries
        output << "Binaries\n";
        for (int j (1); j <= m_id_count; ++j)
            output << "x" << j << '\n';
        output << "End";
        output.close();
        return 0;
    }
    
    void Encoder::remove_tmp_files() const
    {
        std::string output_filename (m_file_name);
        if (m_formalism == "lp" && m_lp_solver == "gurobi") {
            output_filename += ".sol";
        }
        else
            output_filename += ".out";
        std::string error_filename (m_file_name + ".err");
        remove(m_file_name.c_str());
        remove(output_filename.c_str());
        remove(error_filename.c_str());
    }
    
    int Encoder::sat_solve()
    {
        if (m_verbosity > 0 && m_verbosity <= 2)
            std::cout << "c Calling SAT solver..." << std::endl;
        if (write_cnf_file(0) != 0)
            return -1;
        // call the solver
        if (call_solver("decision") != 0) {
            if (!m_leave_temporary_files)
                remove_tmp_files();
            return -1;
        }
        m_solver_output = true; // there is solver output to read
        // read output of solver
        std::vector<int> model;
        if (read_sat_output(model) != 0) {
            if (!m_leave_temporary_files)
                remove_tmp_files();
            return -1;
        }
        m_solution = std::move(model);
        m_sat = !(m_solution.empty());
        m_solver_output = false; // I have read solver output
        if (!m_leave_temporary_files)
            remove_tmp_files();
        // set m_file_name back to pid
        reset_file_name();
        return 0;
    }

    // choose next variable in todo sequentially
    int Encoder::mss_choose_var_seq (std::vector<std::vector<int>> &todo_vec, std::vector<int> &obj_vector, int &obj_index)
    {
        if (todo_vec.empty()) {
            print_error_msg("In function mss_choose_var_seq(), todo_vec parameter is empty");
            exit(EXIT_FAILURE);
        }
        // search for the next todo variable sequentially (1st obj, 2nd obj, ...)
        obj_index = 0;
        auto todo_it (todo_vec.begin());
        while (todo_it->empty() && todo_it != todo_vec.end()) {
            ++todo_it;
            ++obj_index;
        }
        if (todo_it == todo_vec.end()) {
            print_error_msg("In function mss_choose_var_seq(), all todos are empty");
            exit(EXIT_FAILURE);
        }
        int next_var (todo_it->at(0));
        // erase next_var from todo by moving last element to next_var's position
        todo_it->at(0) = todo_it->back();
        todo_it->pop_back();
        // decrement obj_vector
        --obj_vector[obj_index];
        return next_var;
    }
    
    // choose next variable in todo from one of the objectives available with the maximum value
    int Encoder::mss_choose_var_max(std::vector<std::vector<int>> &todo_vec, std::vector<int> &obj_vector, int &obj_index)
    {
        if (todo_vec.empty()) {
            print_error_msg("In function mss_choose_var_max(), todo_vec parameter is empty");
            exit(EXIT_FAILURE);
        }
        // obj_index points to an objective whose todo is not empty
        for (obj_index = 0; obj_index < m_num_objectives; ++obj_index) {
            if (!todo_vec.at(obj_index).empty())
                break;
        }
        if (obj_index == m_num_objectives) {
            print_error_msg("In function mss_choose_var_max(), all todos are empty");
            exit(EXIT_FAILURE);
        }
        // and now obj_index will point to a non-empty obj with maximum value
        for (int i (0); i < m_num_objectives; ++i) {
            if (!todo_vec.at(i).empty() && obj_vector.at(i) > obj_vector.at(obj_index))
                obj_index = i;
        }
        int next_var (todo_vec.at(obj_index).at(0));
        // erase next_var from todo by moving last element to its position
        todo_vec.at(obj_index).at(0) = todo_vec.at(obj_index).back();
        todo_vec.at(obj_index).pop_back();
        return next_var;
    }
    
    // move falsified variables in todo to mss; decrement obj_vector accordingly
    void Encoder::add_falsified_to_mss (std::vector<int> &mss, std::vector<std::vector<int>> &todo_vec, std::vector<int> &obj_vector)
    {
        for (int j (0); j < m_num_objectives; ++j) {
            std::vector<int> &todo (todo_vec[j]);
            for (size_t i (0); i < todo.size(); ++i) {
                int var (todo.at(i));
                if (m_solution[var] < 0) {
                    mss.push_back(-var);
                    // decrement objective value
                    --obj_vector[j];
                    // erase element in todo by moving last element to current position
                    todo.at(i) = todo.back();
                    todo.pop_back();
                }
            }
        }    
    }
    
    int Encoder::mss_solve()
    {
        if (m_verbosity > 0 && m_verbosity <= 2)
            std::cout << "c Finding MSS..." << std::endl;
        /* TODO: see how this can behave with signals
         * Can something go wrong with the sat solver?*/
        IpasirWrap solver(m_id_count);
        for (const Clause *hard_cl : m_constraints)
            solver.addClause(hard_cl);
        bool is_sat = solver.solve();
        m_sat = is_sat;
        if (!m_sat)
            return 0;
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
        std::vector<int> mss;
        bool todo_is_empty (false);
        int obj_index (0);
        add_falsified_to_mss (mss, todo_vec, obj_vector);
        while (!todo_is_empty) {
            if (m_verbosity == 2)
                print_mss_and_todo(mss, todo_vec);
            /* choose var in todo using an heuristic
             * var is removed from todo
             * The obj value in obj_vector is decremented
             * obj_index is set to the objective that contains var */
            //int next_var (mss_choose_var_seq (todo_vec, obj_vector, obj_index));
            int next_var (mss_choose_var_max (todo_vec, obj_vector, obj_index));
            mss.push_back (-next_var);
            if (solver.solve(mss)) {
                m_solution = std::move(solver.model());
                solver.model().clear();
                add_falsified_to_mss (mss, todo_vec, obj_vector);
                if (m_verbosity > 0 && m_verbosity <= 2)
                    print_obj_vector(obj_vector);
            }
            else {
                mss.pop_back(); // remove last assumption from mss
                ++obj_vector[obj_index]; // correct obj value (it was decremented before)
            }
            // check if todo is empty
            todo_is_empty = true;
            for (const std::vector<int> &todo : todo_vec) {
                if (!todo.empty()) {
                    todo_is_empty = false;
                    break;
                }
            }
        }
        return 0;
    }
    
    int Encoder::calculate_upper_bound()
    {   
        if (m_verbosity > 0 && m_verbosity <= 2)
            std::cout << "c Presolving to obtain upper bound..." << std::endl;
        int retv (0);
        double initial_time (read_cpu_time());
        if (m_ub_encoding == 1) { // call sat solver once
            retv = sat_solve();
        }
        else if (m_ub_encoding == 2) { // calculate an MSS of the problem with sum of obj funcs 
            retv = mss_solve();
        }
        else if (m_ub_encoding == 3) { // get optimal solution of problem with sum of obj funcs 
            // TODO add negate all objective variables to form soft clauses and call external_solve(0)
            // be careful that external_solve may add cardinality constraints in non-zero iterations
        }
        double final_time (read_cpu_time());
        if (m_verbosity > 0 && m_verbosity <= 2) {
            std::cout << "c Time of Upper Bound Presolve: " << final_time - initial_time;
            std::cout << " seconds" << std::endl;
        }
        return retv;
    }
    
    double read_cpu_time()
    {
        struct rusage ru;
        getrusage(RUSAGE_SELF, &ru);
        double total_time ((double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1000000);
        getrusage(RUSAGE_CHILDREN, &ru);
        total_time += (double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1000000;
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
