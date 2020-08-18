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
    std::forward_list<LINT> m_relax_vars;
    std::string m_solver;
    std::string m_input_file_name;
    bool m_pbo;
    bool m_debug;
    std::string m_multiplication_string;
    
public:
    
    Leximax_encoder(int num_objectives) :
        m_id_count(0),
        m_constraints(),
        m_soft_clauses(),
        m_objectives(num_objectives, nullptr),
        m_num_objectives(num_objectives),
        m_sorted_vecs(num_objectives, nullptr),
        m_sorted_relax_vecs(num_objectives, nullptr),
        m_relax_vars(),
        m_solver("openwbo"),
        m_input_file_name("tbd"),
        m_pbo(false),
        m_debug(true),
        m_multiplication_string("*")
    {
    // just initialization    
    }
    
    int read(char *argv[]);
    
    void encode_sorted();
    
    int solve();
    
    void print_cnf();
    
private:
    
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
    
    void write_atmost_pb(int i);
    
    int solve_pbo(int i);

};
    
#endif
