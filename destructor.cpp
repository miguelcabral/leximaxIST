#include "Leximax_encoder.h"

// remove temporary files and free memory
Leximax_encoder::~Leximax_encoder()
{
    if (!m_leave_temporary_files) {
        remove(m_pienum_file_name.c_str());
    }
    
}
