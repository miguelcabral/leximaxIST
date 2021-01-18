#include <Leximax_encoder.h>
#include <sstream>
#include <unistd.h>

void Leximax_encoder::add_hard_clause(const std::vector<LINT> &lits)
{
    Clause *cl (new Clause(lits));
    m_constraints.push_back(cl);
}

void Leximax_encoder::add_soft_clause(const std::vector<LINT> &lits)
{
    Clause *cl (new Clause(lits));
    m_soft_clauses.push_back(cl);
}

void Leximax_encoder::update_id_count(const std::vector<LINT> &clause)
{
    for (LINT lit : clause) {
        LINT var( lit < 0 ? -lit : lit );
        if (var > m_id_count)
            m_id_count = var;
    }
}

Leximax_encoder::Leximax_encoder(const std::vector<std::vector<LINT>> &constraints, const std::vector<std::vector<std::vector<LINT>>> &objective_functions) : 
    m_debug(false),
    m_id_count(0),
    m_constraints(),
    m_soft_clauses(),
    m_objectives(objective_functions.size(), nullptr),
    m_num_objectives(objective_functions.size()),
    m_sorted_vecs(objective_functions.size(), nullptr),
    m_sorted_relax_vecs(objective_functions.size(), nullptr),
    m_all_relax_vars(),
    m_solver_command("rc2.py -vv"),
    m_solver_format("wcnf"),
    m_lp_solver("cplex"),
    m_valid_lp_solvers {"cplex", "gurobi", "glpk", "scip", "cbc", "lpsolve"},
    m_file_name(),
    m_err_file(),
    m_child_pid(0),
    m_leave_temporary_files(false),
    m_sat(false),
    m_multiplication_string(" "),
    m_solution(),
    m_sorting_net_size(0)
{  // TODO: what to do when constraints are empty or when objective functions are empty...
    for (const std::vector<LINT> &hard_clause : constraints) {
        // determine max id and update m_id_count
        update_id_count(hard_clause);
        // store clause
        add_hard_clause(hard_clause);
    }
    // update m_id_count if there are new variables in objective_functions
    for (const std::vector<std::vector<LINT>> &obj : objective_functions) {
        for (const std::vector<LINT> &soft_clause : obj)
            update_id_count(soft_clause);
    }
    // store objective functions - convert clause satisfaction maximisation to minimisation of sum of variables
    int i(0);
    for (const std::vector<std::vector<LINT>> &obj : objective_functions) {
        m_objectives[i] = new std::vector<LINT>(obj.size(), 0);
        std::vector<LINT> lits;
        int j(0);
        for (const std::vector<LINT> &soft_clause : obj) {
            LINT fresh_var(m_id_count + 1);
            m_id_count++;
            lits = soft_clause;
            (m_objectives.at(i))->at(j) = fresh_var;
            lits.push_back(fresh_var);
            add_hard_clause(lits);
            lits.clear();
            j++;
        }
        ++i;
    }
    // name for temporary files
    std::stringstream strstr;
    strstr << getpid();
    m_file_name = strstr.str();
}
