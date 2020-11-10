#ifndef LEXIMAX_ENCODER
#define LEXIMAX_ENCODER
#include <assert.h>
#include <string>
#include <iostream> // std::cout, std::cin
#include <vector> // std::vector
#include <utility> // std::pair
#include <forward_list> // std::forward_list
#include <leximax_types.h>

class Leximax_encoder {

private:

    size_t m_id_count;
    std::vector<Clause*> m_constraints;
    std::vector<Clause*> m_soft_clauses;
    std::vector<std::vector<LINT>*> m_objectives;
    int m_num_objectives;
    std::vector<std::vector<LINT>*> m_sorted_vecs;
    std::vector<std::vector<LINT>*> m_sorted_relax_vecs;
    std::forward_list<LINT> m_relax_vars;
    std::string m_solver_command;
    std::string m_input_name;
    bool m_leave_temporary_files;
    bool m_sat;
    bool m_pbo;
    bool m_debug;
    std::string m_multiplication_string;
    std::vector<LINT> m_optimum;
    std::vector<LINT> m_solution;
    size_t m_sorting_net_size;
    
public:    

    // constraints is a set of hard clauses;
    // objective_functions is a collection of objective functions, each one being a vector of soft clauses;
    // each objective function is the sum of its falsified soft clauses.
    Leximax_encoder(std::vector<std::vector<LINT>> &constraints, std::vector<std::vector<std::vector<LINT>>> &objective_functions); 
    
    ~Leximax_encoder();
    
    void solve();
    
    bool get_sat() { return m_sat; }
    
    size_t get_sorting_net_size() { return m_sorting_net_size; }
    
    std::vector<LINT>& get_solution() { return m_solution; }
    
    std::vector<LINT>& get_optimum() { return m_optimum; }
    
    void set_solver_command(std::string &command) { m_solver_command = command; }
    
    void set_pbo(bool val) { m_pbo = val; }
    
    void set_leave_temporary_files(bool val) { m_leave_temporary_files = val; }
    
    void set_multiplication_string(std::string &str) { m_multiplication_string = str; }
    
private:
    
    // constructors.cpp
    
    void add_soft_clause(const std::vector<LINT> &lits);
    
    void add_hard_clause(const std::vector<LINT> &lits);
    
    void update_id_count(std::vector<LINT> &clause);
    
    // destructor.cpp
    
    void clear_soft_clauses();
    
    void clear_hard_clauses();
    
    // sorting_net.cpp
    
    void encode_max(LINT var_out_max, LINT var_in1, LINT var_in2);
    
    void encode_min(LINT var_out_min, LINT var_in1, LINT var_in2);
    
    void insert_comparator(LINT el1, LINT el2, std::vector<LINT> *objective, SNET &sorting_network);
    
    void odd_even_merge(std::pair<std::pair<LINT,LINT>,LINT> seq1, std::pair<std::pair<LINT,LINT>,LINT> seq2, std::vector<LINT> *objective, SNET &sorting_network);
    
    void encode_network(std::pair<LINT,LINT> elems_to_sort, std::vector<LINT> *objective, SNET &sorting_network);
    
    void delete_snet(SNET &sorting_network);
    
    // encoding.cpp
    
    void encode_sorted();
    
    size_t largest_obj();
    
    void generate_soft_clauses(int i);
    
    size_t get_obj_value(std::vector<LINT> &model);
    
    void all_subsets(std::forward_list<LINT> set, int i, std::vector<LINT> &clause_vec);
    
    void at_most(std::forward_list<LINT> &set, int i);
    
    void encode_relaxation(int i);
    
    void componentwise_OR(int i);
    
    // solver_call.cpp
    
    int solve_maxsat(int i);
    
    void write_atmost_pb(int i, ostream &output);
    
    void write_pbconstraint(Clause *cl, ostream& output);
    
    void write_sum_equals_pb(int i, ostream &output);
    
    int solve_pbo(int i);
    
    int call_solver(std::string &file_name);

};
    
#endif
