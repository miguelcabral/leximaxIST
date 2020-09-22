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
    m_solution,
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

void Leximax_encoder::update_id_count(std::vector<LINT> &clause)
{
    for (LINT lit : clause) {
        LINT var( lit < 0 ? -lit : lit );
        if (var > m_id_count)
            m_id_count = var;
    }
}

Leximax_encoder::Leximax_encoder(std::vector<std::vector<LINT>> &constraints, std::vector<std::vector<std::vector<LINT>>> &objective_functions) : 
    m_id_count(0),
    m_constraints(),
    m_soft_clauses(),
    m_objectives(objective_functions.size(), nullptr),
    m_num_objectives(objective_functions.size()),
    m_sorted_vecs(objective_functions.size(), nullptr),
    m_sorted_relax_vecs(objective_functions.size(), nullptr),
    m_relax_vars(),
    m_solver_command("rc2.py -vv"),
    m_input_files(),
    m_input_name(),
    m_leave_temporary_files(false),
    m_sat(true),
    m_pbo(false),
    m_debug(false),
    m_multiplication_string("*"),
    m_optimum(objective_functions.size(), 0),
    m_solution(),
    // verification:
    m_pienum_file_name()
{
    for (std::vector<LINT> &hard_clause : constraints) {
        // determine max id and update m_id_count
        update_id_count(hard_clause);
        // store clause
        m_constraints.create_clause(hard_clause);
    }
    // update m_id_count if there are new variables in objective_functions
    for (std::vector<std::vector<LINT>> &obj : objective_functions) {
        for (std::vector<LINT> &soft_clause : obj)
            update_id_count(soft_clause);
    }
    // store objective functions - convert clause satisfaction maximisation to minimisation of sum of variables
    int i(0);
    for (std::vector<std::vector<LINT>> &obj : objective_functions) {
        m_objectives[i] = new std::vector<LINT>(obj.size(), 0);
        std::vector<LINT> lits;
        int j(0);
        for (std::vector<LINT> &soft_clause : obj) {
            LINT fresh_var(m_id_count + 1);
            m_id_count++;
            lits = soft_clause;
            (m_objectives.at(i))->at(j) = fresh_var;
            lits.push_back(fresh_var);
            m_constraints.create_clause(lits);
            lits.clear();
            j++;
        }
        ++i;
    }
}
