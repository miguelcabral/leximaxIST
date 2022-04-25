#include <leximaxIST_Solver.h>
#include <leximaxIST_ILPConstraint.h>
#include <leximaxIST_rusage.h>
#include <leximaxIST_printing.h>
#include <string>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>

namespace leximaxIST {

    void Solver::optimise_ilp()
    {
        std::vector<ILPConstraint> constraints;
        // first copy the hard clauses to constraints
        assert(m_encoding.empty());
        for (const Clause &cl : m_input_hard) {
            int rhs (1);
            std::string sign (">=");
            std::vector<int> vars;
            std::vector<int> coeffs;
            // check negations and decrement rhs
            for (int l : cl) {
                if (l > 0) {
                    coeffs.push_back(1);
                    vars.push_back(l);
                }
                else {
                    coeffs.push_back(-1);
                    vars.push_back(-l);
                    --rhs;
                }
            }
            ILPConstraint ilpc (vars, coeffs, sign, rhs);
            constraints.push_back(ilpc);
        }        
        // next, iteratively solve single-objective by calling gurobi external executable
        // the constraints are added in each iteration
        std::vector<int> max_vars;
        for (int i (0); i < m_num_objectives; ++i) {
            if (m_verbosity >= 1)
                std::cout << "c Minimising the " << ordinal(i + 1) << " maximum...\n";
            // ith maximum
            int max_i (fresh());
            max_vars.push_back(max_i);
            if (m_verbosity >= 2) {
                std::cout << "c " << ordinal(i + 1) << " maximum integer variable: ";
                std::cout << max_i << '\n'; 
            }
            // relaxation variables
            std::vector<int> relax_vars;
            int bound (0);
            if (i > 0) {
                bound = ilp_bound();
                if (m_verbosity >= 2) {
                    std::cout << "c Relax Bound: " << bound << '\n';
                    std::cout << "c Relaxation variables: ";
                }
                for (int j (0); j < m_num_objectives; ++j) {
                    relax_vars.push_back(fresh());
                    if (m_verbosity >= 2)
                        std::cout << relax_vars.at(j) << ' ';
                }
                if (m_verbosity >= 2)
                    std::cout << '\n';
            }
            // add constraints
            for (int j (0); j < m_num_objectives; ++j) {
                std::vector<int> constr_vars;
                std::vector<int> coeffs;
                const std::string sign ("<=");
                constexpr int rhs (0);
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
                ILPConstraint ilpc (constr_vars, coeffs, sign, rhs);
                constraints.push_back(ilpc);
                /*if (m_verbosity >= 2) {
                    std::cout << "c Added ILP constraint:\n";
                    ilpc.print(std::cout);
                }*/
            }
            call_ilp_solver(constraints, max_vars, i);
            // fix ith maximum (add to constraints)
            std::vector<int> vars {max_i};
            std::vector<int> coeffs {1};
            std::string sign ("=");
            std::vector<int> obj_vec (get_objective_vector());
            if (obj_vec.empty())
                return;
            int rhs (obj_vec.at(i));
            ILPConstraint ilpc (vars, coeffs, sign, rhs);
            constraints.push_back(ilpc);
        }
    }
    
    /* returns the bound used in the relaxation of the constraints
     * at the moment it simply is an upper bound on the value of the 1st maximum
     * it may be possible to improve the bound, e.g. by using lower bounds on the sum of the objectives
     */
    int Solver::ilp_bound() const
    {
        const std::vector<int> obj_vec (get_objective_vector());
        return *std::max_element(obj_vec.begin(), obj_vec.end());
    }

} /* namespace leximaxIST */
