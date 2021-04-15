#include <leximaxIST_Encoder.h>
#include <leximaxIST_rusage.h>
#include <leximaxIST_error.h>
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
    
    std::string ordinal (int i)
    {
        std::string i_str (std::to_string(i));
        if (i_str.back() == '1')
            i_str += "st";
        else if (i_str.back() == '2')
            i_str += "nd";
        else if (i_str.back() == '3')
            i_str += "rd";
        else
            i_str += "th";
        return i_str;
    }
    
    void Encoder::encode_sorted()
    {
        if (m_num_objectives != 1) { // when there is only one objective function there is no need for this
            for (int i{0}; i < m_num_objectives; ++i) {   
                const std::vector<int> *objective = m_objectives[i];
                const size_t num_terms = objective->size();
                m_sorted_vecs[i] = new std::vector<int>(num_terms, 0);
                // sorting_network is initialized to a vector of pairs (-1,-1)
                SNET sorting_network(num_terms, {-1,-1});
                // elems_to_sort is represented by a pair (first element, number of elements).
                std::pair<int,int> elems_to_sort(0, num_terms);
                if (m_verbosity == 2)
                    print_obj_func(i);
                m_sorting_net_size = 0; // it is incremented every time a comparator is inserted
                if (m_verbosity == 2)
                    std::cout << "c -------- Sorting Network Encoding --------\n";
                encode_network(elems_to_sort, objective, sorting_network);
                if (m_verbosity >= 1)
                    print_snet_size(i);
                // sorted_vec variables are the outputs of sorting_network
                if (num_terms == 1) { // in this case the sorting network is empty
                    std::vector<int> *sorted_vec = m_sorted_vecs[i];
                    sorted_vec->at(0) = objective->at(0);
                }
                else {
                    for (size_t j{0}; j < num_terms; j++) {
                        int output_j = sorting_network[j].second;
                        std::vector<int> *sorted_vec = m_sorted_vecs[i];
                        sorted_vec->at(j) = output_j;
                    }
                }
                if (m_verbosity == 2)
                    print_sorted_vec(i);
            }
            // add order encoding to each sorted vector
            for (const std::vector<int> *svec : m_sorted_vecs)
                order_encoding(*svec);
        }
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

    void Encoder::encode_relaxation(int i)
    {
        std::list<int> relax_vars;
        for (int j = 0; j < m_num_objectives; ++j)
            relax_vars.push_back(fresh());
        m_all_relax_vars.push_back(relax_vars);
        int first_relax_var (relax_vars.front());
        if (m_verbosity == 2) {
            std::cout << "c ------------ Relaxation variables of iteration " << i << " ------------\n";
            for (int v : relax_vars)
                std::cout << "c " << v << '\n';
            if (!m_simplify_last || i != m_num_objectives - 1)
                std::cout << "c ------------ Sorted vecs after relax of iteration " << i << " ------------\n";
        }
        if (!m_simplify_last || i != m_num_objectives - 1) {
            std::vector<std::vector<int>*> sorted_relax_vecs(m_num_objectives, nullptr);
            for (int j = 0; j < m_num_objectives; ++j) {
                // encode relaxation variable of the j-th objective
                std::vector<int> *sorted_vec = m_sorted_vecs[j];
                std::vector<int> *sorted_relax = new std::vector<int>(sorted_vec->size(), 0);
                sorted_relax_vecs[j] = sorted_relax;
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
            for (const std::vector<int> *srel : sorted_relax_vecs)
                order_encoding(*srel);
            // at most i constraint on relaxation variables
            if (m_verbosity == 2)
                std::cout << "c ---------------- At most " << i << " Constraint ----------------\n";
            at_most(relax_vars, i);
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

    void Encoder::componentwise_OR(int i)
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
            sorted_vecs = &m_sorted_relax_collection[i-1];
        }
        size_t k = 0;
        for (int soft_lit : m_soft_clauses) {
            std::vector<int> disjunction;
            for (int j = 0; j < m_num_objectives; ++j) {
                std::vector<int> *sorted_vec = sorted_vecs->at(j);
                // padding with zeros to the left
                size_t largest = m_soft_clauses.size();
                if (k >= largest - sorted_vec->size()) { // no undefined behaviour because largest is the nb of vars of the largest obj.
                    // add component of sorted_vec to the disjunction
                    size_t position = k - (largest - sorted_vec->size());
                    int component = sorted_vec->at(position);
                    disjunction.push_back(component);
                }
            }
            // disjunction implies soft variable
            for (int component : disjunction)
                add_hard_clause(-soft_lit, -component);
            ++k;        
            // soft variable implies disjunction -> It is not necessary
            /*disjunction.push_back(soft_lit);
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
        // if m_num_objectives is 1 then this is a single objective problem
        if (m_num_objectives == 1) {
            std::vector<int> *objective = m_objectives[0];
            m_soft_clauses = *objective;
            for (size_t j (0); j < objective->size(); ++j)
                m_soft_clauses.at(j) = -objective->at(j);
            return;
        }
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
    
    // returns bounds of the optimal i-th max (lower bound, upper bound)
    std::pair<int, int> Encoder::encode_bounds(int i, int sum)
    {
        int lb (0);
        if (m_maxsat_presolve)
            lb = encode_lower_bound(i, sum);
        int ub (encode_upper_bound(i));
        std::pair<int, int> bounds (lb, ub);
        return bounds;
    }
    
    // computes lower bounds of all objective functions and of the 
    int Encoder::encode_lower_bound(int i, int sum)
    {
        const std::vector<int> &obj_vec (get_objective_vector());
        std::sort (obj_vec.begin(), obj_vec.end(), descending_order);
        // components 0 to i-1 have been minimised and are fixed
        int sum_fixed (0);
        for (int j (0); j <= i - 1; ++j)
            sum_fixed += obj_vec.at(j);
        int sum_not_fixed (sum - sum_fixed);
        int nb_components (m_num_objectives - i);
        // ceiling of the division
        int lb_soft (sum_not_fixed / nb_components);
        if (sum % nb_components != 0)
            ++lb_soft;
        // lower bound on all objective functions
        // in the last iteration (i == m_num_objectives - 1) lb_all = lb_soft, since
        // f >= minimum >= lb_soft, for each objective f
        // lb_all can not be improved (increased) with the existing information, because
        // otherwise we could improve (increase) lb_soft,
        // and it can not be improved with just the minimum value of the sum,
        // because this information can not exclude the scenario with all objectives equal
        int lb_all (0);
        int ub (obj_vec.at(i));
        if (i == m_num_objectives - 1)
            lb_all = lb_soft;
        else 
            lb_all = sum_not_fixed - ub * (m_num_objectives - i - 1);
        if (m_verbosity == 2)
            std::cout << "c ------------ Lower bound encoding ------------\n";
        if (m_verbosity >= 1) {
            std::cout << "c Lower bound of optimum: " << lb_soft << '\n';
            std::cout << "c Lower bound on all objectives: " << lb_all << '\n';
        }
        // TODO: constraint on the soft clauses + constraint on all obj funcs (through snet outputs)
        // I know that (sum - fi)/(n-1) <= ub, which means we have a lower bound on all obj funcs
        // limit the cost of the soft clauses

        encode_lb_soft(lb_soft);
        encode_lb_sorted(lb_all);
        return lb_soft;
    }
    
    void Encoder::encode_lb_soft(int lb)
    {
        if (lb > 0) {
            int size (m_soft_clauses.size());
            int sc (m_soft_clauses.at(size - lb));
            add_hard_clause(-sc); // positive literal
        }
    }
    
    void Encoder::encode_lb_sorted(int lb)
    {
        if (m_verbosity == 2)
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
                std::cout << "c Trival upper bound (size of largest objective): ";
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
        // encode sorted vectors with sorting network
        encode_sorted();
        // iteratively call (SAT/MaxSAT/PBO/ILP) solver
        for (int i = 0; i < m_num_objectives; ++i) {
            if (m_verbosity >= 1 && m_verbosity <= 2)
                std::cout << "c Minimising " << ordinal(i+1) << " maximum..." << '\n';
            clear_soft_clauses();
            generate_soft_clauses(i);
            if (i != 0) // in the first iteration i == 0 there is no relaxation
                encode_relaxation(i);
            // encode the componentwise OR between sorted_relax vectors (except maybe in the last iteration)
            if (!m_simplify_last || i != m_num_objectives - 1)
                componentwise_OR(i);
            // encode bounds obtained from presolving or previous iteration
            const std::pair<int, int> &bounds (encode_bounds(i, sum));
            // call optimisation solver (external solver or internal optimisation with SAT solver)
            if (m_opt_mode == "external" || (i == m_num_objectives - 1 && m_simplify_last))
                external_solve(i);
            else
                internal_solve(i, bounds.first, bounds.second); // can't use with simplify_last since it depends on order
            // fix value of current maximum; in the end of last iteration there is no need for this
            if (i != m_num_objectives - 1)
                fix_soft_vars(i);
        }
        if (m_verbosity == 2)
            print_sorted_true();
        if (m_verbosity > 0 && m_verbosity < 3) { // print total solving time
            std::cout << "c Total solving CPU time: ";
            std::cout << read_cpu_time() - initial_time << 's' << '\n';
        }
    }

}/* namespace leximaxIST */
