#ifndef LEXIMAXIST_SOLVER
#define LEXIMAXIST_SOLVER
#include <leximaxIST_types.h>
#include <IpasirWrap.h>
#include <leximaxIST_parsing_utils.h>
#include <leximaxIST_ILPConstraint.h>
#include <string> // std::string
#include <vector> // std::vector
#include <unordered_map> // std::unordered_map
#include <utility> // std::pair
#include <list> // std::list
#include <sys/types.h> // pid_t


namespace leximaxIST
{

    class Solver {
        /* TODO: check if pointers can be removed without affecting performance
         * The problem is reallocation of vector while growing
         * It seems that it is not mandatory that the implementation of the STL
         * uses the move constructor while copying the elements of the vector
         * So it is not guaranteed for every compiler that the performance will
         * not be affected
         * TODO: signal handling: (1) external solver (2) internal ipasir solver
         */

    private:

        int m_verbosity; // 0: nothing, 1: solving phases, time + obj vector, 2: everything including encoding
        int m_id_count;
        int m_input_nb_vars; // number of vars of input problem - useful to return assignment of only these variables
        std::vector<Clause> m_input_hard; // also contains the equivalence between soft clauses and obj variables
        std::vector<Clause> m_encoding; // encoding clauses
        std::vector<int> m_soft_clauses; // unit clauses
        std::vector<std::vector<int>> m_objectives;
        int m_num_objectives;
        std::vector<std::vector<int>> m_sorted_vecs;
        std::vector<std::vector<std::vector<int>>>  m_sorted_relax_collection;
        std::vector<std::list<int>> m_all_relax_vars; // relax_vars of each iteration
        std::string m_ext_solver_cmd; // for external call to optimisation solver
        std::string m_formalism;
        std::string m_ilp_solver; // ilp solver for the ilp-based algorithm
        std::string m_file_name;
        std::string m_opt_mode; // optimisation algorithm : lin-su, lin-us, bin, core-merge, ...
        std::string m_approx; // approximation algorithm : mss, gia
        bool m_disjoint_cores; // use disjoint cores strategy in the core-guided algorithm
        pid_t m_child_pid;
        double m_timeout; // timeout for signal handling in milliseconds
        bool m_leave_tmp_files;
        bool m_simplify_last; // if true the algorithm does not use the sorting networks in the last iteration
        char m_status; // 's' for SATISFIABLE, 'u' for UNSATISFIABLE, '?' for UNKNOWN, 'o' for OPTIMUM FOUND
        double m_approx_tout; // timeout for approximation
        bool m_gia_incr; // whether to use the same SAT solver in every Pareto-optimal solution search
        bool m_gia_pareto; // whether to continue to minimise to Pareto optimality, even though the maximum can not be improved
        int m_mss_add_cls; // how to use the models returned by the SAT solver in the construction of the MSS
        bool m_mss_incr; // (truly incremental enumeration) - whether to use the same SAT solver in every MSS search
        int m_mss_nb_limit; // stop the enumeration when this number of MSSes is reached
        int m_mss_tolerance; // tolerance for choosing the next clause from a maximum objective
        bool m_maxsat_presolve; // to get lower bound (and upper bound) of optimum
        std::string m_maxsat_psol_cmd;
        // the next one is usefull if computation is stopped and you get an intermediate solution
        // you want to know which values of the objective vector are in theory guaranteed to be optimal
        //int m_num_opts; // number of optimal values found: 0 = none; 1 = first maximum is optimal; 2 = first and second; ...
        std::string m_multiplication_string;
        std::vector<int> m_solution;
        std::vector<std::pair<int, int>> m_snet_info; // first = nb wires and second = nb comparators 
        //std::vector<double> m_times; // time of each step of solving (only external solver times)
        IpasirWrap *m_sat_solver;
        std::vector<std::string> m_tmp_files; // container with the names of all temporary files used by the solver
        
    public:    

        Solver(); 

        ~Solver();
        
        int nVars() const;
        
        void optimise();
        
        void approximate();
        
        char get_status() const;
        
        void print_solution() const; // prints solution in a similar format to the MaxSAT output format
        
        //int get_num_opts() const;
        
        //const std::vector<double>& get_times() const;
        
        /* if the problem is satisfiable, then m_solution is a satisfying assignment;
        * each entry i of m_solution is +i if variable i is true and -i otherwise;
        * if the problem is not satisfiable, m_solution is empty.*/
        std::vector<int> get_solution() const;
        
        // empty if unsat; not a const reference because it is not a member variable
        std::vector<int> get_objective_vector() const;
        
        void add_hard_clause(const Clause &cl);
        
        void add_soft_clauses(const std::vector<Clause> &soft_clauses);
        
        void set_simplify_last(bool val);
        
        void set_timeout(double val); // for terminate function
        
        void set_opt_mode(const std::string &mode);
        
        void set_ext_solver_cmd(const std::string &command);
        
        void set_formalism(const std::string &format);
        
        void set_verbosity(int v); // if value is invalid the program is terminated
        
        void set_ilp_solver(const std::string &ilp_solver);
        
