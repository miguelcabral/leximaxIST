#include <leximaxIST_Encoder.h>
#include <vector>

namespace leximaxIST {

    /* int Encoder:: get_num_opts() const
    {
        return m_num_opts;
    }*/

    //const std::vector<double>& Encoder::get_times() const { return m_times; }

    bool Encoder::get_sat() const
    {
        return m_sat; 
    }

    const std::vector<int>& Encoder::get_ub_vec() const
    {
        return m_ub_vec;
    }

    size_t Encoder::get_sorting_net_size() const
    {
        return m_sorting_net_size; 
    }

    const std::vector<int>& Encoder::get_solution() const 
    { 
        return m_solution;
    }

    // if unsat return empty vector
    std::vector<int> Encoder::get_objective_vector(const std::vector<int> &assignment) const
    {
        std::vector<int> objective_vector;
        if (!assignment.empty()) {
            for (std::vector<int> *obj_func : m_objectives) {
                int obj_value (0);
                for (int var : *obj_func) {
                    if (assignment[var] > 0)
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
            for (std::vector<int> *obj_func : m_objectives) {
                int obj_value (0);
                for (int var : *obj_func) {
                    if (m_solution[var] > 0)
                        ++obj_value;
                }
                objective_vector.push_back(obj_value);
            }
        }
        return objective_vector;
    }

}/* namespace leximaxIST */
