#ifndef LEXIMAX_ENCODER
#define LEXIMAX_ENCODER
#include <string>
#include <iostream> // std::cout, std::cin
#include <vector> // std::vector
#include <utility> // std::pair
#include "old_packup/basic_clause.hh"
#include "old_packup/basic_clset.hh"
#include "old_packup/basic_types.h"
#include "old_packup/clause_utils.hh"
#include "old_packup/cl_registry.hh"
#include "old_packup/cl_globals.hh"
#include "old_packup/cl_types.hh"

typedef std::vector<std::pair<LINT, LINT>*> SNET;

class Leximax_encoder {

    LINT m_id_count;
    BasicClauseSet m_constraints;
    std::vector<std::vector<LINT>*> m_objectives;
    int m_num_objectives;
    std::vector<std::pair<LINT, LINT>> m_sorted_vecs;
    
    Leximax_encoder(int num_objectives);
    
    int read(char *files[]);
    
    void encode_sorted();
    
    int solve();
    

};
    
#endif