        void set_leave_tmp_files(bool val);
        
        void set_multiplication_string(const std::string &str);
        
        void set_maxsat_presolve(bool v);
        
        void set_maxsat_psol_cmd(const std::string &cmd);
        
        void set_approx_tout(double t);
        
        void set_gia_incr(bool v);
        
        void set_gia_pareto(bool v);
        
        void set_approx(const std::string &algorithm);
        
        /* 0: add all satisfied clauses to the MSS in construction
         * 1: add the same amount of satisfied clauses per objective
         * 2: do not add satisfied clauses to the MSS in construction
         */
        void set_mss_add_cls(int v);
        
        void set_mss_incr(bool v);
        
        void set_mss_nb_limit(int n);
        
        void set_mss_tol(int t);
        
        void set_disjoint_cores(bool v);
                
        int terminate(); // kill external solver and read approximate solution
        
        void clear(); // frees memory, and sets internal parameters to their initial value
        
    private:
        
        // setters.cpp
        
        int fresh();
        
        void reset_file_name();
        
        void reset_id_count();
        
        void update_id_count(const Clause &clause);
        
        // update m_solution if the model is leximax-better than the current solution
        // e.g. in the case the external solver is killed and outputs a suboptimal solution
        // or if I get an MSS and it may be worse than the solution that I already have
        // returns the objective vector of the leximax-best assignment
        // the model is moved to m_solution and then cleared
        // if the model is better and if verbode, the obj vec and the time is printed
        // Returns the best objective vector of the two
        std::vector<int> set_solution(std::vector<int> &model);
        
        // getters.cpp
        
        std::vector<int> get_objective_vector(const std::vector<int> &assignment) const;
        
        // constructors.cpp
        
        void add_clause(const Clause &cl, std::vector<Clause> &set_of_clauses);
        
        void add_clause_enc(const Clause &cl);
        
        void add_clause(int l);
        
        void add_clause(int l1, int l2);
        
        void add_clause(int l1, int l2, int l3);
        
        // sorting_net.cpp
        
        void encode_max(int var_out_max, int var_in1, int var_in2);
        
        void encode_min(int var_out_min, int var_in1, int var_in2);
        
        void insert_comparator(int el1, int el2, const std::vector<int> *objective, SNET &sorting_network);
        
        int odd_even_merge(std::pair<std::pair<int,int>,int> seq1, std::pair<std::pair<int,int>,int> seq2, const std::vector<int> *objective, SNET &sorting_network);
        
        int encode_network(const std::pair<int,int> elems_to_sort, const std::vector<int> *objective, SNET &sorting_network);
        
        void merge_core_guided(const std::vector<std::vector<int>> &inputs_to_sort, const std::vector<std::vector<int>> &unit_core_vars);
        
        //void delete_snet(SNET &sorting_network);
        
        // encoding.cpp
        
        void encode_sorted(const std::vector<int> &inputs_to_sort, int i);
        
        size_t largest_obj() const;
        
        void order_encoding(const std::vector<int>& vars);
        
        void generate_soft_clauses(int i);
        
        void all_subsets(std::list<int> set, int i, Clause &clause);
        
        void at_most(const std::list<int> &set, int i);
        
        void encode_relaxation(int i);
        
        void componentwise_OR(int i, const std::vector<int> &max_vars);
        
        int encode_bounds(int i, int sum);
        
        int encode_lower_bound(int i, int sum);
        
        void encode_lb_soft(int lb);
        
        void encode_lb_sorted(int lb);
        
        int encode_upper_bound(int i);
        
        void encode_ub_sorted(int first_max);
        
        void encode_ub_soft(int max_i);
        
        void fix_soft_vars(int i);
        
        void fix_all(int i);
        
        void fix_only_some();
        
        void optimise_non_core(int sum);
        
        void optimise_core_guided();
        
        void generate_max_vars(int i, std::vector<std::vector<int>> &max_vars_vec);
        
        void gen_assumps(const std::vector<int> &lower_bounds, const std::vector<std::vector<int>> &max_vars_vec,
                     const std::vector<std::vector<int>> &inputs_not_sorted, std::vector<int> &assumps) const;
                     
        bool disjoint_cores(std::vector<std::vector<int>> &inputs_not_sorted,
                                 std::vector<std::vector<int>> &unit_core_vars,
                                 std::vector<int> &lower_bounds, std::unordered_map<int, int> &lb_map);
        
        void increase_lb(std::vector<int> &lower_bounds, const std::vector<int> &core,
                              const std::vector<std::vector<int>> &max_vars_vec) const;
                              
        void fix_max(int j, const std::vector<std::vector<int>> &max_vars_vec, const std::vector<int> &lower_bounds);
                                    
        bool find_vars_in_core(std::vector<std::vector<int>> &inputs_not_sorted, const std::vector<int> &core,
                                    std::vector<std::vector<int>> &new_inputs) const;
                                    
        void initialise_lb_map(std::unordered_map<int, int> &lb_map, int nb_objs) const;
        
