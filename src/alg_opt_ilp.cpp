#include <leximaxIST_Solver.h>
#include <leximaxIST_rusage.h>
#include <leximaxIST_printing.h>
#include <cassert>
#include <string>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>

namespace leximaxIST {
    
    class ILPConstraint {
    public:
        ILPConstraint(const std::vector<int> &vars,
            const std::vector<int> &coeffs,
            const std::string &sign,
            int rhs
        )
        m_vars(vars),
        m_coeffs(coeffs),
        m_sign(sign),
        m_rhs(rhs)
        {
            assert(m_sign == ">=" || m_sign == "<=" || m_sign == "=");
            assert(m_vars.size() == m_coeffs.size());
        }
        
        std::vector<int> m_vars;
        std::vector<int> m_coeffs;
        std::string m_sign; // can be either '<=', '>=' or '='
        int m_rhs;
        
        void print(std::ostream &os) const
        {
            std::string line ("");
            for (size_t i (0); i < m_vars.size(); ++i) {
                line += (m_coeffs.at(i) > 0) ? " + " : " - ";
                line += std::string(std::abs(m_coeffs.at(i)));
                line += " x" + m_vars.at(i) + " ";
                if (line.size() >= 80) {
                    os << line << '\n';
                    line = "";
                }
            }
            line += m_sign + " " + m_rhs;
        }
        
    }

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
        std::vector<int>
        for (int i (0); i < m_num_objectives; ++i) {
            if (m_verbosity >= 1)
                std::cout << "c Minimising the " << ordinal(i + 1) << " maximum...\n";
            // ith maximum
            int max_i (fresh());
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
                        std::cout << relax_vars.at(j) << ' '
                }
                if (m_verbosity >= 2)
                    std::cout << '\n';
            }
            // add constraints
            for (int j (0); j < m_num_objectives; ++j) {
                std::vector<int> constr_vars;
                std::vector<int> coeffs;
                constexpr std::string sign ("<=");
                constexpr int rhs (0);
                // add jth objective function variables
                for (int v : m_objectives.at(j)) {
                    constr_vars.push_back(constr_vars);
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
                if (m_verbosity >= 2) {
                    std::cout << "c Added ILP constraint:\n";
                    ilpc.print(std::cout);
                }
            }           
            // write lp file for gurobi
            write_lp_file(constraints, max_i);
            // call gurobi
            std::string command ("gurobi_cl");
            command += " Threads=1 ResultFile=" + m_file_name + ".sol";
            command += " LogFile= LogToConsole=0 "; // disable logging
            command += m_file_name + " &> " + m_file_name + ".err";;
            system(command.c_str());
            // read gurobi .sol file and update m_solution
            // read output of solver
            std::vector<int> model;
            read_solver_output(model);
            // if ext solver is killed before it finds a sol, the problem might not be unsat
            set_solution(model); // update solution and print obj vector
            if (!m_leave_tmp_files)
                remove_tmp_files();
            // set m_file_name back to pid
            reset_file_name();
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

} /* namespace leximaxIST */
