#include <leximaxIST_Encoder.h>
#include <leximaxIST_printing.h>
#include <iostream>

namespace leximaxIST {
        
    Encoder::Encoder() : 
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
        m_pareto_presolve(false), // do not pareto presolve
        m_pareto_timeout(0.0),
        m_pareto_incremental(false),
        m_truly_pareto(false),
        m_mss_presolve(false), // do not enumerate msses
        m_mss_add_cls(1),
        m_mss_incremental(true),
        m_mss_timeout(0.0),
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
        // debug
         
        //std::cout << std::unitbuf;
         
    }

}/* namespace leximaxIST */
