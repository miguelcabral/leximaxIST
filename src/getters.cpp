#include <leximaxIST_Encoder.h>
#include <vector>
#include <iostream>

namespace leximaxIST {

    /* int Encoder:: get_num_opts() const
    {
        return m_num_opts;
    }*/

    //const std::vector<double>& Encoder::get_times() const { return m_times; }

    char Encoder::get_status() const
    {
        return m_status; 
    }

    std::vector<int> Encoder::get_solution() const 
    {
        std::vector<int> assignment;
        if (m_status != 's')
            return assignment;
        assignment.resize(m_input_nb_vars + 1);
        for (int j (0); j <= m_input_nb_vars; ++j) {
            assignment.at(j) = m_solution.at(j);
        }
        return assignment;
    }

    // if unsat return empty vector
    std::vector<int> Encoder::get_objective_vector(const std::vector<int> &assignment) const
    {
        std::vector<int> objective_vector;
        if (!assignment.empty()) {
            for (const std::vector<int> &obj_func : m_objectives) {
                int obj_value (0);
                for (int var : obj_func) {
                    if (assignment.at(var) > 0)
                        ++obj_value;
                }
                objective_vector.push_back(obj_value);
            }
        }
        return objective_vector;
    }

    std::vector<int> Encoder::get_objective_vector() const
    {
        std::vector<int> objective_vector;
        if (!m_solution.empty()) {
            for (const std::vector<int> &obj_func : m_objectives) {
                int obj_value (0);
                for (int var : obj_func) {
                    if (m_solution[var] > 0)
                        ++obj_value;
                }
                objective_vector.push_back(obj_value);
            }
        }
        return objective_vector;
    }

}/* namespace leximaxIST */
