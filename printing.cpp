#include "Leximax_encoder.h"

void print_clause(ostream &out, BasicClause *cl)
{
    for (auto l : *cl)
        out << l << " ";
    out << "0\n";
}

void Leximax_encoder::print_cnf(ostream &out)
{
    out << "c =========================================\n";
    out << "p cnf " << m_id_count << " " << m_constraints.size() << '\n';
    for(BasicClause *cl : m_constraints)
        print_clause(out, cl);
    out << "c =========================================\n";
}

void Leximax_encoder::print_solution(IntVector &model)
{
    if (!m_sat)
        std::cout << "s UNSATISFIABLE\n";
    else {
        std::cout << "s SATISFIABLE\n";
        for (int j = 0; j < m_num_objectives; ++j) {
            std::cout << "o " << m_optimum[j] << '\n';
        }
        std::cout << "v ";
        for (size_t j = 1; j < model.size(); ++j) {
            std::cout << model[j] << " ";
        }
        std::cout << '\n';
    }
}
