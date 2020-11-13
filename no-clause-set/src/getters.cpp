#include <Leximax_encoder.h>

bool Leximax_encoder::get_sat() 
{
    return m_sat; 
}

size_t Leximax_encoder::get_sorting_net_size()
{
    return m_sorting_net_size; 
}

std::vector<LINT>& Leximax_encoder::get_solution() 
{ 
    return m_solution;
}

std::vector<LINT>& Leximax_encoder::get_optimum() 
{
    return m_optimum;
}