        void change_lb_map(int min_index, std::vector<int> &lower_bounds, const std::vector<int> &core,
                      const std::vector<std::vector<int>> &max_vars_vec, std::unordered_map<int, int> &lb_map) const;
                      
        void add_unit_core_vars(const std::vector<std::vector<int>> &unit_core_vars, int j);
        
        // alg_opt_ilp.cpp
        
        void optimise_ilp();
        
        int ilp_bound() const;
        
        // solver_call.cpp
        
        void call_ilp_solver(const std::vector<ILPConstraint> &constraints, const std::vector<int> &max_vars, int i);
        
        void write_lp_file(const std::vector<ILPConstraint> &constraints, const std::vector<int> &max_vars, int i) const;
        
        bool call_sat_solver(IpasirWrap *solver, const std::vector<int> &assumps);
        
        void bound_objs(std::vector<int> &unit_clauses, int max, const std::vector<int> &obj_vec) const;
        
        void fix_previous_max(std::vector<int> &unit_clauses, int max_index, const std::vector<int> &obj_vec) const;
        
        void decrease_max(std::vector<int> &unit_clauses, int max_index, const std::vector<int> &obj_vec) const;
        
        void gia();
        
        int pareto_search(int &max_index, IpasirWrap *solver);
        
        void internal_solve(const int i, const int lb);
        
        void update_lb(int &lb);
        
        void search(int i, int lb, int ub);
        
        void mss_add_falsified (IpasirWrap *solver, const std::vector<int> &model, std::vector<std::vector<int>> &mss, std::vector<std::vector<int>> &todo_vec, std::vector<int> &assumps);
        
        int mss_choose_obj (const std::vector<std::vector<int>> &todo_vec, const std::vector<std::vector<int>> &mss, const int best_max) const;
        
        int mss_linear_search(std::vector<std::vector<int>> &mss, IpasirWrap *solver, int &best_max);
        
        void mss_enumerate();
        
        int get_lower_bound(const std::vector<int> &model);
        
        int maxsat_presolve();
        
        int presolve();
        
        void remove_tmp_files();

        void split_command(const std::string &command, std::vector<std::string> &command_split);
        
        void call_ext_solver(const std::string &cmd);
        
        void read_solver_output(std::vector<int> &model, const std::string &filename);
        
        void external_solve(int i);
        
        void write_solver_input(int i);
        
        void write_cnf_file(int i);
        
        void write_lp_file(int i);
        
        void write_opb_file(int i);
        
        void write_wcnf_file(int i);
        
        void read_sat_output(std::vector<int> &model, bool &sat, StreamBuffer &r);
        
        void read_cplex_output(std::vector<int> &model, bool &sat, StreamBuffer &r);
        
        void read_gurobi_output(std::vector<int> &model, bool &sat, StreamBuffer &r);
        /*
        int read_glpk_output(std::vector<int> &model);
        
        int read_lpsolve_output(std::vector<int> &model);
        
        int read_scip_output(std::vector<int> &model);
        
        int read_cbc_output(std::vector<int> &model);
        */
        
        // printing.cpp
        
        void print_hard_clauses(std::ostream &output) const;
        
        void print_soft_clauses(std::ostream &output) const;
        
        void print_soft_clauses() const;
        
        void print_sorted_vec (int i) const;
        
        void print_sorted_true() const; // for debugging
        
        void print_objs_sorted(const std::vector<std::vector<int>> &inputs_not_sorted) const;
        
        void print_obj_func(int i) const;
        
        void print_snet_info() const;
        
        void print_mss_debug(const std::vector<std::vector<int>> &todo_vec, const std::vector<std::vector<int>> &mss) const;
        
        void print_mss_info(int nb_calls, const std::vector<std::vector<int>> &todo_vec,  const std::vector<std::vector<int>> &mss) const;
        
        void print_mss_enum_info() const;
        
        void print_obj_vector(const std::vector<int> &obj_vec) const;
        
        void print_obj_vector() const; // this one computes the obj_vector
        
        void print_waitpid_error(const std::string &errno_str) const;
        
        void print_clause(std::ostream &output, const Clause &cl, const std::string &leadingStr = "") const;
        
//         void print_wcnf_clauses(std::ostream &output, const std::vector<Clause*> &clauses, size_t weight) const;
        
//         void print_atmost_lp(int i, std::ostream &output) const;
        
        void print_lp_constraint(const Clause &cl, std::ostream &output) const;
        
//         void print_sum_equals_lp(int i, std::ostream &output) const;
        
//         void print_atmost_pb(int i, std::ostream &output) const;
        
        void print_pb_constraint(const Clause &cl, std::ostream &output) const;
        
//         void print_sum_equals_pb(int i, std::ostream &output) const;
        
        /* Friend classes:
         * openwbo PB encodings so that they can call fresh() to produce new variables
         * I do not want fresh() to be public
         */
        
        friend class SWC;
        friend class Totalizer;
        friend class MTotalizer;
        friend class Adder;
        friend class Ladder;
        friend class KPA;
        friend class CNetworks;
        friend class GTE;
        
    }; /* Solver class definition */

} /* namespace leximaxIST */


#endif
