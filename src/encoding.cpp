#include <leximaxIST_Encoder.h>
#include <leximaxIST_rusage.h>
#include <leximaxIST_printing.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <utility>
#include <algorithm>
#include <list>
#include <cmath>

namespace leximaxIST {

    bool descending_order (int i, int j);
    
    void Encoder::encode_sorted()
    {   
        for (int i (0); i < m_num_objectives; ++i) {
            const std::vector<int> *objective (m_objectives.at(i));
            const size_t nb_wires = objective->size();
            m_sorted_vecs.at(i)->resize(nb_wires, 0);
            // sorting_network is initialized to a vector of pairs (-1,-1)
            SNET sorting_network(nb_wires, {-1,-1});
            // elems_to_sort is represented by a pair (first element, number of elements).
            std::pair<int,int> elems_to_sort(0, nb_wires);
            if (m_verbosity == 2)
                std::cout << "c -------- Sorting Network Encoding --------\n";
            m_snet_info.at(i).first = encode_network(elems_to_sort, objective, sorting_network);
            m_snet_info.at(i).second = nb_wires;
            if (m_verbosity >= 1)
                print_snet_info(i);
            // sorted_vec variables are the outputs of sorting_network
            if (nb_wires == 1) { // in this case the sorting network is empty
                std::vector<int> *sorted_vec = m_sorted_vecs[i];
                sorted_vec->at(0) = objective->at(0);
            }
            else {
                for (size_t j{0}; j < nb_wires; j++) {
                    int output_j = sorting_network[j].second;
                    std::vector<int> *sorted_vec = m_sorted_vecs[i];
                    sorted_vec->at(j) = output_j;
                }
            }
            if (m_verbosity == 2)
                print_sorted_vec(i);
        }
        // add order encoding to each sorted vector
        for (int i (0); i < m_num_objectives; ++i)
            order_encoding(*(m_sorted_vecs.at(i)));
    }

    void Encoder::all_subsets(std::list<int> set, int i, Clause &clause)
    {
        int size = clause.size();
        // Base of recursion: 
        // when |set| == i, then set is the only subset of size i
        int set_size = std::distance(set.begin(), set.end());
        if (set_size == i) {
            int j = size - i;
            for (int elem : set) {
                clause[j] = -elem;
                j++;
            }
            // add clause to constraints
            if (m_verbosity == 2)
                std::cout << "c Combination: ";
            add_hard_clause(clause);
        }
        // when i == 1, then each element of set is a subset of size 1
        else if (i == 1) {
            for (int elem : set) {
                clause[size-1] = -elem;
                // add clause to constraints
                if (m_verbosity == 2)
                    std::cout << "c Combination: ";
                add_hard_clause(clause);
            }
        }
        else {
        // Step of recursion: the combinations that include the first element of set + those that don't include it
        int first_el = set.front();
        set.pop_front();
        clause[size - i] = -first_el;
        all_subsets(set, i-1, clause); // combinations that include first_el
        all_subsets(set, i, clause); // combinations that don't include first_el
        }
    }

    void Encoder::at_most(const std::list<int> &set, int i)
    {
        // implementation with naive encoding
        // for every combination of i + 1 vars, one of them must be false
        Clause clause(i + 1, -1); // fill constructor i+1 elements with value -1
        all_subsets(set, i + 1, clause);
    }

