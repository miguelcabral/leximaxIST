#include <Leximax_encoder.h>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <zlib.h>
#include <fmtutils.hh>
#include <errno.h> // for errno
#include <sys/wait.h>
#include <unistd.h>
#include <cstring> // for strerror()

int Leximax_encoder::read_gurobi_output(std::vector<LINT> &model)
{
    // TODO
    std::string output_filename (m_file_name + ".out");
    gzFile of = gzopen(output_filename.c_str(), "rb");
    if (of == Z_NULL) {
        std::string errmsg ("Could not open external solver output file '");
        errmsg += output_filename + "' for reading";
        print_error_msg(errmsg);
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
            const LINT var = parseInt(r);
            assert(model.size()>(size_t)var);
            ++r; // skip whitespace
            if (*r == '1')
                model[var] = var;
            else if (*r == '0')
                model[var] = -var;
            else {
                std::string errmsg ("Can not read gurobi output '" + output_filename);
                char current_char (*r);
                errmsg += "' - expecting '1' or '0' but instead got '" + current_char + "'";
            }
        }
    }
    if (!sat) model.clear();
    return 0;
}

int Leximax_encoder::read_glpk_output(std::vector<LINT> &model)
{
    // TODO
    return 0;
}

int Leximax_encoder::read_lpsolve_output(std::vector<LINT> &model)
{
    // TODO
    return 0;
}

int Leximax_encoder::read_scip_output(std::vector<LINT> &model)
{
    // TODO
    return 0;
}

int Leximax_encoder::read_cbc_output(std::vector<LINT> &model)
{
    // TODO
    return 0;
}

int Leximax_encoder::read_cplex_output(std::vector<LINT> &model)
{
    std::string output_filename (m_file_name + ".out");
    gzFile of = gzopen(output_filename.c_str(), "rb");
    if (of == Z_NULL) {
        std::string errmsg ("Could not open external solver output file '");
        errmsg += output_filename + "' for reading";
        print_error_msg(errmsg);
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
                        const LINT l = parseInt(r);
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

int Leximax_encoder::read_solver_output(std::vector<LINT> &model)
// TODO: Change this to a function that receives a std::vector<LINT> model and stores solution in that model.
//          Why? For flexibility - I can use this function to read an approximate solution that solver outputs (when program is interrupted)
{
    if (m_solver_format == "wcnf" || m_solver_format == "opb") {
        std::string output_filename (m_file_name + ".out");
        gzFile of = gzopen(output_filename.c_str(), "rb");
        if (of == Z_NULL) {
            std::string errmsg ("Could not open external solver output file '");
            errmsg += output_filename + "' for reading";
            print_error_msg(errmsg);
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
                    const LINT l = parseInt(r);
                    assert(model.size()>(size_t)l);
                    model[l] = (sign ? l : -l);
                }
                assert (*r=='\n');
                ++r; // skip '\n'
            }
        }
        if (!sat) model.clear();
    }
    else if (m_solver_format == "lp") {
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

int Leximax_encoder::call_solver()
{
    const std::string output_filename = m_file_name + ".out";
    const std::string error_filename = m + ".err";
    std::string command (m_solver_command + " ");
    if (m_solver_format == "lp") { // TODO: set CPLEX parameters : number of threads, tolerance, etc.
        if (m_lp_solver == "cplex")
            command += "-c \"read " + input_filename + "\" \"optimize\" \"display solution variables -\"";
        if (m_lp_solver == "cbc")
            command += input_filename + " solve solution $";
        if (m_lp_solver == "scip")
            command += "-f " + input_filename;
    }
    else
        command += input_filename;
    /*pid_t pid (fork());
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
            std::cout << "command_split: ";
    for (std::string &s : command_split)
        std::cout << s << ", ";
    std::cout << std::endl;
        // call solver
        std::string ola ("/bin/grep");
        if (execv(args[0]ola.c_str(), args.data()) == -1) {
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
    }  */
    command += " > " + output_filename;
    command += " 2> " + error_filename;
    system(command.c_str());
    // set to zero, i.e. no external solver is currently running
    m_child_pid = 0;
    return 0;
}

int Leximax_encoder::write_wcnf_file(int i)
{
    m_file_name += "_" + std::to_string(i) + ".opb";
    std::ofstream out (m_file_name);
    if (!out) {
        print_error_msg("Could not open " + file_name + " for writing");
        return -1;
    }
    // prepare input for the solver
    size_t weight = m_soft_clauses.size() + 1;
    output << "p wcnf " << m_id_count << " " << m_constraints.size() << " " << weight << '\n';
    // print hard clauses
    print_clauses(output, m_constraints, weight);
    // print soft clauses
    print_clauses(output, m_soft_clauses, 1);
    output.close();
    return 0;
}

int Leximax_encoder::write_opb_file(int i)
{
    m_file_name += "_" + std::to_string(i) + ".opb";
    std::ofstream out (m_file_name);
    if (!out) {
        print_error_msg("Could not open " + file_name + " for writing");
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
        for (Clause *cl : m_soft_clauses) {
            LINT soft_var = -(*(cl->begin())); // cl is unitary clause
            out << " " << "+1" << m_multiplication_string << "x" << soft_var;
        }
        out << ";\n";
    }
    // print all constraints except for cardinality constraint
    for (Clause *cl : m_constraints) {
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

int Leximax_encoder::write_solver_input(int i)
{
    if (m_solver_format == "wcnf")
        return write_wcnf_file(i);
    else if (m_solver_format == "opb")
        return write_opb_file(i);
    else if (m_solver_format == "lp") {
        if (m_lp_solver == "gurobi" || m_lp_solver == "scip")
            return write_opb_file(i);
        if (m_lp_solver == "cplex" || m_lp_solver == "cbc")
            return write_lp_file(i);
    }
    else
        return -1; // wrong m_solver_format - error msg already in set_solver_format
}

void Leximax_encoder::reset_file_name()
{
    // right now m_file_name looks like this: pid_i.wcnf
    while (m_file_name.back() != '_')
        m_file_name.pop_back();
    m_file_name.pop_back(); // remove '_'
}

int Leximax_encoder::external_solve(int i)
{
    if (write_solver_input(i) != 0)
        return -1;
    // call the solver
    if (call_solver() != 0)
        return -1;
    m_solver_output = true; // there is solver output to read
    // read output of solver
    if (read_solver_output(m_solution) != 0)
        return -1;
    m_solver_output = false; // I have read solver output
    // set m_file_name back to pid
    reset_file_name();
    if (!m_leave_temporary_files)
        remove_tmp_files();    // if it fails does not matter
    return 0;
}

int Leximax_encoder::write_lp_file(int i)
{
    m_file_name += "_" + std::to_string(i) + ".lp");
    std::ofstream output(file_name); 
    if (!output) {
        print_error_msg("Could not open " + file_name + " for writing");
        return -1;
    }
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

void Leximax_encoder::remove_tmp_files()
{
    std::string output_filename (m_file_name);
    if (m_solver_format == "lp" && m_lp_solver == "gurobi") {
        output_filename += ".sol";
    }
    else
        output_filename += ".out";
    std::string error_filename (m_file_name + ".err");
    std::string explanation ("File does not exist or some error occured.");
    if (remove(m_file_name.c_str()) != 0)
        print_error_msg("Failed to remove file: '" + m_file_name + "'. " + explanation);
    if (remove(output_filename.c_str()) != 0)
        print_error_msg("Failed to remove file: '" + output_filename + "'. " + explanation);
    if (remove(error_filename.c_str()) != 0)
        print_error_msg("Failed to remove file: '" + error_filename + "'. " + explanation);
}
