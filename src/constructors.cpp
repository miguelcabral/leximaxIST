#include <leximaxIST_Encoder.h>
#include <leximaxIST_error.h>

namespace leximaxIST {

    // this uses copy constructor
    void Encoder::add_hard_clause(const Clause &cl)
    {
        if (cl.empty()) {
            print_error_msg("Empty hard clause");
            exit(EXIT_FAILURE);
        }
        Clause *clptr (new Clause(cl)); // copy constructor
        m_hard_clauses.push_back(clptr);
        m_sat_solver->addClause(clptr);
    }

    const Clause* Encoder::add_hard_clause(int l)
    {
        Clause *clptr (new Clause({l}));
        m_hard_clauses.push_back(clptr);
        m_sat_solver->addClause(l);
        return clptr;
    }
    
    const Clause* Encoder::add_hard_clause(int l1, int l2)
    {
        Clause *clptr (new Clause({l1, l2}));
        m_hard_clauses.push_back(clptr);
        m_sat_solver->addClause(l1, l2);
        return clptr;
    }
    
    const Clause* Encoder::add_hard_clause(int l1, int l2, int l3)
    {
        Clause *clptr (new Clause({l1, l2, l3}));
        m_hard_clauses.push_back(clptr);
        m_sat_solver->addClause(l1, l2, l3);
        return clptr;
    }
        
    Encoder::Encoder() : 
        m_verbosity(0),
        m_id_count(0),
        m_num_objectives(0),
        m_formalism("wcnf"),
        m_lp_solver("gurobi"),
        m_valid_lp_solvers {"cplex", "gurobi", "glpk", "scip", "cbc", "lpsolve"},
        m_child_pid(0),
        m_timeout(3000.0), // 3 seconds
        m_leave_tmp_files(false),
        m_simplify_last(false),
        m_status('?'),
        m_ub_presolve(0), // 0 - don't presolve; 1 - sat solver; 2 - MSS-seq; 3 - MSS-max
        //m_num_opts(0),
        m_multiplication_string(" "),
        m_sorting_net_size(0),
        m_opt_mode("bin"),
        m_sat_solver(nullptr)
    {}

}/* namespace leximaxIST */
