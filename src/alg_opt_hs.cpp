#include <leximaxIST_Solver.h>
#include <leximaxIST_ILPConstraint.h>
#include <leximaxIST_printing.h>
#include <leximaxIST_rusage.h>
#include <vector>
#include <string>
#include <cassert>
#include <algorithm>

namespace leximaxIST {
    
    bool descending_order (int i, int j);
    
    /* This algorithm is a generalisation of the algorithm of MaxHS to solve MaxSAT
     * Hybrid approach based on hitting sets - use SAT and ILP solvers
     * */
    void Solver::optimise_hs()
    {
        // TODO : Disjoint Cores Presolving with actual storage of the cores

        std::vector<std::vector<int>> cores;
        // find minimal (according to leximax) cost hitting set - call ILP solver
        // check if formula minus hitting set is satisfiable, if not get core
        // set variables in the hitting set to true and the remaining ones to false (as assumptions)
        std::vector<int> assumps;
		// first try to satisfy all soft clauses in assumps
		for (const auto &obj : m_objectives) {
			for (int v : obj)
				assumps.push_back(-v);
		}
        while (!call_sat_solver(m_sat_solver, assumps)) {
            std::vector<int> core (m_sat_solver->conflict());
            cores.push_back(core);
            if (m_verbosity >= 2)
                print_core(core);
            if (m_verbosity >= 1)
                std::cout << "c Core size: " << core.size() << '\n';
            if (m_verbosity >= 1)
                std::cout << "c Finding leximax minimum cost hitting set...\n";
            const double initial_time (read_cpu_time());
            find_minimum_hs(assumps, cores);
            if (m_verbosity >= 1)
                print_time(read_cpu_time() - initial_time, "c Hitting Set (ILP) CPU time: ");
			return; // NOTE: This is for debugging purposes only, please comment or remove
        }
    }
    
    /* Find leximax minimum cost hitting set of the set of cores
     * Call ILP solver multiple times
     * Each core is a list of literals
     * We use a hash table to associate each of those literals with a fresh variable
     */
    void Solver::find_minimum_hs(std::vector<int> &assumps, const std::vector<std::vector<int>> &cores)
    {
        std::vector<int> model;
        std::vector<ILPConstraint> constraints;
        // add hard constraints - for each core choose at least one literal
        for (const std::vector<int> &core : cores) {
            std::vector<int> vars (core.size(), 0);
            std::vector<int> coeffs (core.size(), 0);
            for (size_t k (0); k < core.size(); ++k) {
                if (core.at(k) > 0) {
                    vars.at(k) = core.at(k);
                    coeffs.at(k) = 1;
                }
                else {
                    vars.at(k) = - core.at(k);
                    coeffs.at(k) = -1;
                }
            }
            ILPConstraint ilpc (vars, coeffs, ">=", 1);
            constraints.push_back(ilpc);
        }
        // next, iteratively solve single-objective by calling gurobi/cplex external executable
        // the constraints are added in each iteration
        std::vector<int> max_vars;
        for (int i (0); i < m_num_objectives; ++i) {
            if (m_verbosity >= 2)
                std::cout << "c Minimising the " << ordinal(i + 1) << " maximum...\n";
            // ith maximum
            int max_i (fresh());
            max_vars.push_back(max_i);
            if (m_verbosity >= 2) {
                std::cout << "c " << ordinal(i + 1) << " maximum integer variable: ";
                std::cout << max_i << '\n'; 
            }
            // relaxation variables
            std::list<int> relax_vars;
            int bound (0);
            if (i > 0) {
                const std::vector<int> obj_vec (get_objective_vector(model));
                bound = *std::max_element(obj_vec.begin(), obj_vec.end());
                if (m_verbosity >= 2) {
                    std::cout << "c Relax Bound: " << bound << '\n';
                    std::cout << "c Relaxation variables: ";
                }
                for (int j (0); j < m_num_objectives; ++j) {
                    relax_vars.push_back(fresh());
                    if (m_verbosity >= 2)
                        std::cout << relax_vars.back() << ' ';
                }
                if (m_verbosity >= 2)
                    std::cout << '\n';
            }
            // add bound constraints
            for (int j (0); j < m_num_objectives; ++j) {
                std::vector<int> constr_vars;
                std::vector<int> coeffs;
                const int rhs (0);
                // add jth objective function variables
                for (int v : m_objectives.at(j)) {
                    constr_vars.push_back(v);
                    coeffs.push_back(1);
                }
                // add ith maximum integer var
                constr_vars.push_back(max_i);
                coeffs.push_back(-1);
                // add relaxation term if i > 0
                if (i > 0) {
                    constr_vars.push_back(relax_vars.at(j));
                    coeffs.push_back(-bound);
                }
                ILPConstraint ilpc (constr_vars, coeffs, "<=", rhs);
                constraints.push_back(ilpc);
            }
            // add atmost i constraint setting the number of satisfied relaxation variables
            if (i > 0) {
                std::vector<int> vars;
                std::vector<int> coeffs (m_num_objectives, 1);
                const int rhs (i);
                for (int v : relax_vars)
                    vars.push_back(v);
                ILPConstraint ilpc (vars, coeffs, "<=", rhs);
                constraints.push_back(ilpc);
            }
            model = call_ilp_solver(constraints, max_vars, i);
            // fix ith maximum (add to constraints)
            std::vector<int> vars {max_i};
            std::vector<int> coeffs {1};
            std::vector<int> obj_vec (get_objective_vector(model));
            assert(!obj_vec.empty());
            std::sort(obj_vec.begin(), obj_vec.end(), descending_order);
            const int rhs (obj_vec.at(i));
            ILPConstraint ilpc (vars, coeffs, "=", rhs);
            constraints.push_back(ilpc);
        }
        gen_assumps_hs(model, assumps); 
        reset_id_count();
    }
    
    void Solver::gen_assumps_hs(const std::vector<int> &model, std::vector<int> &assumps) const
    {
        assumps.clear();
        // go through all objective variables
        for (const std::vector<int> &objective : m_objectives) {
            for (int v : objective)
                assumps.push_back(model.at(v));
        }
		if (m_verbosity >= 2) {
			std::cout << "c ------------------ Assumptions ------------------\n";
			std::string line ("c ");
			for (int v : assumps) {
				line += std::to_string(v) + " ";
				if (line.size() >= 80) {
					std::cout << line << '\n';
					line.clear();	
				}
			}
		}
    }

} // namespace leximaxIST

