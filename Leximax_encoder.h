#ifndef LEXIMAX_ENCODER
#define LEXIMAX_ENCODER
#include <assert.h>
#include <string>
#include <iostream> // std::cout, std::cin
#include <vector> // std::vector
#include <utility> // std::pair
#include <forward_list> // std::forward_list
#include <unordered_set>
#include "basic_clause.hh"
#include "basic_clset.hh"
#include "basic_types.h"
#include "clause_utils.hh"
#include "cl_registry.hh"
#include "cl_globals.hh"
#include "cl_types.hh"
#include "Options.hh"

typedef std::vector<std::pair<LINT, LINT>*> SNET;

class Leximax_encoder {

private:

    size_t m_id_count;
    BasicClauseSet m_constraints;
    BasicClauseSet m_soft_clauses;
    std::vector<std::vector<LINT>*> m_objectives;
    int m_num_objectives;
    std::vector<std::vector<LINT>*> m_sorted_vecs;
    std::vector<std::vector<LINT>*> m_sorted_relax_vecs;
    std::forward_list<LINT> m_relax_vars;
    std::string m_solver_command;
    std::vector<std::string> m_input_files;
    std::string m_input_name;
    bool m_leave_temporary_files;
    bool m_sat;
    bool m_pbo;
    bool m_debug;
    std::string m_multiplication_string;
    std::vector<LINT> m_optimum;
    std::vector<LINT> m_solution;
    size_t m_sorting_net_size; // number of comparators
    // verification:
    std::string m_pienum_file_name;
    
public:
    
    Leximax_encoder(Options &options);
    
    ~Leximax_encoder();
    
    int read();
    
    void solve();
    
    std::vector<LINT>& get_solution() { return m_solution; }
    
    std::vector<LINT>& get_optimum() { return m_optimum; }
    
    void set_solver_command(std::string &command) { m_solver_command = command; }
    
    // print to standard output
    void print_solution();
    
    // for debuging
    void verify();
    
private:
    
    // constructors.cpp
    
    void update_id_count(std::vector<LINT> &clause);
    
    // reading.cpp
    
    void encode_fresh(BasicClause *cl, LINT fresh_var);
    
    // sorting_net.cpp
    
    void encode_max(LINT var_out_max, LINT var_in1, LINT var_in2);
    
    void encode_min(LINT var_out_min, LINT var_in1, LINT var_in2);
    
    void insert_comparator(LINT el1, LINT el2, std::vector<LINT> *objective, SNET &sorting_network);
    
    void odd_even_merge(std::pair<std::pair<LINT,LINT>,LINT> seq1, std::pair<std::pair<LINT,LINT>,LINT> seq2, std::vector<LINT> *objective, SNET &sorting_network);
    
    void encode_network(std::pair<LINT,LINT> elems_to_sort, std::vector<LINT> *objective, SNET &sorting_network);
    
    void delete_snet(SNET &sorting_network);
    
    // encoding.cpp
    
    size_t get_obj_value(std::vector<LINT> &model);
    
    void encode_sorted();
    
    size_t largest_obj();
    
    void generate_soft_clauses(int i);
    
    void all_subsets(std::forward_list<LINT> set, int i, std::vector<LINT> &clause_vec);
    
    void at_most(std::forward_list<LINT> &set, int i);
    
    void encode_relaxation(int i);
    
    void componentwise_OR(int i);
    
    // solver_call.cpp
    
    int solve_maxsat(int i);
    
    void write_atmost_pb(int i, ostream &output);
    
    void write_pbconstraint(BasicClause *cl, ostream& output);
    
    void write_sum_equals_pb(int i, ostream &output);
    
    int solve_pbo(int i);
    
    int call_solver(std::string &file_name);
    
    // printing.cpp
    
    void print_cnf(ostream &out);
    
    // verify.cpp
    
    void brute_force_optimum(std::vector<LINT> &pienum_opt, std::forward_list<std::vector<LINT>> &sorted_obj_vectors);
    
    void collect_sorted_obj_vecs(std::string &output_filename, std::forward_list<std::vector<LINT>> &sorted_obj_vectors);

};
    
#endif
