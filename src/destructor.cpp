#include <leximaxIST_Encoder.h>
#include <leximaxIST_printing.h>
#include <vector>

namespace leximaxIST {
    
    // members that are not a user parameter are cleared and set to initial value
    void Encoder::clear()
    {
        m_objectives.clear();
        m_sorted_vecs.clear();
        // clear relaxation variables
        m_all_relax_vars.clear();
        m_sorted_relax_collection.clear();
        m_input_hard.clear();
        m_encoding.clear();
        m_soft_clauses.clear();
        m_solution.clear();
        m_id_count = 0;
        m_input_nb_vars = 0;
        m_num_objectives = 0;
        m_child_pid = 0;
        m_status = '?';
        m_snet_info.clear();
        //m_times.clear();
        // clear sat solver
        delete m_sat_solver;
        m_sat_solver = nullptr;
    }
    
    // remove temporary files and free memory
    Encoder::~Encoder()
    {
        clear();
    }

}/* namespace leximaxIST */
