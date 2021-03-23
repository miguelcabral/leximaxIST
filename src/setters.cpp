#include <leximaxIST_Encoder.h>
#include <leximaxIST_error.h>
#include <string>
#include <climits>
#include <cstdlib>
#include <sys/types.h>
#include <unistd.h>

namespace leximaxIST {

    int Encoder::fresh()
    {
        // check for overflow
        if (m_id_count == INT_MAX) {
            print_error_msg("The number of variables exceeded INT_MAX");
            exit(EXIT_FAILURE);
        }
        return ++m_id_count; 
    }
    
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
    
    int Encoder::set_verbosity(int v)
    {
        if (v < 0 || v > 2) {
            std::string msg ("Invalid value '");
            msg += std::to_string(v) + "' for verbosity parameter";
            print_error_msg(msg);
            return -1;
        }
        m_verbosity = v;
        return 0;
    }
    
    void Encoder::set_leave_temporary_files(bool val) { m_leave_temporary_files = val; }

    void Encoder::set_multiplication_string(const std::string &str) { m_multiplication_string = str; }
    
    void Encoder::update_id_count(const Clause &clause)
    {
        for (int lit : clause) {
            int var( lit < 0 ? -lit : lit );
            if (var > m_id_count)
                m_id_count = var;
        }
    }
    
    void print_header()
    {
        std::cout << "c ----------------------------------------------------------------------\n";
        std::cout << "c leximaxIST\n";
        std::cout << "c C++ library for leximax optimisation in a Boolean Satisfaction setting\n";
        std::cout << "c Authors: Miguel Cabral, Mikolas Janota, Vasco Manquinho\n";
        std::cout << "c ----------------------------------------------------------------------" << std::endl;
    }
    
    int Encoder::set_problem(const std::vector<std::vector<int>> &constraints, const std::vector<std::vector<std::vector<int>>> &objective_functions)
    {
        if (m_verbosity > 0 && m_verbosity <= 2) {
            print_header();
            std::cout << "c Reading problem..." << std::endl;
        }
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
        /*if (m_ub_encoding > 0)
            m_times.resize(m_num_objectives + 1, 0.0); // presolve, 1st iter, 2nd iter, ...
        else
            m_times.resize(m_num_objectives, 0.0); // no presolve*/
        m_objectives.resize(m_num_objectives, nullptr);
        m_sorted_vecs.resize(m_num_objectives, nullptr);
        // read problem
        for (const std::vector<int> &hard_clause : constraints) {
            // determine max id and update m_id_count
            update_id_count(hard_clause);
            // store clause (check if it is empty)
            if (add_hard_clause(hard_clause) != 0)
                return -1;
        }
        // update m_id_count if there are new variables in objective_functions
        for (const std::vector<std::vector<int>> &obj : objective_functions) {
            for (const std::vector<int> &soft_clause : obj)
                update_id_count(soft_clause);
        }
        if (m_verbosity > 0 && m_verbosity <= 2) {
            std::cout << "c Number of input variables: " << m_id_count << '\n';
            std::cout << "c Number of input hard clauses: " << m_constraints.size() << '\n';
            std::cout << "c Number of objective functions: " << m_num_objectives << std::endl;
        }
        // store objective functions - convert clause satisfaction maximisation to minimisation of sum of variables
        int i(0);
        for (const std::vector<std::vector<int>> &obj : objective_functions) {
            m_objectives[i] = new std::vector<int>(obj.size(), 0);
            int j(0);
            for (const std::vector<int> &soft_clause : obj) {
                // neg fresh_var implies soft_clause
                int fresh_var (fresh());
                Clause hard_clause (soft_clause); // copy constructor
                (m_objectives.at(i))->at(j) = fresh_var;
                hard_clause.push_back(fresh_var);
                add_hard_clause(hard_clause);
                hard_clause.clear();
                j++;
                // other implication: soft_clause implies neg fresh_var
                for (const int soft_lit : soft_clause) {
                    hard_clause.push_back(-soft_lit);
                    hard_clause.push_back(-fresh_var);
                    add_hard_clause(hard_clause);
                    hard_clause.clear();
                }
            }
            ++i;
        }
        return 0;
    }

}/* namespace leximaxIST */
