#include <Leximax_encoder.h>

// free clauses in m_soft_clauses and clear vector
void Leximax_encoder::clear_soft_clauses()
{
    for (Clause *cl : m_soft_clauses)
        delete cl;
    m_soft_clauses.clear();
}

void Leximax_encoder::clear_sorted_relax()
{
    // in first iteration (of the algorithm) nullptr is ignored by delete
    for (size_t i (0); i < m_sorted_relax_vecs.size(); ++i) {
        delete m_sorted_relax_vecs[i];
        m_sorted_relax_vecs[i] = nullptr;
    }
}

// free clauses in m_constraints and clear vector
void Leximax_encoder::clear_hard_clauses()
{
    for (Clause *cl : m_constraints)
        delete cl;
    m_constraints.clear();
}

// remove temporary files and free memory
Leximax_encoder::~Leximax_encoder()
{
    // free m_objectives
    for (std::vector<LINT> *objective : m_objectives)
        delete objective;
    // free m_sorted_vecs
    for (std::vector<LINT> *sorted_vec : m_sorted_vecs)
        delete sorted_vec;
    // free m_sorted_relax_vecs
    clear_sorted_relax();
    clear_hard_clauses();
    clear_soft_clauses();
}
