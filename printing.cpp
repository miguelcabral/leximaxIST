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

void Leximax_encoder::print_solution(IntVector &tmp_model)
{
    if (!m_sat)
        std::cout << "s UNSATISFIABLE\n";
    else {
        
    }
}
