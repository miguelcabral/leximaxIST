#include <leximaxIST_Solver.h>
#include <leximaxIST_printing.h>
#include <leximaxIST_rusage.h>
#include <string>
#include <iostream>
#include <algorithm> // std::sort
#include <climits>
#include <cstdlib>
#include <sys/types.h>
#include <unistd.h>

namespace leximaxIST {
    
    bool descending_order (int i, int j);
    
    // set_of_clauses can be m_input_hard for input hard clauses or m_encoding for encoding hard clauses
    void Solver::add_clause(const Clause &cl, std::vector<Clause> &set_of_clauses)
    {
        if (cl.empty()) {
            print_error_msg("Empty hard clause");
            exit(EXIT_FAILURE);
        }
        update_id_count(cl);
        set_of_clauses.push_back(cl);
        if (m_verbosity == 2)
            print_clause(std::cout, cl, "c ");
    }
    
    // this is public, one can use it to add input hard clauses
    void Solver::add_hard_clause(const Clause &cl)
    {
        if (m_sat_solver == nullptr)
            m_sat_solver = new IpasirWrap();
        add_clause(cl, m_input_hard);
        m_sat_solver->addClause(cl);
        // update status - if unsat it remains unsat, otherwise set to unknown
        if (m_status != 'u')
            m_status = '?';
    }
    
    void Solver::add_clause_enc(const Clause &cl)
    {
        add_clause(cl, m_encoding);
        // In 'core-rebuild' we create a new ipasir solver everytime the sorting networks grow
        // is this really necessary ?  maybe I can remove the if
        if (m_opt_mode != "core-rebuild")
            m_sat_solver->addClause(cl);
    }

    void Solver::add_clause(int l)
    {
        const Clause cl {l};
        add_clause_enc(cl);
    }
    
    void Solver::add_clause(int l1, int l2)
    {
        const Clause cl {l1, l2};
        add_clause_enc(cl);
    }
    
    void Solver::add_clause(int l1, int l2, int l3)
    {
        const Clause cl {l1, l2, l3};
        add_clause_enc(cl);
    }
    
    int Solver::fresh()
    {
        // check for overflow
        if (m_id_count == INT_MAX) {
            print_error_msg("The number of variables exceeded INT_MAX");
            exit(EXIT_FAILURE);
        }
        return ++m_id_count; 
    }
    
    void Solver::reset_file_name()
    {
        m_file_name = std::to_string(getpid());
    }
    
    void Solver::set_opt_mode(const std::string &mode)
    {
        if (mode != "external" && mode != "bin" && mode != "lin_su" &&
            mode != "lin_us" && mode != "core_static" && mode != "core_merge"
            && mode != "core_rebuild" && mode != "core_rebuild_incr" && mode != "ilp") {
            print_error_msg("Invalid optimisation mode: '" + mode + "'");
            exit(EXIT_FAILURE);
        }
        m_opt_mode = mode;
    }
    
    void Solver::set_approx(const std::string &algorithm)
    {
        if (algorithm != "mss" && algorithm != "gia") {
            std::string msg ("In function leximaxIST::Solver::set_approx, ");
            msg += "Invalid approximation algorithm: '" + algorithm + "'";
            print_error_msg(msg);
            exit(EXIT_FAILURE);
        }
        m_approx = algorithm;
    }
    
    void Solver::set_ext_solver_cmd(const std::string &command)
    {
        m_ext_solver_cmd = command;
    }
    
    void Solver::set_formalism(const std::string &format)
    {
        if (format != "wcnf" && format != "opb" && format != "lp") {
            std::string msg ("The external solver formalism entered: '" + format + "' is not valid\n");
            msg += "Valid external solver formalisms: 'wcnf' 'opb' 'lp'";
            print_error_msg(msg);
            exit(EXIT_FAILURE);
        }
        m_formalism = format;  
    }

    void Solver::set_timeout(double val) { m_timeout = val; }
    
    void Solver::set_lp_solver(const std::string &lp_solver)
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

    void Solver::set_simplify_last(bool val) { m_simplify_last = val; }
    
    void Solver::set_verbosity(int v)
    {
        if (v < 0 || v > 2) {
            std::string msg ("Invalid value '");
            msg += std::to_string(v) + "' for verbosity parameter";
            print_error_msg(msg);
            exit(EXIT_FAILURE);
        }
        m_verbosity = v;
    }
    
    void Solver::set_leave_tmp_files(bool val) { m_leave_tmp_files = val; }

    void Solver::set_multiplication_string(const std::string &str) { m_multiplication_string = str; }
    
    void Solver::set_maxsat_presolve(bool v)
    {
        m_maxsat_presolve = v;
    }
    
    void Solver::set_maxsat_psol_cmd(const std::string &cmd)
    {
        if (cmd.empty()) {
            print_error_msg("Empty MaxSAT presolve command");
            exit(EXIT_FAILURE);
        }
        m_maxsat_psol_cmd = cmd;
    }
    