    // create new relaxation variables and sorted vectors after the relaxation
    // and add the relaxation constraints to the hard clauses
    void Encoder::encode_relaxation(int i)
    {
        std::list<int> relax_vars;
        for (int j = 0; j < m_num_objectives; ++j)
            relax_vars.push_back(fresh());
        if (m_opt_mode == "core-guided")
            m_all_relax_vars.at(i) = relax_vars;
        else
            m_all_relax_vars.push_back(relax_vars);
        int first_relax_var (relax_vars.front());
        if (m_verbosity == 2) {
            std::cout << "c ------------ Relaxation variables of " <<  ordinal(i+1) << " maximum ------------\n";
            for (int v : relax_vars)
                std::cout << "c " << v << '\n';
            if (!m_simplify_last || i != m_num_objectives - 1)
                std::cout << "c ------------ Sorted vecs after relaxation for " << ordinal(i+1) << " maximum ------------\n";
        }
        if (!m_simplify_last || i != m_num_objectives - 1 || m_opt_mode == "core-guided") {
            std::vector<std::vector<int>*> sorted_relax_vecs(m_num_objectives, nullptr);
            for (int j = 0; j < m_num_objectives; ++j) {
                // encode relaxation variable of the j-th objective
                std::vector<int> *sorted_vec = m_sorted_vecs.at(j);
                std::vector<int> *sorted_relax = new std::vector<int>(sorted_vec->size(), 0);
                sorted_relax_vecs.at(j) = sorted_relax;
                if (m_verbosity == 2)
                    std::cout << "c --------------- sorted_relax_vecs[" << j << "] ---------------\n";
                for (size_t k = 0; k < sorted_relax->size(); ++k) {
                    // create sorted_relax variables
                    sorted_relax->at(k) = fresh();
                    // encoding:
                    // relax_j implies not sorted_relax_j_k
                    if (m_verbosity == 2) {
                        std::cout << "c sorted_relax[" << k << "]: " << sorted_relax->at(k) << "\n";
                        std::cout << "c -------------- relax_var implies not sorted_relax["<< k << "] ------\n";
                    }
                    add_hard_clause(-(first_relax_var + j), -sorted_relax->at(k));
                    if (m_verbosity == 2)
                        std::cout << "c ------- not relax_var implies sorted_relax["<< k << "] equals sorted[" << k << "] ------\n";
                    // not relax_j implies sorted_relax_j_k equals sorted_j_k
                    add_hard_clause(first_relax_var + j, -sorted_relax->at(k), sorted_vec->at(k));
                    add_hard_clause(first_relax_var + j, sorted_relax->at(k), -sorted_vec->at(k));
                }
            }
            // add order encoding to each sorted vector after relaxation
            if (m_opt_mode != "core-guided") {
                for (const std::vector<int> *srel : sorted_relax_vecs)
                    order_encoding(*srel);
            }
            // at most i constraint on relaxation variables
            if (m_verbosity == 2)
                std::cout << "c ---------------- At most " << i << " Constraint ----------------\n";
            at_most(relax_vars, i);
            if (m_opt_mode == "core-guided")
                m_sorted_relax_collection.at(i-1) = sorted_relax_vecs;
            else
                m_sorted_relax_collection.push_back(sorted_relax_vecs);
        }
        else { // last iteration
            // choose exactly one obj function to minimise
            int k = 0;
            for (int relax_var : relax_vars) {
                std::vector<int> *objective = m_objectives[k];
                size_t j = 0;
                for (const int lit : m_soft_clauses) {
                    int soft_var = -lit;
                    if (j >= objective->size()) {
                        // relax_vars[k] implies neg soft_var[j]
                        if (m_verbosity == 2)
                            std::cout << "c relax_var implies neg soft_var: ";
                        add_hard_clause(-relax_var, -soft_var);
                    }
                    else {
                        // relax_vars[k] implies objective[j] implies soft_var[j]
                        if (m_verbosity == 2)
                            std::cout << "c relax_var implies obj_var implies soft_var: ";
                        add_hard_clause(-relax_var, -(objective->at(j)), soft_var);
                        // let's check: relax_vars[k] implies soft_var[j] implies objective[j]
                        if (m_verbosity == 2)
                            std::cout << "c relax_var implies soft_var implies obj: ";
                        add_hard_clause(-relax_var, objective->at(j), -soft_var);
                    }
                    ++j;
                }
                ++k;
            }
            // encode at most 1 constraint to cnf
            if (m_verbosity == 2)
                std::cout << "c ---------------- At most " << 1 << " Constraint ----------------\n";
            at_most(relax_vars, 1);
            // encode at least 1 constraint to cnf
            const Clause cl (relax_vars.begin(), relax_vars.end());
            if (m_verbosity == 2)
                std::cout << "c ---------------- At least " << 1 << " Constraint ----------------\n";
            add_hard_clause(cl);
        }
    }

    size_t Encoder::largest_obj()
    {
        size_t largest = 0;
        for (std::vector<int> *objective : m_objectives) {
            if (objective->size() > largest)
                largest = objective->size();        
        }
        return largest;
    }

