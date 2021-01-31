#include <leximaxIST_Encoder.h>
#include <leximaxIST_parsing_utils.h>
#include <zlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h> // for errno
#include <stdlib.h>
#include <stdio.h> // for fopen()
#include <fstream>
#include <cstring> // for strerror()
#include <iostream>
#include <vector>
#include <sstream>

namespace leximaxIST {

    int Encoder::read_gurobi_output(std::vector<long long> &model)
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
                const long long var = parseInt(r);
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
        return 0;
    }

    int Encoder::read_glpk_output(std::vector<long long> &model)
    {
        // TODO
        return 0;
    }

    int Encoder::read_lpsolve_output(std::vector<long long> &model)
    {
        // TODO
        return 0;
    }

    int Encoder::read_scip_output(std::vector<long long> &model)
    {
        // TODO
        return 0;
    }

    int Encoder::read_cbc_output(std::vector<long long> &model)
    {
        // TODO
        return 0;
    }

    int Encoder::read_cplex_output(std::vector<long long> &model)
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
                            const long long l = parseInt(r);
                            assert(model.size()>(size_t)l);
                            model[l] = l;
                        }
                    }
                }
            }
        }
        if (!sat) model.clear();   
        return 0;
    }

    int Encoder::read_solver_output(std::vector<long long> &model)
    {
        if (m_formalism == "wcnf" || m_formalism == "opb") {
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
                        const long long l = parseInt(r);
                        assert(model.size()>(size_t)l);
                        model[l] = (sign ? l : -l);
                    }
                    assert (*r=='\n');
                    ++r; // skip '\n'
                }
            }
            if (!sat) model.clear();
            return 0;
        }
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

    int Encoder::split_solver_command(const std::string &command, std::vector<std::string> &command_split)
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

    int Encoder::call_solver()
    {
        std::string output_filename = m_file_name + ".out";
        const std::string error_filename = m_file_name + ".err";
        std::string command (m_solver_command + " ");
        if (m_formalism == "lp") { // TODO: set CPLEX parameters : number of threads, tolerance, etc.
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
       /* pid_t pid (fork());
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
            if (split_solver_command(command, command_split) == -1)
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
        if (m_formalism != "lp" || m_lp_solver != "gurobi") {
            command += " > " + output_filename;
        }
        command += " 2> " + error_filename;
        system(command.c_str());
        // set to zero, i.e. no external solver is currently running
        m_child_pid = 0;
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
        print_clauses(out, m_constraints, weight);
        // print soft clauses
        print_clauses(out, m_soft_clauses, 1);
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
                long long soft_var = -(*(cl->begin())); // cl is unitary clause
                out << " " << "+1" << m_multiplication_string << "x" << soft_var;
            }
            out << ";\n";
        }
        // print all constraints except for cardinality constraint
        for (const Clause *cl : m_constraints) {
            print_pb_constraint(cl, out);
        }
        // write at most constraint for 1, 2, 3, ..., until min(i, m_num_objectives - 2)
        for (int j (1); j <= ( i < m_num_objectives - 2 ? i : m_num_objectives - 2 ); ++j)
            print_atmost_pb(j, out);
        if (i == m_num_objectives - 1 && m_num_objectives != 1)
            print_sum_equals_pb(1, out); // in the last iteration print =1 cardinality constraint
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

    void Encoder::reset_file_name()
    {
        // right now m_file_name looks like this: pid_i.wcnf
        while (m_file_name.back() != '_')
            m_file_name.pop_back();
        m_file_name.pop_back(); // remove '_'
    }

    int Encoder::external_solve(int i)
    {
        if (write_solver_input(i) != 0)
            return -1;
        // call the solver
        if (call_solver() != 0) {
            if (!m_leave_temporary_files)
                remove_all_tmp_files();
            return -1;
        }
        m_solver_output = true; // there is solver output to read
        // read output of solver
        std::vector<long long> model;
        if (read_solver_output(model) != 0) {
            if (!m_leave_temporary_files)
                remove_all_tmp_files();
            return -1;
        }
        m_solution = model;
        m_sat = !(m_solution.empty());
        m_solver_output = false; // I have read solver output
        if (!m_leave_temporary_files)
            remove_all_tmp_files();
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
                long long soft_var = -(*(cl->begin())); // cl is unitary clause
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
        // write at most constraint for 1, 2, 3, ..., until min(i, m_num_objectives - 2)
        for (int j (1); j <= ( i < m_num_objectives - 2 ? i : m_num_objectives - 2 ); ++j)
            print_atmost_lp(j, output);
        if (i == m_num_objectives - 1 && m_num_objectives != 1)
            print_sum_equals_lp(1, output); // in the last iteration print =1 cardinality constraint 
        // print all variables after Binaries
        output << "Binaries\n";
        for (size_t j (1); j <= m_id_count; ++j)
            output << "x" << j << '\n';
        output << "End";
        output.close();
        return 0;
    }

    void Encoder::remove_tmp_file(const std::string &filename) const
    {
        if (remove(filename.c_str()) != 0)
            print_error_msg("Can't remove temporary file: '" + filename + "'");
    }
    
    void Encoder::remove_all_tmp_files() const
    {
        std::string output_filename (m_file_name);
        if (m_formalism == "lp" && m_lp_solver == "gurobi") {
            output_filename += ".sol";
        }
        else
            output_filename += ".out";
        std::string error_filename (m_file_name + ".err");
        remove_tmp_file(m_file_name);
        remove_tmp_file(output_filename);
        remove_tmp_file(error_filename);
    }

} /* namespace leximaxIST */