    std::vector<int> Solver::set_solution(std::vector<int> &model)
    {
        const std::vector<int> &new_obj_vec (get_objective_vector(model));
        const std::vector<int> &old_obj_vec (get_objective_vector(m_solution));
        if (model.empty())
            return old_obj_vec;
        if (m_solution.empty()) {
            m_solution.swap(model);
            model.clear();
            if (m_verbosity >= 1) {
                print_time(read_cpu_time(), "c Leximax-better solution found: ");
                print_obj_vector(new_obj_vec);
            }
            return new_obj_vec;
        }
        std::vector<int> s_new_obj_vec (new_obj_vec);
        std::vector<int> s_old_obj_vec (old_obj_vec);
        std::sort(s_new_obj_vec.begin(), s_new_obj_vec.end(), descending_order);
        std::sort(s_old_obj_vec.begin(), s_old_obj_vec.end(), descending_order);
        for (size_t j (0); j < s_new_obj_vec.size(); ++j) {
            if (s_new_obj_vec.at(j) < s_old_obj_vec.at(j)) { // model is better
                m_solution.swap(model);
                model.clear();
                if (m_verbosity >= 1) {
                    print_time(read_cpu_time(), "c Leximax-better solution found: ");
                    print_obj_vector(new_obj_vec);
                }
                return new_obj_vec;
            }
            else if (s_new_obj_vec.at(j) > s_old_obj_vec.at(j)) { // m_solution is better
                model.clear();
                return old_obj_vec;
            }
        }
        return old_obj_vec; // the obj vecs are leximax-equal
    }
    
    // set m_id_count to the maximum id without the encoding - m_input_nb_vars + obj vars
    void Solver::reset_id_count()
    {
        m_id_count = m_input_nb_vars;
        for (const std::vector<int> &objective : m_objectives) {
            m_id_count += objective.size();
        }
        if (m_verbosity == 2)
            std::cout << "c Resetting m_id_count... m_id_count = " << m_id_count << '\n';
    }
    
    void Solver::update_id_count(const Clause &clause)
    {
        for (int lit : clause) {
            int var( lit < 0 ? -lit : lit );
            if (var > m_id_count)
                m_id_count = var;
        }
    }
    
    // add an objective function in the form of a set of soft clauses (so the goal is to minimise clause falsification)
    void Solver::add_soft_clauses(const std::vector<Clause> &soft_clauses)
    {
        if (soft_clauses.empty()) {
            print_error_msg("In function leximaxIST::Solver::add_soft_clauses, empty objective function");
            exit(EXIT_FAILURE);
        }
        ++m_num_objectives;
        // set m_snet_info to a vector of (0,0) pairs
        m_snet_info.resize(m_num_objectives, std::pair(0,0));
        m_objectives.resize(m_num_objectives);
        m_sorted_vecs.resize(m_num_objectives);
        // set m_all_relax_vars to a vector of empty lists
        m_all_relax_vars.resize(m_num_objectives);
        // set m_sorted_relax_collection to a vector of empty vectors
        m_sorted_relax_collection.resize(m_num_objectives);
        for (const Clause &soft_clause : soft_clauses)
            update_id_count(soft_clause);
        // convert clause satisfiaction maximisation to minimisation of sum of variables
        if (m_verbosity == 2)
            std::cout << "c ---- Input soft clauses conversion to variables ----\n";
        int i (m_num_objectives - 1); // position in m_objectives of the current objective
        for (const std::vector<int> &soft_clause : soft_clauses) {
            // neg fresh_var implies soft_clause
            int fresh_var (fresh());
            Clause hard_clause (soft_clause); // copy constructor
            m_objectives.at(i).push_back(fresh_var);
            hard_clause.push_back(fresh_var);
            add_hard_clause(hard_clause);
            // other implication: soft_clause implies neg fresh_var
            for (const int soft_lit : soft_clause) {
                Clause cl {-soft_lit, -fresh_var};
                add_hard_clause(cl);
            }
        }
        // update status - if optimum found then it becomes sat, otherwise status is not changed
        if (m_status == 'o')
            m_status = 's';
    }
        
    void Solver::set_gia_incr(bool v) { m_gia_incr = v; }
    
    void Solver::set_gia_pareto(bool v) { m_gia_pareto = v; }
    
    void Solver::set_mss_add_cls(int v)
    {
        if (v < 0 || v > 2) {
            std::string msg("Solver::set_mss_add_cls - invalid argument '");
            msg += std::to_string(v) + "'. The parameter must be in the range 0..2";
            print_error_msg(msg);
        }
        m_mss_add_cls = v;
    }
    
    void Solver::set_mss_incr(bool v) { m_mss_incr = v; }
    
    void Solver::set_approx_tout(double t)
    {
        if (t <= 0) {
            std::string msg ("Solver::set_approx_tout - argument '");
            msg += std::to_string(t) + "' is not positive";
            print_error_msg(msg);
            exit(EXIT_FAILURE);
        }
        m_approx_tout = t;
    }
        
    void Solver::set_mss_nb_limit(int n) 
    {
        m_mss_nb_limit = n; 
    }
    
    void Solver::set_mss_tol(int t)
    {
        if (t < 0 || t > 100) {
            std::string msg ("Solver::set_mss_tol - argument '");
            msg += std::to_string(t) + "' is not an integer between 0 and 100!";
            print_error_msg(msg);
            exit(EXIT_FAILURE);
        }
        m_mss_tolerance = t; 
    }
    
    void Solver::set_disjoint_cores(bool v) { m_disjoint_cores = v; }
    
}/* namespace leximaxIST */
