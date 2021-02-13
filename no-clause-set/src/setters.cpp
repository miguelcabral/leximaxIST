#include <leximaxIST_Encoder.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>

namespace leximaxIST {

    void Encoder::reset_file_name()
    {
        m_file_name = std::to_string(getpid());
    }
    
    int Encoder::set_ub_encoding(int val)
    {
        if (val < 0 || val > 3) {
            print_error_msg("Invalid value '" + std::to_string(val) + "' for ub_encoding parameter");
            return -1;
        }
        m_ub_encoding = val;
        return 0;
    }
    
    void Encoder::set_opt_solver_cmd(const std::string &command)
    {
        m_opt_solver_cmd = command;
    }

    void Encoder::set_sat_solver_cmd(const std::string &command)
    {
        m_sat_solver_cmd = command;
    }
    
    int Encoder::set_formalism(const std::string &format)
    {
        m_formalism = format;
        if (format != "wcnf" && format != "opb" && format != "lp") {
            std::string msg ("The external solver formalism entered: '" + format + "' is not valid\n");
            msg += "Valid external solver formalisms: 'wcnf' 'opb' 'lp'";
            print_error_msg(msg);
            return -1;
        }
        return 0;    
    }

    void Encoder::set_timeout(double val) { m_timeout = val; }
    
    int Encoder::set_lp_solver(const std::string &lp_solver)
    {
        m_lp_solver = lp_solver;
        bool found (false);
        for (std::string &valid_lp_solver : m_valid_lp_solvers)
            if (lp_solver == valid_lp_solver)
                found = true;
        if (!found) {
            std::string msg = "The lp solver name entered: '" + lp_solver + "' is not valid\n";
            msg += "Valid lp solvers: ";
            for (std::string &valid_lp_solver : m_valid_lp_solvers)
                msg += valid_lp_solver + ' ';
            print_error_msg(msg);
            return -1;
        }
        return 0;
    }

    void Encoder::set_simplify_last(bool val) { m_simplify_last = val; }
    
    void Encoder::set_leave_temporary_files(bool val) { m_leave_temporary_files = val; }

    void Encoder::set_multiplication_string(const std::string &str) { m_multiplication_string = str; }

    void Encoder::set_err_file(const std::string &name) { m_err_file = name; }
    
    void Encoder::update_id_count(const std::vector<long long> &clause)
    {
        for (long long lit : clause) {
            long long var( lit < 0 ? -lit : lit );
            if (var > m_id_count)
                m_id_count = var;
        }
    }
    
    int Encoder::set_problem(const std::vector<std::vector<long long>> &constraints, const std::vector<std::vector<std::vector<long long>>> &objective_functions)
    {
        // restart object: if it has not been cleared from a previous problem
        clear();
        // name for temporary files
        reset_file_name();
        // check for trivial cases
        if (objective_functions.empty()) {
            print_error_msg("The problem does not have an objective function");
            return -1;
        }
        if (constraints.empty()) {
            print_error_msg("The problem does not have constraints");
            return -1;
        }
        m_num_objectives = objective_functions.size();
        m_ub_vec.resize(m_num_objectives, -1);
        m_objectives.resize(m_num_objectives, nullptr);
        m_sorted_vecs.resize(m_num_objectives, nullptr);
        // read problem
        for (const std::vector<long long> &hard_clause : constraints) {
            // determine max id and update m_id_count
            update_id_count(hard_clause);
            // store clause (check if it is empty)
            if (add_hard_clause(hard_clause) != 0)
                return -1;
        }
        // update m_id_count if there are new variables in objective_functions
        for (const std::vector<std::vector<long long>> &obj : objective_functions) {
            for (const std::vector<long long> &soft_clause : obj)
                update_id_count(soft_clause);
        }
        // store objective functions - convert clause satisfaction maximisation to minimisation of sum of variables
        int i(0);
        for (const std::vector<std::vector<long long>> &obj : objective_functions) {
            m_objectives[i] = new std::vector<long long>(obj.size(), 0);
            std::vector<long long> lits;
            int j(0);
            for (const std::vector<long long> &soft_clause : obj) {
                // neg fresh_var implies soft_clause
                long long fresh_var(m_id_count + 1);
                m_id_count++;
                lits = soft_clause;
                (m_objectives.at(i))->at(j) = fresh_var;
                lits.push_back(fresh_var);
                add_hard_clause(lits);
                lits.clear();
                j++;
                // other implication: soft_clause implies neg fresh_var
                for (const long long soft_lit : soft_clause) {
                    lits.push_back(-soft_lit);
                    lits.push_back(-fresh_var);
                    add_hard_clause(lits);
                    lits.clear();
                }
            }
            ++i;
        }
        return 0;
    }

}/* namespace leximaxIST */
