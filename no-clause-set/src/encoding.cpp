#include <leximaxIST_Encoder.h>
#include <leximaxIST_error.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <utility>
#include <algorithm>
#include <list>

namespace leximaxIST {

    bool descending_order (int i, int j);
    
    void Encoder::encode_sorted()
    {
        if (m_num_objectives != 1) { // when there is only one objective function there is no need for this
            for (int i{0}; i < m_num_objectives; ++i) {   
                std::vector<int> *objective = m_objectives[i];
                const size_t num_terms = objective->size();
                m_sorted_vecs[i] = new std::vector<int>(num_terms, 0);
                // sorting_network is initialized to a vector of pairs (-1,-1)
                SNET sorting_network(num_terms, {-1,-1});
                // elems_to_sort is represented by a pair (first element, number of elements).
                std::pair<int,int> elems_to_sort(0, num_terms);
                if (m_verbosity == 2) {
                    std::cout << "c --------------- Objective Function " << i << " (size = ";
                    std::cout << num_terms << ") --------------\n";
                    for (size_t j = 0; j < num_terms; ++j)
                        std::cout << objective->at(j) << '\n';
                }
                size_t old_snet_size (m_sorting_net_size);
                m_sorting_net_size = 0;
                encode_network(elems_to_sort, objective, sorting_network);
                if (m_verbosity == 2)
                    std::cout << "c -------------- Size of Sorting Network " << i << ": " << m_sorting_net_size << " --------------\n";
                if (old_snet_size > m_sorting_net_size)
                    m_sorting_net_size = old_snet_size; // in the end m_sorting_net_size is the size of the largest sorting network
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
                if (m_verbosity == 2) {
                    std::cout << "c ---------------- m_sorted_vecs[" << i << "] -----------------\n";
                    for(size_t j{0}; j < num_terms; j++)
                        std::cout << "c sorted_vec[" << j << "]: " << m_sorted_vecs[i]->at(j) << '\n';
                }
            }
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
        m_all_relax_vars.push_back(relax_vars); // TODO: maybe move the relax_vars
        int first_relax_var (relax_vars.front());
        if (m_verbosity == 2) {
            std::cout << "c ------------ Relaxation variables of iteration " << i << " ------------\n";
            for (int v : relax_vars)
                std::cout << v << '\n';
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
                    Clause cl;
                    // relax_j implies not sorted_relax_j_k
                    cl.push_back(-(first_relax_var + j));
                    cl.push_back(-sorted_relax->at(k));
                    if (m_verbosity == 2) {
                        std::cout << "c sorted_relax[" << k << "]: " << sorted_relax->at(k) << "\n";
                        std::cout << "c -------------- relax_var implies not sorted_relax["<< k << "] ------\n";
                        print_clause(std::cout, &cl, "c ");
                        std::cout << "c ------- not relax_var implies sorted_relax["<< k << "] equals sorted[" << k << "] ------\n";
                    }
                    add_hard_clause(cl);
                    // not relax_j implies sorted_relax_j_k equals sorted_j_k
                    cl.push_back(first_relax_var + j);
                    cl.push_back(-sorted_relax->at(k));
                    cl.push_back(sorted_vec->at(k));
                    if (m_verbosity == 2)
                        print_clause(std::cout, &cl, "c ");
                    add_hard_clause(cl);
                    cl.push_back(first_relax_var + j);
                    cl.push_back(sorted_relax->at(k));
                    cl.push_back(-(sorted_vec->at(k)));
                    if (m_verbosity == 2)
                        print_clause(std::cout, &cl, "c ");
                    add_hard_clause(cl);

                }
            }
            // at most i constraint 
            if(m_formalism == "wcnf") {
                if (m_verbosity == 2)
                    std::cout << "c ---------------- At most " << i << " Constraint ----------------\n";
                at_most(relax_vars, i);
                // at least i constraint -> is not necessary
            }
            m_sorted_relax_collection.push_back(sorted_relax_vecs);
        }
        else { // last iteration
            // choose exactly one obj function to minimise
            int k = 0;
            for (int relax_var : relax_vars) {
                std::vector<int> *objective = m_objectives[k];
                size_t j = 0;
                for (Clause *cl : m_soft_clauses) {
                    int soft_var = -(*(cl->begin()));
                    if (j >= objective->size()) {
                        // relax_vars[k] implies neg soft_var[j]
                        Clause cl {-relax_var, -soft_var};
                        if (m_verbosity == 2) {
                            std::cout << "c relax_var implies neg soft_var: ";
                            print_clause(std::cout, &cl);
                        }
                        add_hard_clause(cl);
                    }
                    else {
                        // relax_vars[k] implies objective[j] implies soft_var[j]
                        Clause cl {-relax_var, -(objective->at(j)), soft_var};
                        if (m_verbosity == 2) {
                            std::cout << "c relax_var implies obj_var implies soft_var: ";
                            print_clause(std::cout, &cl);
                        }
                        add_hard_clause(cl);
                        cl.resize(3);
                        // let's check: m_relax_vars[k] implies soft_var[j] implies objective[j]
                        cl[0] = -relax_var;
                        cl[1] = objective->at(j);
                        cl[2] = -soft_var;
                        if (m_verbosity == 2) {
                            std::cout << "c relax_var implies soft_var implies obj: ";
                            print_clause(std::cout, &cl);
                        }
                        add_hard_clause(cl);
                    }
                    ++j;
                }
                ++k;
            }
            if(m_formalism == "wcnf") { // when solving with pbo or lp, this constraint is written before calling the solver
                // at most 1 constraint
                if (m_verbosity == 2)
                    std::cout << "c ---------------- At most " << 1 << " Constraint ----------------\n";
                at_most(relax_vars, 1);
                // at least 1 constraint
                Clause cl (relax_vars.begin(), relax_vars.end());
                if (m_verbosity == 2) {
                    std::cout << "c ---------------- At least " << 1 << " Constraint ----------------\n";
                    print_clause(std::cout, &cl, "c ");
                }
                add_hard_clause(cl);
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
        for (Clause *cl : m_soft_clauses) {
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
            int soft_lit = *(cl->begin());
            for (int component : disjunction) {
                Clause c;
                c.push_back(-soft_lit);
                c.push_back(-component);
                add_hard_clause(c);
                if (m_verbosity == 2)
                    print_clause(std::cout, &c, "c ");
            }
            ++k;        
            // soft variable implies disjunction -> It is not necessary
            /*disjunction.push_back(soft_lit);
            add_hard_clause(disjunction);*/
        }
    }

    void Encoder::generate_soft_clauses(int i)
    {
        // if m_num_objectives is 1 then this is a single objective problem
        if (m_num_objectives == 1) {
            std::vector<int> *objective = m_objectives[0];
            for (size_t j (0); j < objective->size(); ++j) {
                Clause c (1, -(objective->at(j))); // fill constructor
                add_soft_clause(c);
            }
                return;
        }
        // find size of largest objective function
        size_t largest = largest_obj();
        // create new soft vars and soft clauses
        for (size_t j (0); j < largest; ++j) {
            Clause c (1, -fresh()); // fill constructor; unit clause, neg of var
            add_soft_clause(c);
        }
        if (m_verbosity == 2) {
            std::cout << "c ------------ Soft Clauses of iteration " << i << " ------------\n";
            for (const Clause *cl : m_soft_clauses)
                print_clause(std::cout, cl, "c ");
        }
    }
    
    void Encoder::encode_upper_bound(int i, std::vector<int> &old_obj_vec)
    {
        std::vector<int> obj_vec (get_objective_vector());
        if (obj_vec.empty())
            return;
        std::sort (obj_vec.begin(), obj_vec.end(), descending_order);
        m_ub_vec[i] = obj_vec[i];
        if (m_verbosity == 2) {
            std::cout << "c ------------ Upper bound encoding ------------\n";
            std::cout << "c Sorted objective vector: ";
            for (int v : obj_vec)
                std::cout << v << ' ';
            std::cout << '\n';
        }
        if (i == 0 || i == 1) { // refine upper bound on all obj functions (sorted vecs)
            if (m_verbosity == 2)
                std::cout << "c ------------ Upper bound on Sorted Vecs ------------\n";
            int first_max (obj_vec[0]);
            for (const std::vector<int> *sorted_vec : m_sorted_vecs) {
                int size (sorted_vec->size());
                int old_first_max ( i == 0 ? size : old_obj_vec[0]);
                int starting_position (size - old_first_max);
                if (starting_position < 0)
                    starting_position = 0;
                int end_position (size - 1 - first_max);
                if (m_verbosity == 2) {
                    std::cout << "c ----- Sorted Vec -----\n";
                    std::cout << "c size: " << size << '\n';
                    std::cout << "c upper bound: " << first_max << '\n';
                }
                for (int j (starting_position); j <= end_position; ++j) {
                    Clause cl;
                    cl.push_back(-(sorted_vec->at(j))); // neg sorted vec
                    if (m_verbosity == 2)
                        print_clause(std::cout, &cl, "c ");
                    add_hard_clause(cl);
                }
            }
        }
        if (!m_simplify_last || i != m_num_objectives - 1) { // upper bound on soft clauses
            int max_i (obj_vec[i]); // upper bound on minimum max_i
            m_ub_vec[i] = max_i; // update upper bound of current iteration
            int end_position (m_soft_clauses.size() - 1 - max_i);
            if (m_verbosity == 2) {
                std::cout << "c ------------ Upper bound on soft clauses ------------\n";
                std::cout << "c size: " << m_soft_clauses.size() << '\n';
                std::cout << "c upper bound: " << max_i << '\n';
            }
            for (int j (0); j <= end_position; ++j) {
                const Clause *cl (m_soft_clauses[j]);
                if (m_verbosity == 2)
                    print_clause(std::cout, cl, "c ");
                add_hard_clause(*cl);
            }
        }
        old_obj_vec = obj_vec;
    }
    
    void Encoder::debug_print_all(const std::vector<std::vector<int>> &true_ys, const std::vector<int> &y_vector)
    {
        if (!m_solution.empty()){
            // compare y_vector with objective vector and print true sorted vector variables
            std::cout << "c Y vector: ";
            for (int v : y_vector)
                std::cout << v << ' ';
            std::cout << '\n';
            std::vector<std::vector<int>> true_input;
            for (std::vector<int> *obj : m_objectives) {
                std::vector<int> true_obj_vars;
                for (int var : *obj) {
                    if (m_solution[var] > 0)
                        true_obj_vars.push_back(var);
                }
                true_input.push_back(true_obj_vars);
            }
            std::vector<int> obj_vector (m_num_objectives, 0);
            std::cout << "c Input vector: ";
            for (const std::vector<int> &true_obj_vars : true_input)
                std::cout << true_obj_vars.size() << ' ';
            std::cout << '\n';      
            std::cout << "c Y true variables:" << '\n';
            int j (0);
            for (const std::vector<int> &true_vars : true_ys) {
                std::cout << "c Maximum " << j << ": ";
                for (int var : true_vars)
                    std::cout << var << ' ';
                std::cout << '\n';
                ++j;
            }
            std::cout << "c Input true variables:" << '\n';
            j = 0;
            for (const std::vector<int> &true_vars : true_input) {
                std::cout << "c Function " << j << ": ";
                for (int var : true_vars)
                    std::cout << var << ' ';
                std::cout << '\n';
                ++j;
            }
            std::cout << "c Sorted vecs true variables:" << '\n';
            j = 0;
            for (std::vector<int> *sorted_vec : m_sorted_vecs) {
                std::cout << "c Sorted vec " << j << ": ";
                for (int var : *sorted_vec) {
                    if (m_solution[var] > 0)
                        std::cout << var << ' ';
                }
                std::cout << '\n';
                ++j;
            }
            j = 1;
            for (std::vector<std::vector<int>*> &sorted_relax_vecs : m_sorted_relax_collection) {
                std::cout << "c Sorted Relax Vecs true variables of iteration " << j << ":" << '\n';
                int k (0);
                for (std::vector<int> *sorted_relax : sorted_relax_vecs) {
                    if (sorted_relax != nullptr) {
                        std::cout << "c Sorted Relax vec " << k << ": ";
                        for (int var : *sorted_relax) {
                            if (m_solution[var] > 0)
                                std::cout << var << ' ';
                        }
                        std::cout << '\n';
                        ++k;
                    }
                }
                ++j;
            }
        }
    }
    
    int Encoder::solve()
    {
        if (m_num_objectives == 0) {
            print_error_msg("No objective function");
            return -1;
        }
        if (m_constraints.empty()) {
            print_error_msg("Empty constraints");
            return -1;
        }
        std::vector<std::vector<int>> true_ys; // for debugging
        std::vector<int> y_vector; // for debugging
        // if there is only one objective function then it is a simple single objective problem
        if (m_num_objectives == 1) {
            generate_soft_clauses(0);
            // call solver and read output 
            int retv = external_solve(0);
            return retv;
        }
        if (m_ub_encoding != 0) { // call sat solver (or MSS or MaxSAT/PBO/ILP solver) first to get upper bound of optimum
            if (calculate_upper_bound() != 0)
                return -1;
            if (!m_sat)
                return 0;
        }
        std::vector<int> old_obj_vec; // used in upper bound encoding
        // encode sorted vectors with sorting network
        encode_sorted();
        // iteratively call (MaxSAT, PBO or ILP) solver       
        for (int i = 0; i < m_num_objectives; ++i) {
            if (m_verbosity >= 1) {
                std::string i_str (std::to_string(i + 1));
                if (i_str.back() == '1')
                    i_str += "st";
                else if (i_str.back() == '2')
                    i_str += "nd";
                else if (i_str.back() == '3')
                    i_str += "rd";
                std::cout << "c Minimising " << i_str << " maximum..." << std::endl;
            }
            clear_soft_clauses();
            generate_soft_clauses(i);
            if (i != 0) // in the first iteration i == 0 there is no relaxation
                encode_relaxation(i);
            // encode the componentwise OR between sorted_relax vectors (except maybe in the last iteration)
            if (!m_simplify_last || i != m_num_objectives - 1)
                componentwise_OR(i);
            // encode upper bound obtained from sat solver (or MSS or MaxSAT)
            if (m_ub_encoding != 0 && (!m_simplify_last || i != m_num_objectives - 1)) {
                encode_upper_bound(i, old_obj_vec);
            }
            // call solver
            if (external_solve(i) == -1)
                return -1;
            // if unsat end computation
            if (!m_sat)
                return 0;
            if (m_verbosity >= 1 && m_verbosity <= 2)
                print_obj_vector();
            // fix value of current maximum; in the end of last iteration there is no need for this
            if (i != m_num_objectives - 1) {
                for (Clause *scl : m_soft_clauses) {
                    int soft_var = -(*(scl->begin()));
                    Clause hc (1, m_solution[soft_var]); // fill constructor
                    add_hard_clause(hc);
                }
            }
            if (m_verbosity == 2) {
                std::vector<int> current_true_ys;
                for (Clause *cl : m_soft_clauses) {
                    int soft_var = -(*(cl->begin()));
                    if (m_solution[soft_var] > 0)
                        current_true_ys.push_back(soft_var);
                }
                y_vector.push_back(current_true_ys.size());
                true_ys.push_back(current_true_ys);
            }
        }
        if (m_verbosity == 2)
            debug_print_all(true_ys, y_vector);
        return 0;
    }

}/* namespace leximaxIST */