    void Encoder::componentwise_OR(int i, const std::vector<int> &max_vars)
    {
        if (m_verbosity == 2)
            std::cout << "c ------------ Componentwise OR ------------\n";
        std::vector<std::vector<int>*> *sorted_vecs(nullptr);
        if (i == 0) {
            // the OR is between sorted vecs
            sorted_vecs = &m_sorted_vecs;
        }
        else {
            // the OR is between sorted vecs after relaxation
            sorted_vecs = &m_sorted_relax_collection.at(i-1);
        }
        size_t k = 0;
        for (int max_var : max_vars) {
            std::vector<int> disjunction;
            for (int j = 0; j < m_num_objectives; ++j) {
                std::vector<int> *sorted_vec = sorted_vecs->at(j);
                // padding with zeros to the left
                size_t max_nb_wires = max_vars.size();
                if (k >= max_nb_wires - sorted_vec->size()) { // no undefined behaviour because max_nb_wires >= sorted_vec size
                    // add component of sorted_vec to the disjunction
                    size_t position = k - (max_nb_wires - sorted_vec->size());
                    int component = sorted_vec->at(position);
                    disjunction.push_back(component);
                }
            }
            // disjunction implies max variable
            for (int component : disjunction)
                add_hard_clause(max_var, -component);
            ++k;        
            // max variable implies disjunction -> It is not necessary
            /*disjunction.push_back(-max_var);
            add_hard_clause(disjunction);*/
        }
    }

    void Encoder::order_encoding(const std::vector<int>& vars)
    {
        if (m_verbosity == 2)
            std::cout << "c ------------ Order Encoding ------------\n";
        int size (vars.size());
        for (int i (0); i < size - 1; ++i) {
            // vars[i] implies vars[i+1]
            add_hard_clause(-vars.at(i), vars.at(i + 1));
        }
    }
    
    void Encoder::generate_soft_clauses(int i)
    {
        // find size of largest objective function
        size_t largest = largest_obj();
        // create new soft vars and soft clauses
        m_soft_clauses.resize(largest);
        std::vector<int> soft_vars(largest, 0);
        for (size_t j (0); j < largest; ++j) {
            int f (fresh());
            soft_vars.at(j) = f;
            m_soft_clauses.at(j) = -f;
        }
        if ((!m_simplify_last || i != m_num_objectives - 1))
            order_encoding(soft_vars);
        if (m_verbosity == 2)
            print_soft_clauses();
    }
    
    /* returns the lower bound of the optimal i-th max
     * the upper bound might change and can be computed using get_objective_vector()
     */
    int Encoder::encode_bounds(int i, int sum)
    {
        int lb (0);
        if (m_maxsat_presolve)
            lb = encode_lower_bound(i, sum);
        encode_upper_bound(i);
        return lb;
    }
    
    /* Computes lower bounds of (1) all objective functions and (2) of the cost
     * of the soft clauses of iteration i (the optimal ith maximum), based on
     * the minimum value of the sum of all objective functions
     * NOTE: in the last iteration (i == m_num_objectives - 1) lb_all = lb_soft,
     * since f >= minimum >= lb_soft, for each objective f
     * lb_all can not be improved (increased) with the existing information,
     * because otherwise we could improve (increase) lb_soft,
     * and it can not be improved with just the minimum value of the sum,
     * because this information can not exclude the scenario with all objectives equal
     */
    int Encoder::encode_lower_bound(int i, int sum)
    {
        std::vector<int> obj_vec (get_objective_vector());
        std::sort (obj_vec.begin(), obj_vec.end(), descending_order);
        // components 0 to i-1 have been minimised and are fixed
        int sum_fixed (0);
        for (int j (0); j <= i - 1; ++j)
            sum_fixed += obj_vec.at(j);
        int sum_not_fixed (sum - sum_fixed);
        int nb_components (m_num_objectives - i);
        int lb_soft (0);
        if (sum_not_fixed > 0) {
            // ceiling of the division
            lb_soft = sum_not_fixed / nb_components;
            if (sum_not_fixed % nb_components != 0)
                ++lb_soft;
        }
        int lb_all (0);
        int ub (obj_vec.at(i));
        if (i == m_num_objectives - 1)
            lb_all = lb_soft;
        else 
            lb_all = sum_not_fixed - ub * (m_num_objectives - i - 1);
        if (m_verbosity == 2) {
            std::cout << "c ------------ Lower bound encoding ------------\n";
            std::cout << "c nb_components: " << nb_components << '\n';
            std::cout << "c sum_fixed: " << sum_fixed << '\n';
            std::cout << "c sum_not_fixed: " << sum_not_fixed << '\n';
        }
        if (m_verbosity >= 1) {
            std::cout << "c Lower bound of optimum: " << lb_soft << '\n';
            std::cout << "c Lower bound on all objectives: " << lb_all << '\n';
        }
        // limit the cost of the soft clauses
        if (!m_simplify_last || i != m_num_objectives - 1)
            encode_lb_soft(lb_soft);
        // limit the value of all objective functions
        encode_lb_sorted(lb_all);
        return lb_soft;
    }
    
