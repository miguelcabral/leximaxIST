#include <leximaxIST_Solver.h>
#include <leximaxIST_ILPConstraint.h>

namespace leximaxIST {
    
    /* This algorithm is a generalisation of the algorithm of MaxHS to solve MaxSAT
     * Hybrid approach based on hitting sets - use SAT and ILP solvers
     * */
    void Solver::optimise_hs()
    {
        std::vector<std::vector<int>> cores;
        // find minimal (according to leximax) cost hitting set - call ILP solver
        std::vector<std::vector<int>> hitting_set; // variables by objective
        // check if formula minus hitting set is satisfiable, if not get core
        // set variables in the hitting set to true (as assumptions)
        std::vector<int> assumps;
        // satisfy variables in the hitting set and falsify the remaining ones
        gen_assumps_hs(hitting_set, assumps);
        while (!call_sat_solver(m_sat_solver, assumps)) {
            std::vector<int> core (solver->conflict());
            cores.push_back(core);
            find_minimum_hs(hitting_set, cores);
            gen_assumps_hs(hitting_set, assumps);
        }
    }
    
    void Solver::find_minimum_hs(std::vector<std::vector<int>> &hitting_set, const std::vector<std::vector<int>> &cores)
    {
        std::vector<ILPConstraint> constraints;
        // add hard constraints - for each core at least one variable must be chosen
        //for ()
    }

} // namespace leximaxIST

