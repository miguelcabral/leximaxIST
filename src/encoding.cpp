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
            if (m_verbosity == 2) {
                std::cout << "c Combination: ";
                print_clause(std::cout, &clause);
            }
            add_hard_clause(clause);
        }
        // when i == 1, then each element of set is a subset of size 1
        else if (i == 1) {
            for (int elem : set) {
                clause[size-1] = -elem;
                // add clause to constraints
                if (m_verbosity == 2) {
                    std::cout << "c Combination: ";
                    print_clause(std::cout, &clause);
                }
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
                    const Clause *cl (add_hard_clause(-(first_relax_var + j), -sorted_relax->at(k)));
                    if (m_verbosity == 2) {
                        std::cout << "c sorted_relax[" << k << "]: " << sorted_relax->at(k) << "\n";
                        std::cout << "c -------------- relax_var implies not sorted_relax["<< k << "] ------\n";
                        print_clause(std::cout, cl, "c ");
                        std::cout << "c ------- not relax_var implies sorted_relax["<< k << "] equals sorted[" << k << "] ------\n";
                    }
                    // not relax_j implies sorted_relax_j_k equals sorted_j_k
                    cl = add_hard_clause(first_relax_var + j, -sorted_relax->at(k), sorted_vec->at(k));
                    if (m_verbosity == 2)
                        print_clause(std::cout, cl, "c ");
                    cl = add_hard_clause(first_relax_var + j, sorted_relax->at(k), -(sorted_vec->at(k)));
                    if (m_verbosity == 2)
                        print_clause(std::cout, cl, "c ");
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
                        const Clause *cl (add_hard_clause(-relax_var, -soft_var));
                        if (m_verbosity == 2) {
                            std::cout << "c relax_var implies neg soft_var: ";
                            print_clause(std::cout, cl);
                        }
                    }
                    else {
                        // relax_vars[k] implies objective[j] implies soft_var[j]
                        const Clause *cl (add_hard_clause(-relax_var, -(objective->at(j)), soft_var));
                        if (m_verbosity == 2) {
                            std::cout << "c relax_var implies obj_var implies soft_var: ";
                            print_clause(std::cout, cl);
                        }
                        // let's check: relax_vars[k] implies soft_var[j] implies objective[j]
                        cl = add_hard_clause(-relax_var, objective->at(j), -soft_var);
                        if (m_verbosity == 2) {
                            std::cout << "c relax_var implies soft_var implies obj: ";
                            print_clause(std::cout, cl);
                        }
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
            add_hard_clause(cl);
            if (m_verbosity == 2) {
                std::cout << "c ---------------- At least " << 1 << " Constraint ----------------\n";
                print_clause(std::cout, &cl, "c ");
            }
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
            for (int component : disjunction) {
                const Clause *c (add_hard_clause(-soft_lit, -component));
                if (m_verbosity == 2)
                    print_clause(std::cout, c, "c ");
            }
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
            const Clause *c = add_hard_clause(-vars.at(i), vars.at(i + 1));
            if (m_verbosity == 2)
                print_clause(std::cout, c, "c ");
        }
    }
    
    void Encoder::generate_soft_clauses()
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
        order_encoding(soft_vars);
        if (m_verbosity == 2)
            print_soft_clauses();
    }
    
    // returns the upper bound of the optimal i-th max
    int Encoder::encode_upper_bound(int i)
    {
        if (i == 0 && m_ub_presolve == 0)
            return m_soft_clauses.size();
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
                std::cout << m_soft_clauses.size() << std::endl;
            }
            std::cout << "c Upper bound of current optimal maximum: ";
            std::cout << obj_vec.at(i) << std::endl;
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
        const Clause *c (add_hard_clause(m_soft_clauses.at(pos)));
        if (m_verbosity == 2)
            print_clause(std::cout, c, "c ");
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
            if (pos >= 0) {
                const Clause *c (add_hard_clause(-(sorted_vec->at(pos)))); // neg sorted vec
                if (m_verbosity == 2)
                    print_clause(std::cout, c, "c ");
            }
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
            const Clause *c (add_hard_clause(-m_soft_clauses.at(j)));
            if (m_verbosity == 2)
                print_clause(std::cout, c, "c ");
        }
        if (j != 0) {
            // neg y_j-1
            const Clause *c (add_hard_clause(m_soft_clauses.at(j - 1)));
            if (m_verbosity == 2)
                print_clause(std::cout, c, "c ");
        }
    }
    
    void Encoder::fix_all()
    {
        for (int l : m_soft_clauses) {
            const Clause *c (add_hard_clause(m_solution.at(std::abs(l))));
            if (m_verbosity == 2)
                print_clause(std::cout, c, "c ");
        }
    }
    
    void Encoder::fix_soft_vars()
    {
        if (m_verbosity == 2)
            std::cout << "c ---------- Fix value of current maximum ----------\n";
        // fix_only_some(); // this needs the order encoding
        fix_all(); // this may allow the sat solver to eliminate the order encoding
    }
    
    void Encoder::solve()
    {
        double initial_time (read_cpu_time());
        if (m_num_objectives == 0) {
            print_error_msg("Can not solve - no objective function");
            exit(EXIT_FAILURE);
        }
        if (m_hard_clauses.empty()) {
            print_error_msg("Can not solve - no hard clauses");
            exit(EXIT_FAILURE);
        }
        // if there is only one objective function then it is a simple single objective problem
        if (m_num_objectives == 1) {
            if (m_opt_mode != "external") {
                print_error_msg("Can't use internal optimisation in single-objective problems");
                exit(EXIT_FAILURE);
            }
            generate_soft_clauses();
            external_solve(0);
            return;
        }
        if (m_ub_presolve != 0) { // call sat solver (once or MSS) to get upper bound of optimum
            calculate_upper_bound();
            if (m_status == 'u')
                return;
        }
        // encode sorted vectors with sorting network
        encode_sorted();
        // iteratively call (SAT/MaxSAT/PBO/ILP) solver
        for (int i = 0; i < m_num_objectives; ++i) {
            if (m_verbosity >= 1 && m_verbosity <= 2)
                std::cout << "c Minimising " << ordinal(i+1) << " maximum..." << std::endl;
            clear_soft_clauses();
            generate_soft_clauses();
            if (i != 0) // in the first iteration i == 0 there is no relaxation
                encode_relaxation(i);
            // encode the componentwise OR between sorted_relax vectors (except maybe in the last iteration)
            if (!m_simplify_last || i != m_num_objectives - 1)
                componentwise_OR(i);
            // encode upper bound obtained from presolving or previous iteration
            int ub (encode_upper_bound(i));
            // call optimisation solver (external solver or internal optimisation with SAT solver)
            if (m_opt_mode == "external")
                external_solve(i);
            else
                internal_solve(i, ub);
            // if unsat end computation
            if (m_status == 'u')
                return;
            // fix value of current maximum; in the end of last iteration there is no need for this
            if (i != m_num_objectives - 1)
                fix_soft_vars();
        }
        if (m_verbosity == 2)
            print_sorted_true();
        if (m_verbosity > 0 && m_verbosity < 3) { // print total solving time
            std::cout << "c Total solving CPU time: ";
            std::cout << read_cpu_time() - initial_time << 's' << std::endl;
        }
    }

}/* namespace leximaxIST */
