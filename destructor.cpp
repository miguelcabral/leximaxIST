#include "Leximax_encoder.h"

// remove temporary files and free memory
Leximax_encoder::~Leximax_encoder()
{
    if (!m_leave_temporary_files) {
        remove(m_pienum_file_name.c_str());
    }
    // free m_objectives
    for (std::vector<LINT> *objective : m_objectives)
        delete objective;
    // free m_sorted_vecs
    for (std::vector<LINT> *sorted_vec : m_sorted_vecs)
        delete sorted_vec;
    // free m_sorted_relax_vecs
    for (std::vector<LINT> *sorted_relax_vec : m_sorted_relax_vecs)
        delete sorted_relax_vec;
}
