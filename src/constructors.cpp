#include <leximaxIST_Solver.h>
#include <leximaxIST_printing.h>
#include <iostream>

namespace leximaxIST {
        
    Solver::Solver() : 
        m_verbosity(0),
        m_id_count(0),
        m_input_nb_vars(0),
        m_num_objectives(0),
        m_formalism("wcnf"),
        m_lp_solver("gurobi"),
        m_valid_lp_solvers {"cplex", "gurobi", "glpk", "scip", "cbc", "lpsolve"},
        m_child_pid(0),
        m_timeout(3000.0), // 3 seconds
        m_leave_tmp_files(false),
        m_simplify_last(false),
        m_status('?'),
        m_approx("mss"), // default - mss enumeration
        m_approx_tout(86400), // 1 day I think
        m_gia_incr(false),
        m_gia_pareto(false),
        m_mss_add_cls(1),
        m_mss_incr(false),
        m_mss_nb_limit(0),
        m_mss_tolerance(50), // 50 percent
        m_maxsat_presolve(false), // do not maxsat presolve
        //m_num_opts(0),
        m_multiplication_string(" "),
        m_opt_mode("core-merge"),
        m_disjoint_cores(true),
        m_sat_solver(nullptr)
    {
        m_sat_solver = new IpasirWrap();
    }

}/* namespace leximaxIST */
