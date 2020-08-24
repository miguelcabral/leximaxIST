#include "Leximax_encoder.h"

void print_clause(BasicClause *cl)
{
    for (auto l : *cl)
        std::cout << l << " ";
    std::cout << "0" << std::endl;
}

void Leximax_encoder::print_cnf()
{
    std::cout << "c =========================================\n";
    std::cout << "p cnf " << m_id_count << " " << m_constraints.size() << std::endl;
    for(BasicClause *cl : m_constraints)
        print_clause(cl);
    std::cout << "c =========================================\n";
}

void Leximax_encoder::print_optimum(IntVector &model)
{
    if (m_sat) {
        size_t optimum = 0;
        for (BasicClause *cl : m_soft_clauses) {
            LINT var = -(*(cl->begin()));
            if (model[var] > 0)
                optimum++;
        }
        std::cout << "o " << optimum << '\n';
    }
}

void Leximax_encoder::print_solution(IntVector &model)
{
    if (!m_sat)
        std::cout << "s UNSATISFIABLE\n";
    else {
        std::cout << "s SATISFIABLE\n";
        std::cout << "v ";
        for (size_t j = 0; j < model.size(); ++j) {
            std::cout << model[j] << " ";
        }
        std::cout << '\n';
    }
}
