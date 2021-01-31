#ifndef LEXIMAXIST_ENCODER
#define LEXIMAXIST_ENCODER
#include <leximaxIST_types.h>
#include <string>
#include <vector> // std::vector
#include <utility> // std::pair
#include <forward_list> // std::forward_list
#include <sys/types.h>


namespace leximaxIST
{

    class Encoder {

    private:

        bool m_debug;
        size_t m_id_count;
        std::vector<Clause*> m_constraints;
        std::vector<Clause*> m_soft_clauses;
        std::vector<std::vector<long long>*> m_objectives;
        int m_num_objectives;
        std::vector<std::vector<long long>*> m_sorted_vecs;
        std::vector<std::vector<long long>*> m_sorted_relax_vecs;
        std::vector<std::vector<std::vector<long long>*>>  m_sorted_relax_collection;
        std::vector<std::forward_list<long long>> m_all_relax_vars; // relax_vars of each iteration
        std::string m_solver_command;
        std::string m_formalism;
        std::string m_lp_solver;
        std::string m_valid_lp_solvers[6];
        std::string m_file_name;
        std::string m_err_file;
        bool m_solver_output; // is there an output file to read?
        pid_t m_child_pid;
        double m_timeout; // timeout for signal handling in milliseconds
        bool m_leave_temporary_files;
        bool m_sat;
        std::string m_multiplication_string;
        std::vector<long long> m_solution;
        size_t m_sorting_net_size; // size of largest sorting network
        
    public:    

        /* a clause is a disjunction of literals;
        * a literal is a variable or a negation of a variable;
        * each variable is represented as a positive integer (long long);
        * a clause is represented as a vector of literals (integers)
        * constraints is a collection of clauses;
        * each entry of constraints contains a clause, stored as a vector;
        * each entry of the clause vector is a literal of that clause.
        * objective_functions is a collection of objective functions;
        * each objective function is a vector of soft clauses;*/
        // each objective function is the sum of its falsified soft clauses.
        Encoder(const std::vector<std::vector<long long>> &constraints, const std::vector<std::vector<std::vector<long long>>> &objective_functions); 
        // TODO: add checks if constraints and/or obj functions is empty
        ~Encoder();
        
        // returns 0 if all want well, -1 otherwise
        int solve();
        
        bool get_sat() const;
        
        size_t get_sorting_net_size() const;
        
        /* if the problem is satisfiable, then m_solution is a satisfying assignment;
        * each entry i of m_solution is +i if variable i is true and -i otherwise;
        * if the problem is not satisfiable, m_solution is empty.*/
        const std::vector<long long>& get_solution() const;
        
        // empty if unsat
        std::vector<long long> get_objective_vector() const;
        
        void set_solver_command(const std::string &command);
        
        int set_formalism(const std::string &format);
        
        void set_err_file(const std::string &name);
        
        int set_lp_solver(const std::string &lp_solver);
        
        void set_leave_temporary_files(bool val); // TODO: change temporary to tmp
        
        void set_multiplication_string(const std::string &str);
        
        int terminate(int signum);
        
    private:
        
        // getters.cpp
        
        std::vector<long long> get_objective_vector(const std::vector<long long> &assignment) const;
        
        // constructors.cpp
        
        void add_soft_clause(const std::vector<long long> &lits);
        
        void add_hard_clause(const std::vector<long long> &lits);
        
        void update_id_count(const std::vector<long long> &clause);
        
        // destructor.cpp
        
        void clear_soft_clauses();
        
        void clear_sorted_relax();
        
        void clear_hard_clauses();
        
        // sorting_net.cpp
        
        void encode_max(long long var_out_max, long long var_in1, long long var_in2);
        
        void encode_min(long long var_out_min, long long var_in1, long long var_in2);
        
        void insert_comparator(long long el1, long long el2, std::vector<long long> *objective, SNET &sorting_network);
        
        void odd_even_merge(std::pair<std::pair<long long,long long>,long long> seq1, std::pair<std::pair<long long,long long>,long long> seq2, std::vector<long long> *objective, SNET &sorting_network);
        
        void encode_network(std::pair<long long,long long> elems_to_sort, std::vector<long long> *objective, SNET &sorting_network);
        
        void delete_snet(SNET &sorting_network);
        
        // encoding.cpp
        
        void encode_sorted();
        
        size_t largest_obj();
        
        void generate_soft_clauses(int i);
        
        void all_subsets(std::forward_list<long long> set, int i, std::vector<long long> &clause_vec);
        
        void at_most(std::forward_list<long long> &set, int i);
        
        void encode_relaxation(int i);
        
        void componentwise_OR(int i);
        
        // solver_call.cpp
        
        void reset_file_name();
        
        void remove_all_tmp_files() const;
        
        void remove_tmp_file(const std::string &filename) const;

        int split_solver_command(const std::string &command, std::vector<std::string> &command_split);
        
        int call_solver();
        
        int read_solver_output(std::vector<long long> &model);
        
        int external_solve(int i);
        
        int write_solver_input(int i);
        
        int write_lp_file(int i);
        
        int write_opb_file(int i);
        
        int write_wcnf_file(int i);
        
        int read_cplex_output(std::vector<long long> &model);
        
        int read_gurobi_output(std::vector<long long> &model);
        
        int read_glpk_output(std::vector<long long> &model);
        
        int read_lpsolve_output(std::vector<long long> &model);
        
        int read_scip_output(std::vector<long long> &model);
        
        int read_cbc_output(std::vector<long long> &model);
        
        // printing.cpp
        
        void print_error_msg(const std::string &msg) const;
        
        void print_waitpid_error(const std::string &errno_str) const;
        
        void print_clause(std::ostream &output, const Clause *cl) const;
        
        void print_clauses(std::ostream &output, const std::vector<Clause*> &clauses, size_t weight) const;
        
        void print_atmost_lp(int i, std::ostream &output) const;
        
        void print_lp_constraint(const Clause *cl, std::ostream &output) const;
        
        void print_sum_equals_lp(int i, std::ostream &output) const;
        
        void print_atmost_pb(int i, std::ostream &output) const;
        
        void print_pb_constraint(const Clause *cl, std::ostream &output) const;
        
        void print_sum_equals_pb(int i, std::ostream &output) const;

    };

}

#endif
