#include <leximaxIST_Encoder.h>
#include <vector>

namespace leximaxIST {

    bool Encoder::get_sat() const
    {
        return m_sat; 
    }

    size_t Encoder::get_sorting_net_size() const
    {
        return m_sorting_net_size; 
    }

    const std::vector<long long>& Encoder::get_solution() const 
    { 
        return m_solution;
    }

    // if unsat return empty vector
    std::vector<long long> Encoder::get_objective_vector(const std::vector<long long> &assignment) const
    {
        std::vector<long long> objective_vector;
        if (!assignment.empty()) {
            for (std::vector<long long> *obj_func : m_objectives) {
                long long obj_value (0);
                for (long long var : *obj_func) {
                    if (assignment[var] > 0)
                        ++obj_value;
                }
                objective_vector.push_back(obj_value);
            }
        }
        return objective_vector;
    }

    std::vector<long long> Encoder::get_objective_vector() const
    {
        std::vector<long long> objective_vector;
        if (!m_solution.empty()) {
            for (std::vector<long long> *obj_func : m_objectives) {
                long long obj_value (0);
                for (long long var : *obj_func) {
                    if (m_solution[var] > 0)
                        ++obj_value;
                }
                objective_vector.push_back(obj_value);
            }
        }
        return objective_vector;
    }

}/* namespace leximaxIST */
