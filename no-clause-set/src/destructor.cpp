#include <leximaxIST_Encoder.h>
#include <vector>

namespace leximaxIST {

    // free clauses in m_soft_clauses and clear vector
    void Encoder::clear_soft_clauses()
    {
        for (Clause *cl : m_soft_clauses)
            delete cl;
        m_soft_clauses.clear();
    }

    void Encoder::clear_sorted_relax()
    {
        // in first iteration (of the algorithm) nullptr is ignored by delete
        for (size_t i (0); i < m_sorted_relax_vecs.size(); ++i) {
            delete m_sorted_relax_vecs[i];
            m_sorted_relax_vecs[i] = nullptr;
        }
    }

    // free clauses in m_constraints and clear vector
    void Encoder::clear_hard_clauses()
    {
        for (Clause *cl : m_constraints)
            delete cl;
        m_constraints.clear();
    }

    // remove temporary files and free memory
    Encoder::~Encoder()
    {
        // free m_objectives
        for (std::vector<long long> *objective : m_objectives)
            delete objective;
        // free m_sorted_vecs
        for (std::vector<long long> *sorted_vec : m_sorted_vecs)
            delete sorted_vec;
        // free m_sorted_relax_vecs
        clear_sorted_relax();
        clear_hard_clauses();
        clear_soft_clauses();
    }

}/* namespace leximaxIST */
