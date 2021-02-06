#include <leximaxIST_Encoder.h>

namespace leximaxIST {

    int Encoder::add_hard_clause(const Clause &lits)
    {
        if (lits.empty()) {
            print_error_msg("Empty hard clause");
            return -1;
        }
        Clause *cl (new Clause(lits));
        m_constraints.push_back(cl);
        return 0;
    }

    int Encoder::add_soft_clause(const Clause &lits)
    {
        if (lits.empty()) {
            print_error_msg("Empty soft clause");
            return -1;
        }
        Clause *cl (new Clause(lits));
        m_soft_clauses.push_back(cl);
        return 0;
    }
    
    
    Encoder::Encoder() : 
        m_debug(true),
        m_id_count(0),
        m_constraints(),
        m_soft_clauses(),
        m_objectives(),
        m_num_objectives(0),
        m_sorted_vecs(),
        m_sorted_relax_collection(),
        m_all_relax_vars(),
        m_opt_solver_cmd("rc2.py -vv"),
        m_formalism("wcnf"),
        m_lp_solver("gurobi"),
        m_valid_lp_solvers {"cplex", "gurobi", "glpk", "scip", "cbc", "lpsolve"},
        m_file_name(),
        m_err_file(),
        m_solver_output(false),
        m_child_pid(0),
        m_timeout(3000.0), // 3 seconds
        m_leave_temporary_files(false),
        m_sat(false),
        m_sat_solver_cmd(), // for external call to sat solver
        m_ub_encoding(0), // 0 - no upper bound encoding; 1 - sat solver; 2 - MSS; 3 - MaxSAT/PBO/ILP solver
        m_multiplication_string(" "),
        m_solution(),
        m_sorting_net_size(0)
    {}

}/* namespace leximaxIST */