    void Encoder::encode_lb_soft(int lb)
    {
        if (m_verbosity == 2 && lb > 0)
            std::cout << "c ------------ Lower bound on soft clauses ------------\n";
        if (lb > 0) {
            int size (m_soft_clauses.size());
            int sc (m_soft_clauses.at(size - lb));
            add_hard_clause(-sc); // positive literal
        }
    }
    
    void Encoder::encode_lb_sorted(int lb)
    {
        if (m_verbosity == 2 && lb > 0)
            std::cout << "c ------------ Lower bound on Sorted Vecs ------------\n";
        for (const std::vector<int> *sorted_vec : m_sorted_vecs) {
            int size (sorted_vec->size());
            if (lb > 0) {
                if (m_verbosity == 2) {
                    std::cout << "c ----- Sorted Vec -----\n";
                    std::cout << "c size: " << size << '\n';
                    std::cout << "c lower bound: " << lb << '\n';
                }
                add_hard_clause(sorted_vec->at(size - lb)); // positive literal
            }
        }
    }
    
    // returns the upper bound of the optimal i-th max
    int Encoder::encode_upper_bound(int i)
    {
        std::vector<int> obj_vec (get_objective_vector());
        std::sort (obj_vec.begin(), obj_vec.end(), descending_order);
        if (m_verbosity == 2) {
            std::cout << "c ------------ Upper bound encoding ------------\n";
            std::cout << "c Sorted objective vector: ";
            for (int v : obj_vec)
                std::cout << v << ' ';
            std::cout << '\n';
        }
        if (m_verbosity >= 1 && m_verbosity <= 2) {
            if (i == 0) {
                std::cout << "c Trivial upper bound (size of largest objective): ";
                std::cout << m_soft_clauses.size() << '\n';
            }
            std::cout << "c Upper bound of optimum: ";
            std::cout << obj_vec.at(i) << '\n';
        }
        if (i == 0 || i == 1)
            encode_ub_sorted(obj_vec.at(0)); // ub on all obj funcs
        // in the last iteration, if m_simplify_last, current max is not sorted
        if ((!m_simplify_last || i != m_num_objectives - 1))
            encode_ub_soft(obj_vec.at(i)); // ub on current max
        return obj_vec.at(i);
    }
    
    // upper bound on current maximum
    void Encoder::encode_ub_soft(int max_i)
    {
        int pos (m_soft_clauses.size() - 1 - max_i);
        if (m_verbosity == 2) {
            std::cout << "c ------------ Upper bound on soft clauses ------------\n";
            std::cout << "c size: " << m_soft_clauses.size() << '\n';
            std::cout << "c upper bound: " << max_i << '\n';
        }
        if (pos >= 0) // pos may be -1 if ub is trivial
            add_hard_clause(m_soft_clauses.at(pos));
    }
    
    // upper bound on all objective functions (first and second iterations)
    void Encoder::encode_ub_sorted(int first_max)
    {
        // refine upper bound on all obj functions (sorted vecs)
        if (m_verbosity == 2)
            std::cout << "c ------------ Upper bound on Sorted Vecs ------------\n";
        for (const std::vector<int> *sorted_vec : m_sorted_vecs) {
            int size (sorted_vec->size());
            int pos (size - 1 - first_max); // pos might be < 0
            if (m_verbosity == 2) {
                std::cout << "c ----- Sorted Vec -----\n";
                std::cout << "c size: " << size << '\n';
                std::cout << "c upper bound: " << first_max << '\n';
            }
            if (pos >= 0)
                add_hard_clause(-sorted_vec->at(pos)); // neg sorted vec
        }
    }
    
