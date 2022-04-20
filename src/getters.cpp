#include <leximaxIST_Solver.h>
#include <vector>
#include <iostream>

namespace leximaxIST {

    /* int Solver::get_num_opts() const
    {
        return m_num_opts;
    }*/

    //const std::vector<double>& Solver::get_times() const { return m_times; }

    int Solver::nVars() const { return m_id_count; }
    
    char Solver::get_status() const
    {
        return m_status; 
    }

    std::vector<int> Solver::get_solution() const 
    {
        std::vector<int> assignment;
        if (m_status == 'u' || m_status == '?')
            return assignment;
        assignment.resize(m_input_nb_vars + 1);
        for (int j (0); j <= m_input_nb_vars; ++j) {
            assignment.at(j) = m_solution.at(j);
        }
        return assignment;
    }

    // if unsat return empty vector
    std::vector<int> Solver::get_objective_vector(const std::vector<int> &assignment) const
    {
        std::vector<int> objective_vector;
        if (!assignment.empty()) {
            for (int i (0); i < m_num_objectives; ++i) {
                const PBObjFunction &obj_func (m_formula.getObjFunction(i));
                int obj_value (obj_func._const);
                for (size_t j (0); j < obj_func._lits.size(); ++j) {
                    int var (std::abs(obj_func._lits.at(j)));
                    int val (obj_func._coeffs.at(j));
                    val *= assignment.at(var) > 0 ? 1 : -1;
                    val *= obj_func._lits.at(j) > 0 ? 1 : -1;
                    obj_value += val;
                }
                objective_vector.push_back(obj_value);
            }
        }
        return objective_vector;
    }

    std::vector<int> Solver::get_objective_vector() const
    {
        return get_objective_vector(m_solution);
    }

}/* namespace leximaxIST */
