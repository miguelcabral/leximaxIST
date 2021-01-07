#include <Leximax_encoder.h>

bool Leximax_encoder::get_sat() const
{
    return m_sat; 
}

size_t Leximax_encoder::get_sorting_net_size() const
{
    return m_sorting_net_size; 
}

const std::vector<LINT>& Leximax_encoder::get_solution() const 
{ 
    return m_solution;
}

std::vector<LINT> Leximax_encoder::get_objective_vector()  const
{
    std::vector<LINT> objective_vector;
    if (!m_solution.empty()) {
        for (std::vector<LINT> *obj_func : m_objectives) {
            LINT obj_value (0);
            for (LINT var : *obj_func) {
                if (m_solution[var] > 0)
                    ++obj_value;
            }
            objective_vector.push_back(obj_value);
        }
    }
    return objective_vector;
}