    void Encoder::fix_only_some()
    {
        // y variables are sorted: 0...01...1
        // find first occurrence of 1
        size_t j (0);
        for (; j < m_soft_clauses.size(); ++j) {
            int var (-m_soft_clauses.at(j));
            if (m_solution.at(var) > 0)
                break;
        }
        if (j != m_soft_clauses.size()) {
            // y_j
            add_hard_clause(-m_soft_clauses.at(j));
        }
        if (j != 0) {
            // neg y_j-1
            add_hard_clause(m_soft_clauses.at(j - 1));
        }
    }
    
    void Encoder::fix_all(int i)
    {
        // Use objective vector because m_solution might not have the correct values
        std::vector<int> obj_vec (get_objective_vector());
        std::sort (obj_vec.begin(), obj_vec.end(), descending_order);
        int obj_val (obj_vec.at(i));
        int size (m_soft_clauses.size());
        // soft variables: size - obj_val zeros followed by obj_val ones
        for (int j (0); j < size; ++j) {
            int sc (m_soft_clauses.at(j));
            if (j >= size - obj_val)
                add_hard_clause(-sc); // one
            else
                add_hard_clause(sc); // zero
        }
    }
    
    void Encoder::fix_soft_vars(int i)
    {
        if (m_verbosity == 2)
            std::cout << "c ---------- Fix value of current maximum ----------\n";
        // fix_only_some(); // this needs the order encoding
        fix_all(i); // this may allow the sat solver to eliminate the order encoding
    }
    
    void Encoder::solve()
    {
        double initial_time (read_cpu_time());
        if (m_status != '?') {
            print_error_msg("Called solve() twice. You must call set_problem() before calling solve()");
            exit(EXIT_FAILURE);
        }
        // check if solve() is called without a problem
        if (m_num_objectives == 0) {
            print_error_msg("Can not solve - no objective function");
            exit(EXIT_FAILURE);
        }
        if (m_hard_clauses.empty()) {
            print_error_msg("Can not solve - no hard clauses");
            exit(EXIT_FAILURE);
        }
        if (m_num_objectives == 1) {
            print_error_msg("Can not solve - single-objective problem");
            exit(EXIT_FAILURE);
        }
        // check if problem is satisfiable and compute bounds on first optimum
        const int sum (presolve()); // sum is used to compute lower bounds
        if (m_status == 'u')
            return;   
        if (m_opt_mode == "core-guided")
            solve_core_guided();
        else
            solve_first_enc(sum);            
        if (m_verbosity >= 1) // print total solving time
            print_time(read_cpu_time() - initial_time, "c Total solving CPU time: ");
    }
    
    // sum is the minimum value of the sum of the objective functions in case of presolving
    // it is used to compute a lower bound of the optimal value of the first maximum
    void Encoder::solve_first_enc(int sum)
    {
        if (m_verbosity >= 1)
            std::cout << "c Original SAT-based Algorithm - Solving...\n";
        // encode sorted vectors with sorting network
        encode_sorted();
        // iteratively call (SAT/MaxSAT/PBO/ILP) solver
        for (int i = 0; i < m_num_objectives; ++i) {
            clear_soft_clauses();
            generate_soft_clauses(i);
            // encode bounds obtained from presolving or previous iteration
            const int lb (encode_bounds(i, sum));
            if (i == 0 && m_pareto_presolve)
                pareto_presolve();
            if (i != 0) // in the first iteration i == 0 there is no relaxation
                encode_relaxation(i);
            // encode the componentwise OR between sorted vectors (except maybe in the last iteration)
            if (!m_simplify_last || i != m_num_objectives - 1) {
                std::vector<int> soft_vars (m_soft_clauses.size(), 0);
                for (size_t k (0); k < soft_vars.size(); ++k)
                    soft_vars.at(k) = -m_soft_clauses.at(k);
                componentwise_OR(i, soft_vars);
            }
            if (m_verbosity >= 1 && m_verbosity <= 2)
                std::cout << "c Minimising the " << ordinal(i+1) << " maximum..." << '\n';
            // call optimisation solver (external solver or internal optimisation with SAT solver)
            if (m_opt_mode == "external" || (i == m_num_objectives - 1 && m_simplify_last))
                external_solve(i);
            else
                internal_solve(i, lb); // can't use with simplify_last since internal solving
                // depends on the soft variables being sorted
            // fix value of current maximum (in the end of last iteration there is no need)
            if (i != m_num_objectives - 1)
                fix_soft_vars(i);
        }
        if (m_verbosity == 2)
            print_sorted_true();
    }
    
