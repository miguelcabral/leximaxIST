#include "Leximax_encoder.h"

Leximax_encoder::Leximax_encoder(Options &options) :
    m_id_count(0),
    m_constraints(),
    m_soft_clauses(),
    m_objectives(options.m_num_objectives, nullptr),
    m_num_objectives(options.m_num_objectives),
    m_sorted_vecs(options.m_num_objectives, nullptr),
    m_sorted_relax_vecs(options.m_num_objectives, nullptr),
    m_relax_vars(),
    m_solver_command(options.m_solver),
    m_input_files(options.m_input_files),
    m_input_name(),
    m_leave_temporary_files(options.m_leave_temporary_files),
    m_sat(true),
    m_pbo(options.m_pbo),
    m_debug(false),
    m_multiplication_string(options.m_multiplication_string),
    m_optimum(options.m_num_objectives, 0),
    m_solution(),
    m_sorting_net_size(0),
    // verification:
    m_pienum_file_name()        
{
    // input name
    m_input_name = options.m_input_files[0];
    size_t position = m_input_name.find_last_of("/\\");
    m_input_name = m_input_name.substr(position + 1);
    for (int i{1}; i < m_num_objectives + 1; ++i) {
        m_input_name.append("_");
        std::string next_file = options.m_input_files[i];
        position = next_file.find_last_of("/\\");
        next_file = next_file.substr(position + 1);
        m_input_name.append(next_file);
    }
    // pienum input file name
    m_pienum_file_name = m_input_name;
    m_pienum_file_name += "_pienum.cnf";
}
