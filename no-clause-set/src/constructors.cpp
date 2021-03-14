#include <leximaxIST_Encoder.h>
#include <leximaxIST_error.h>

namespace leximaxIST {

    // this uses copy constructor
    int Encoder::add_hard_clause(const Clause &cl)
    {
        if (cl.empty()) {
            print_error_msg("Empty hard clause");
            return -1;
        }
        Clause *clptr (new Clause(cl)); // copy constructor
        m_constraints.push_back(clptr);
        return 0;
    }
    
    // this uses move constructor; cl is cleared
    int Encoder::add_hard_clause(Clause &&cl)
    {
        if (cl.empty()) {
            print_error_msg("Empty hard clause");
            return -1;
        }
        Clause *clptr (new Clause(cl)); // move constructor
        cl.clear(); // cl was in an unspecified, but valid state
        m_constraints.push_back(clptr);
        return 0;
    }

    int Encoder::add_soft_clause(const Clause &cl)
    {
        if (cl.empty()) {
            print_error_msg("Empty soft clause");
            return -1;
        }
        Clause *clptr (new Clause(cl));
        m_soft_clauses.push_back(clptr);
        return 0;
    }
    
    // this uses move constructor; cl is cleared
    int Encoder::add_soft_clause(Clause &&cl)
    {
        if (cl.empty()) {
            print_error_msg("Empty soft clause");
            return -1;
        }
        Clause *clptr (new Clause(cl)); // move constructor
        cl.clear(); // cl was in an unspecified, but valid state
        m_soft_clauses.push_back(clptr);
        return 0;
    }
        
    Encoder::Encoder() : 
        m_verbosity(0),
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
        m_solver_output(false),
        m_child_pid(0),
        m_timeout(3000.0), // 3 seconds
        m_leave_temporary_files(false),
        m_simplify_last(true),
        m_sat(false),
        m_sat_solver_cmd(), // for external call to sat solver
        m_ub_encoding(0), // 0 - no upper bound encoding; 1 - sat solver; 2 - MSS; 3 - MaxSAT/PBO/ILP solver
        //m_num_opts(0),
        m_multiplication_string(" "),
        m_solution(),
        m_sorting_net_size(0),
        m_ub_vec()
    {}

}/* namespace leximaxIST */
