#ifndef LEXIMAX_ENCODER
#define LEXIMAX_ENCODER
#include <string>
#include <iostream> // std::cout, std::cin
#include <vector> // std::vector
#include <utility> // std::pair
#include <forward_list> // std::forward_list
#include "old_packup/basic_clause.hh"
#include "old_packup/basic_clset.hh"
#include "old_packup/basic_types.h"
#include "old_packup/clause_utils.hh"
#include "old_packup/cl_registry.hh"
#include "old_packup/cl_globals.hh"
#include "old_packup/cl_types.hh"

typedef std::vector<std::pair<LINT, LINT>*> SNET;

class Leximax_encoder {

private:

    LINT m_id_count;
    BasicClauseSet m_constraints;
    BasicClauseSet m_soft_clauses;
    std::vector<std::vector<LINT>*> m_objectives;
    int m_num_objectives;
    std::vector<std::vector<LINT>*> m_sorted_vecs;
    std::vector<std::vector<LINT>*> m_sorted_relax_vecs;
    std::string m_solver;
    bool m_pbo;
    bool m_debug;
    
public:
    
    Leximax_encoder(int num_objectives);
    
    int read(char *argv[]);
    
    void encode_sorted();
    
    int solve();
    
    void debug_sorted();
    
    void print_cnf();
    
//private:
    
    void print_clause(BasicClause *cl);
    
    void encode_fresh(BasicClause *cl, LINT fresh_var);
    
    void encode_max(LINT var_out_max, LINT var_in1, LINT var_in2);
    
    void encode_min(LINT var_out_min, LINT var_in1, LINT var_in2);
    
    void insert_comparator(LINT el1, LINT el2, std::vector<LINT> *objective, SNET &sorting_network);
    
    void odd_even_merge(std::pair<std::pair<LINT,LINT>,LINT> seq1, std::pair<std::pair<LINT,LINT>,LINT> seq2, std::vector<LINT> *objective, SNET &sorting_network);
    
    void encode_network(std::pair<LINT,LINT> elems_to_sort, std::vector<LINT> *objective, SNET &sorting_network);
    
    size_t largest_obj();
    
    void all_subsets(std::forward_list<LINT> set, int i, std::vector<LINT> &clause_vec);
    
    void at_most(std::forward_list<LINT> &set, int i);
    
    void encode_relaxation(int i);
    
    void componentwise_OR(int i);
    
    int solve_maxsat();
    
    int solve_pbo();

};
    
#endif
