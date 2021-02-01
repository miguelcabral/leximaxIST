#include <leximaxIST_Encoder.h>
#include <sstream>
#include <vector>
#include <unistd.h>

namespace leximaxIST {

    void Encoder::add_hard_clause(const Clause &lits)
    {
        Clause *cl (new Clause(lits));
        m_constraints.push_back(cl);
    }

    void Encoder::add_soft_clause(const Clause &lits)
    {
        Clause *cl (new Clause(lits));
        m_soft_clauses.push_back(cl);
    }

    void Encoder::update_id_count(const std::vector<long long> &clause)
    {
        for (long long lit : clause) {
            long long var( lit < 0 ? -lit : lit );
            if (var > m_id_count)
                m_id_count = var;
        }
    }

    Encoder::Encoder(const std::vector<std::vector<long long>> &constraints, const std::vector<std::vector<std::vector<long long>>> &objective_functions) : 
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
        m_formalism("wcnf"),
        m_lp_solver("gurobi"),
        m_valid_lp_solvers {"cplex", "gurobi", "glpk", "scip", "cbc", "lpsolve"},
        m_file_name(),
        m_err_file(),
        m_solver_output(false),
        m_child_pid(0),
        m_timeout(3000.0), // 3 seconds
        m_leave_temporary_files(false),
        m_sat(false),
        m_algorithm(0), // default: original algorithm
        m_multiplication_string(" "),
        m_solution(),
        m_sorting_net_size(0)
    {  // TODO: what to do when constraints are empty or when objective functions are empty...
        // if a constraint is empty or an objective function is empty then error
        // if constraints or objective functions are empty then error -> nothing to optimise or no constraints -> trivial problem
        // when problem occurs clear constraints and objective functions so that when solving return right away with error
        // Maybe change things: constructor does not read constraints nor objective_functions
        // Create method: read_problem for this phase so that I can have return value indicating if some error occured or not
        for (const std::vector<long long> &hard_clause : constraints) {
            // determine max id and update m_id_count
            update_id_count(hard_clause);
            // store clause
            add_hard_clause(hard_clause);
        }
        // update m_id_count if there are new variables in objective_functions
        for (const std::vector<std::vector<long long>> &obj : objective_functions) {
            for (const std::vector<long long> &soft_clause : obj)
                update_id_count(soft_clause);
        }
        // store objective functions - convert clause satisfaction maximisation to minimisation of sum of variables
        int i(0);
        for (const std::vector<std::vector<long long>> &obj : objective_functions) {
            m_objectives[i] = new std::vector<long long>(obj.size(), 0);
            std::vector<long long> lits;
            int j(0);
            for (const std::vector<long long> &soft_clause : obj) {
                long long fresh_var(m_id_count + 1);
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

}/* namespace leximaxIST */
