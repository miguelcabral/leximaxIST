#include <leximaxIST_Encoder.h>
#include <leximaxIST_error.h>
#include <string>
#include <climits>
#include <cstdlib>
#include <sys/types.h>
#include <unistd.h>

namespace leximaxIST {
    
    bool descending_order (int i, int j);

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
    
    void Encoder::set_ub_presolve(int val)
    {
        if (val < 0 || val > 2) {
            print_error_msg("Invalid value '" + std::to_string(val) + "' for ub_presolve parameter");
            exit(EXIT_FAILURE);
        }
        m_ub_presolve = val;
    }
    
    // Possible values: external, bin. TODO: linear-su, linear-us
    void Encoder::set_opt_mode(const std::string &mode)
    {
        if (mode != "external" && mode != "bin" && mode != "linear-su") {
            print_error_msg("Invalid optimisation mode: '" + mode + "'");
            exit(EXIT_FAILURE);
        }
        m_opt_mode = mode;
    }
    
    void Encoder::set_ext_solver_cmd(const std::string &command)
    {
        m_ext_solver_cmd = command;
    }
    
    void Encoder::set_formalism(const std::string &format)
    {
        if (format != "wcnf" && format != "opb" && format != "lp") {
            std::string msg ("The external solver formalism entered: '" + format + "' is not valid\n");
            msg += "Valid external solver formalisms: 'wcnf' 'opb' 'lp'";
            print_error_msg(msg);
            exit(EXIT_FAILURE);
        }
        m_formalism = format;  
    }

    void Encoder::set_timeout(double val) { m_timeout = val; }
    
    void Encoder::set_lp_solver(const std::string &lp_solver)
    {
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
            exit(EXIT_FAILURE);
        }
        m_lp_solver = lp_solver;
    }

    void Encoder::set_simplify_last(bool val) { m_simplify_last = val; }
    
    void Encoder::set_verbosity(int v)
    {
        if (v < 0 || v > 2) {
            std::string msg ("Invalid value '");
            msg += std::to_string(v) + "' for verbosity parameter";
            print_error_msg(msg);
            exit(EXIT_FAILURE);
        }
        m_verbosity = v;
    }
    
    void Encoder::set_leave_tmp_files(bool val) { m_leave_tmp_files = val; }

    void Encoder::set_multiplication_string(const std::string &str) { m_multiplication_string = str; }
    
    void Encoder::set_maxsat_presolve(bool v)
    {
        m_maxsat_presolve = v;
    }
    
    void Encoder::set_maxsat_psol_cmd(const std::string &cmd)
    {
        if (cmd.empty()) {
            print_error_msg("Empty MaxSAT presolve command");
            exit(EXIT_FAILURE);
        }
        m_maxsat_psol_cmd = cmd;
    }
    
    // set m_solution if model is better in the leximax order
    // in the case the external solver is killed and outputs a suboptimal solution
    void Encoder::set_solution(std::vector<int> &model)
    {
        if (m_solution.empty())
            m_solution.swap(model);
        else {
            std::vector<int> new_obj_vec (get_objective_vector(model));
            std::vector<int> old_obj_vec (get_objective_vector(m_solution));
            std::sort(new_obj_vec.begin(), new_obj_vec.end(), descending_order);
            std::sort(old_obj_vec.begin(), old_obj_vec.end(), descending_order);
            for (size_t j (0); j < new_obj_vec.size(); ++j) {
                if (new_obj_vec[j] < old_obj_vec[j]) { // model is better
                    m_solution.swap(model);
                    break;
                }
                else if (new_obj_vec[j] > old_obj_vec[j]) // m_solution is better
                    break; 
            }
        }
    }
    
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
        std::cout << "c ----------------------------------------------------------------------" << '\n';
    }
    
    void Encoder::set_problem(const std::vector<std::vector<int>> &constraints, const std::vector<std::vector<std::vector<int>>> &objective_functions)
    {
        if (m_verbosity > 0 && m_verbosity <= 2) {
            print_header();
            std::cout << "c Reading problem..." << '\n';
        }
        // restart object: if it has not been cleared from a previous problem
        clear();
        // name for temporary files
        reset_file_name();
        // check for trivial cases
        if (objective_functions.empty()) {
            print_error_msg("The problem does not have an objective function");
            exit(EXIT_FAILURE);
        }
        if (constraints.empty()) {
            print_error_msg("The problem does not have constraints");
            exit(EXIT_FAILURE);
        }
        m_num_objectives = objective_functions.size();
        if (m_num_objectives == 1) {
            print_error_msg("The problem is single-objective");
            exit(EXIT_FAILURE);
        }
        m_objectives.resize(m_num_objectives, nullptr);
        m_sorted_vecs.resize(m_num_objectives, nullptr);
        m_sat_solver = new IpasirWrap();
        // read problem
        if (m_verbosity == 2)
            std::cout << "c ------------- Input hard clauses -------------\n";
        for (const std::vector<int> &hard_clause : constraints) {
            // determine max id and update m_id_count
            update_id_count(hard_clause);
            // store clause (check if it is empty)
            add_hard_clause(hard_clause);
        }
        // update m_id_count if there are new variables in objective_functions
        for (const std::vector<std::vector<int>> &obj : objective_functions) {
            for (const std::vector<int> &soft_clause : obj)
                update_id_count(soft_clause);
        }
        m_input_nb_vars = m_id_count;
        if (m_verbosity > 0 && m_verbosity <= 2) {
            std::cout << "c Number of input variables: " << m_input_nb_vars << '\n';
            std::cout << "c Number of input hard clauses: " << m_hard_clauses.size() << '\n';
            std::cout << "c Number of objective functions: " << m_num_objectives << '\n';
        }
        // store objective functions - convert clause satisfaction maximisation to minimisation of sum of variables
        if (m_verbosity == 2)
            std::cout << "c ---- Input soft clauses conversion to variables ----\n";
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
                for (const int soft_lit : soft_clause)
                    add_hard_clause(-soft_lit, -fresh_var);
            }
            ++i;
        }
    }

}/* namespace leximaxIST */