    void print_core(const std::vector<int> &core)
    {
        std::cout << "c Core: ";
        for (int l : core)
            std::cout << l << " ";
        std::cout << '\n';
    }
    
    void Encoder::gen_assumps(const std::vector<int> &lower_bounds, const std::vector<std::vector<int>> &max_vars_vec,
                     const std::vector<std::vector<int>> &inputs_not_sorted, std::vector<int> &assumps) const
    {
        if (m_verbosity == 2)
            std::cout << "c ----------- Assumptions -----------\n";
        // determine which maximum we are minimising
        size_t j (0);
        while (j < max_vars_vec.size() && !max_vars_vec.at(j).empty()) {
            ++j;
        }
        // determine size of assumps
        size_t new_size (j*max_vars_vec.at(0).size());
        for (const std::vector<int> &inputs : inputs_not_sorted)
            new_size += inputs.size();
        assumps.resize(new_size);
        size_t pos (0); // position where we insert the literal in assumps
        for (size_t k (0); k < j; ++k) {
            if (m_verbosity == 2)
                std::cout << "c " << ordinal(k+1) << " max = " << lower_bounds.at(k) << ": ";
            // add constraint that the kth max is equal to the kth lower bound
            for (size_t p (0); p < max_vars_vec.at(k).size(); ++p) {
                if (p < max_vars_vec.at(k).size() - lower_bounds.at(k)) {
                    // negation of max var
                    assumps.at(pos) = -(max_vars_vec.at(k).at(p));
                }
                else { // max var
                    assumps.at(pos) = max_vars_vec.at(k).at(p);
                }
                if (m_verbosity == 2)
                    std::cout << assumps.at(pos) << ' ';
                ++pos;
            }
            if (m_verbosity == 2)
                std::cout << '\n';
        }
        for (size_t k (0); k < inputs_not_sorted.size(); ++k) {
            if (m_verbosity == 2)
                std::cout << "c negate the " << ordinal(k+1) << " obj vars that are not in the sorting network: ";
            for (int v : inputs_not_sorted.at(k)) {
                assumps.at(pos) = -v;
                ++pos;
                if (m_verbosity == 2)
                    std::cout << -v << ' ';
            }
            if (m_verbosity == 2)
                std::cout << '\n';
        }
    }
    
    /* Generate fresh variables corresponding to the variables whose sum is the ith max
     * Put the collection of those variables in entry i of max_vars_vec
     */
    void Encoder::generate_max_vars(int i, std::vector<std::vector<int>> &max_vars_vec)
    {
        if (m_verbosity == 2)
            std::cout << "c " << ordinal(i + 1) << " maximum variables\n";
        // the number of max_vars is equal to the maximum number of wires of the sorting networks
        int max_nb_wires (0);
        for (const std::pair<int, int> &info : m_snet_info) {
            const int nb_wires (info.first);
            if (nb_wires > max_nb_wires)
                max_nb_wires = nb_wires;
        }
        max_vars_vec.at(i).resize(max_nb_wires);
        for (int j (0); j < max_nb_wires; ++j) {
            max_vars_vec.at(i).at(j) = fresh();
            if (m_verbosity == 2 && j == 0)
                std::cout << "c " << max_vars_vec.at(i).at(j) << " ... ";
            if (m_verbosity == 2 && j == max_nb_wires - 1)
                std::cout << max_vars_vec.at(i).at(j) << '\n';
        }
    }
    
    /* Remove the obj vars in inputs_not_sorted that are in core and put them in new_inputs
     * Return true if the core intersects the obj vars and false otherwise
     */
    bool find_vars_in_core(std::vector<int> &inputs_not_sorted, const std::vector<int> &core, std::vector<int> &new_inputs)
    {
        new_inputs.clear();
        bool intersects (false);
        // the core is {l1, l2, ..., ln} and -li is what appears in the assumptions
        for (int l : core) {
            // find l in inputs_not_sorted
            size_t j (0);
            while (j < inputs_not_sorted.size() && inputs_not_sorted.at(j) != l) {
                ++j;
            }
            if (j < inputs_not_sorted.size()) { // found
                intersects = true;
                // remove from inputs_not_sorted and put in new_inputs
                new_inputs.push_back(l);
                // remove by puting the last element in the position j and erasing the last entry
                inputs_not_sorted.at(j) = inputs_not_sorted.back();
                inputs_not_sorted.pop_back();
            }
        }
        return intersects;
    }
    
    void Encoder::solve_core_guided()
    {
        // first try to satisfy all soft clauses/ falsify all obj variables
        std::vector<int> assumps;
        for (const std::vector<int> *obj : m_objectives) {
            for (int v : *obj)
                assumps.push_back(-v);
        }
        if (m_verbosity >= 1)
            std::cout << "c Core Guided Algorithm - Solving...\n";
        // start minimising each maximum using a core-guided search
        std::vector<int> lower_bounds (m_num_objectives, 0);
        std::vector<std::vector<int>> max_vars_vec (m_num_objectives, std::vector<int>());
        std::vector<std::vector<int>> inputs_not_sorted (m_num_objectives, std::vector<int>());
        for (int j (0); j < m_num_objectives; ++j) {
            const std::vector<int> *obj (m_objectives.at(j));
            inputs_not_sorted.at(j) = *obj;
        }
        for (int i (0); i < m_num_objectives; ++i) {
            if (m_verbosity >= 1)
                std::cout << "c Minimising the " << ordinal(i + 1) << " maximum...\n";
            if (i > 0) { // check if the ith max can be zero
                // encode relaxation and componentwise disjunction
                encode_relaxation(i);
                generate_max_vars(i, max_vars_vec);
                componentwise_OR(i, max_vars_vec.at(i));
                gen_assumps(lower_bounds, max_vars_vec, inputs_not_sorted, assumps);
            }
            while (!call_sat_solver(assumps)) {
                std::vector<int> core (m_sat_solver->conflict());
                if (m_verbosity >= 1)
                    std::cout << "c Core size: " << core.size() << '\n';
                if (m_verbosity == 2)
                    print_core(core);
                bool intersects (false);
                for (int j (0); j < m_num_objectives; ++j) {
                    std::vector<int> new_inputs;
                    // if core intersects input soft clauses change sorting networks
                    if (find_vars_in_core(inputs_not_sorted.at(j), core, new_inputs)) {
                        intersects = true;
                        if (m_verbosity >= 1)
                            std::cout << "c The core intersects the " << ordinal(j+1) << " objective\n";
                        if (m_verbosity == 2)
                            print_objs_sorted(inputs_not_sorted.at(j), j);
                        merge_core_guided(j, new_inputs);
                    }
                }
                if (m_verbosity >= 1) {
                    for (int j (0); j < m_num_objectives; ++j)
                        print_snet_info(j);
                }
                if (!intersects) { // increase ith lower bound
                    ++lower_bounds.at(i);
                    // TODO: check if it is possible to increase by more than 1
                    if (m_verbosity >= 1) {
                        std::cout << "c The core does not intersect the objective functions\n";
                        std::cout << "c Lower bound of " << ordinal(i + 1) << " maximum is increased to: ";
                        std::cout << lower_bounds.at(i) << '\n';
                    }
                }
                else { // if intersects then we need to repeat the encoding with new variables
                    for (int j (0); j <= i ; ++j) {
                        if (j > 0)
                            encode_relaxation(j);
                        generate_max_vars(j, max_vars_vec);
                        componentwise_OR(j, max_vars_vec.at(j));
                    }
                }
                gen_assumps(lower_bounds, max_vars_vec, inputs_not_sorted, assumps);
            }
        }
    }

}/* namespace leximaxIST */
