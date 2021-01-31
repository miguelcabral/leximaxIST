#include <leximaxIST_Encoder.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <utility>
#include <forward_list>

namespace leximaxIST {

    void Encoder::encode_sorted()
    {
        if (m_num_objectives != 1) { // when there is only one objective function there is no need for this
            for (int i{0}; i < m_num_objectives; ++i) {   
                std::vector<long long> *objective = m_objectives[i];
                size_t num_terms = objective->size();
                m_sorted_vecs[i] = new std::vector<long long>(num_terms, 0);
                SNET sorting_network(num_terms, nullptr); // sorting_network is initialized to a vector of nullptrs
                // elems_to_sort is represented by a pair (first element, number of elements).
                std::pair<long long,long long> elems_to_sort(0, num_terms);
                if (m_debug) {
                    std::cerr << "--------------- Objective Function " << i << " --------------\n";
                    for (size_t j = 0; j < num_terms; ++j) {
                        std::cerr << objective->at(j);
                        if (j != num_terms - 1)
                            std::cerr << " + ";
                        else
                            std::cerr << '\n';
                    }
                }
                size_t old_snet_size (m_sorting_net_size);
                m_sorting_net_size = 0;
                encode_network(elems_to_sort, objective, sorting_network);
                if (m_debug)
                    std::cerr << "-------------- Size of Sorting Network " << i << ": " << m_sorting_net_size << " --------------\n";
                if (old_snet_size > m_sorting_net_size)
                    m_sorting_net_size = old_snet_size; // in the end m_sorting_net_size is the size of the largest sorting network
                // sorted_vec variables are the outputs of sorting_network
                if (num_terms == 1) { // in this case the sorting network is empty
                    std::vector<long long> *sorted_vec = m_sorted_vecs[i];
                    sorted_vec->at(0) = objective->at(0);
                }
                else {
                    for (size_t j{0}; j < num_terms; j++) {
                        long long output_j = sorting_network[j]->second;
                        std::vector<long long> *sorted_vec = m_sorted_vecs[i];
                        sorted_vec->at(j) = output_j;
                    }
                }
                // free memory allocated for the comparators of the sorting network
                delete_snet(sorting_network);
                if (m_debug) {
                    std::cerr << "---------------- m_sorted_vecs[" << i << "] -----------------\n";
                    for(size_t j{0}; j < num_terms; j++)
                        std::cerr << "sorted_vec[" << j << "]: " << m_sorted_vecs[i]->at(j) << '\n';
                }
            }
        }
    }

    int list_size(const std::forward_list<long long> &mylist)
    {
        return std::distance(mylist.begin(), mylist.end());
    }

    void Encoder::all_subsets(std::forward_list<long long> set, int i, std::vector<long long> &clause_vec)
    {
        std::vector<long long> lits;
        int size = clause_vec.size();
        // Base of recursion: 
        // when |set| == i, then set is the only subset of size i
        int set_size = list_size(set);
        if (set_size == i) {
            int j = size - i;
            for (long long elem : set) {
                clause_vec[j] = -elem;
                j++;
            }
            // add clause to constraints
            if (m_debug) {
                std::cerr << "Combination: ";
                print_clause(std::cerr, &clause_vec);
            }
            lits.clear();
            for (long long lit : clause_vec)
                lits.push_back(lit);
            add_hard_clause(lits);
        }
        // when i == 1, then each element of set is a subset of size 1
        else if (i == 1) {
            for (long long elem : set) {
                clause_vec[size-1] = -elem;
                // add clause to constraints
            if (m_debug) {
                std::cerr << "Combination: ";
                print_clause(std::cerr, &clause_vec);
            }
                lits.clear();
                for (long long lit : clause_vec)
                    lits.push_back(lit);
                add_hard_clause(lits);
            }
        }
        else {
        // Step of recursion: the combinations that include the first element of set + those that don't include it
        long long first_el = set.front();
        set.pop_front();
        clause_vec[size - i] = -first_el;
        all_subsets(set, i-1, clause_vec); // combinations that include first_el
        all_subsets(set, i, clause_vec); // combinations that don't include first_el
        }
    }

    void Encoder::at_most(std::forward_list<long long> &set, int i)
    {
        // implementation with naive encoding
        // for every combination of i + 1 vars, one of them must be false
        std::vector<long long> clause_vec(i + 1, -1);
        all_subsets(set, i + 1, clause_vec);
    }

    void Encoder::encode_relaxation(int i)
    {
        // free dynamic memory in m_sorted_relax_vecs of previous iteration (i-1)
        if (!m_debug) {
            clear_sorted_relax();
        }
        long long first_relax_var = m_id_count + 1;
        //m_relax_vars.clear(); // clear from previous iteration
        std::forward_list<long long> relax_vars;
        for (int j = 0; j < m_num_objectives; ++j)
            relax_vars.push_front(first_relax_var + j);
        m_id_count += m_num_objectives; // create the remaining relaxation vars
        m_all_relax_vars.push_back(relax_vars);
        if (m_debug) {
            std::cerr << "------------ Relaxation variables of iteration " << i << " ------------\n";
            for (int j = 0; j < m_num_objectives; ++j)
                std::cerr << first_relax_var + j << '\n';
            if (i != m_num_objectives - 1)
                std::cerr << "------------ Sorted vecs after relax of iteration " << i << " ------------\n";
        }
        if (i != m_num_objectives - 1) {
            for (int j = 0; j < m_num_objectives; ++j) {
                // encode relaxation variable of the j-th objective
                std::vector<long long> *sorted_vec = m_sorted_vecs[j];
                std::vector<long long> *sorted_relax = new std::vector<long long>(sorted_vec->size(), 0);
                m_sorted_relax_vecs[j] = sorted_relax;
                if (m_debug)
                    std::cerr << "--------------- m_sorted_relax_vecs[" << j << "] ---------------\n";
                for (size_t k = 0; k < sorted_relax->size(); ++k) {
                    // create sorted_relax variables
                    sorted_relax->at(k) = m_id_count + 1;
                    m_id_count++;
                    // relax_j implies not sorted_relax_j_k
                    std::vector<long long> lits;
                    lits.push_back(-(first_relax_var + j));
                    lits.push_back(-sorted_relax->at(k));
                    add_hard_clause(lits);
                    if (m_debug) {
                        std::cerr << "sorted_relax[" << k << "]: " << sorted_relax->at(k) << "\n";
                        std::cerr << "-------------- relax_var implies not sorted_relax["<< k << "] ------\n";
                        print_clause(std::cerr, &lits);
                        std::cerr << "------- not relax_var implies sorted_relax["<< k << "] equals sorted[" << k << "] ------\n";
                    }
                    lits.clear();
                    // not relax_j implies sorted_relax_j_k equals sorted_j_k
                    lits.push_back(first_relax_var + j);
                    lits.push_back(-sorted_relax->at(k));
                    lits.push_back(sorted_vec->at(k));
                    add_hard_clause(lits);
                    if (m_debug)
                        print_clause(std::cerr, &lits);
                    lits.clear();
                    lits.push_back(first_relax_var + j);
                    lits.push_back(sorted_relax->at(k));
                    lits.push_back(-(sorted_vec->at(k)));
                    add_hard_clause(lits);
                    if (m_debug)
                        print_clause(std::cerr, &lits);
                }
            }
            // at most i constraint 
            if(m_formalism == "wcnf") {
                if (m_debug)
                    std::cerr << "---------------- At most " << i << " Constraint ----------------\n";
                at_most(relax_vars, i);
                // at least i constraint -> is not necessary
            }
        }
        else { // last iteration
            // choose exactly one obj function to minimise
            int k = 0;
            std::vector<long long> lits;
            for (long long relax_var : relax_vars) {
                std::vector<long long> *objective = m_objectives[k];
                size_t j = 0;
                for (Clause *cl : m_soft_clauses) {
                    long long soft_var = -(*(cl->begin()));
                    lits.clear();
                    if (j >= objective->size()) {
                        // relax_vars[k] implies neg soft_var[j]
                        lits.push_back(-relax_var);
                        lits.push_back(-soft_var);
                        if (m_debug) {
                            std::cerr << "relax_var implies neg soft_var: ";
                            print_clause(std::cerr, &lits);
                        }
                        add_hard_clause(lits);
                    }
                    else {
                        // relax_vars[k] implies objective[j] implies soft_var[j]
                        lits.push_back(-relax_var);
                        lits.push_back(-(objective->at(j)));
                        lits.push_back(soft_var);
                        if (m_debug) {
                            std::cerr << "relax_var implies obj_var implies soft_var: ";
                            print_clause(std::cerr, &lits);
                        }
                        add_hard_clause(lits);
                        lits.clear();
                        // let's check: m_relax_vars[k] implies soft_var[j] implies objective[j]
                        lits.push_back(-relax_var);
                        lits.push_back(objective->at(j));
                        lits.push_back(-soft_var);
                        if (m_debug) {
                            std::cerr << "relax_var implies soft_var implies obj: ";
                            print_clause(std::cerr, &lits);
                        }
                        add_hard_clause(lits);
                    }
                    ++j;
                }
                ++k;
            }
            if(m_formalism == "wcnf") { // when solving with pbo or lp, this constraint is written before calling the solver
                // at most 1 constraint
                if (m_debug)
                    std::cerr << "---------------- At most " << 1 << " Constraint ----------------\n";
                at_most(relax_vars, 1);
                // at least 1 constraint
                lits.clear();
                for (long long relax_var : relax_vars)
                    lits.push_back(relax_var);
                if (m_debug) {
                    std::cerr << "---------------- At least " << 1 << " Constraint ----------------\n";
                    print_clause(std::cerr, &lits);
                }
                add_hard_clause(lits);
            }
        }
        if (m_debug) {
            m_sorted_relax_collection.push_back(m_sorted_relax_vecs);
        }
    }

    size_t Encoder::largest_obj()
    {
        size_t largest = 0;
        for (std::vector<long long> *objective : m_objectives) {
            if (objective->size() > largest)
                largest = objective->size();        
        }
        return largest;
    }

    void Encoder::componentwise_OR(int i)
    {
        if (m_debug)
            std::cerr << "------------ Componentwise OR ------------\n";
        std::vector<std::vector<long long>*> *sorted_vecs(nullptr);
        if (i == 0) {
            // the OR is between sorted vecs
            sorted_vecs = &m_sorted_vecs;
        }
        else {
            // the OR is between sorted vecs after relaxation
            sorted_vecs = &m_sorted_relax_vecs;
        }
        size_t k = 0;
        for (Clause *cl : m_soft_clauses) {
            std::vector<long long> disjunction;
            for (int j = 0; j < m_num_objectives; ++j) {
                std::vector<long long> *sorted_vec = sorted_vecs->at(j);
                // padding with zeros to the left
                size_t largest = m_soft_clauses.size();
                if (k >= largest - sorted_vec->size()) { // no undefined behaviour because largest is the nb of vars of the largest obj.
                    // add component of sorted_vec to the disjunction
                    size_t position = k - (largest - sorted_vec->size());
                    long long component = sorted_vec->at(position);
                    disjunction.push_back(component);
                }
            }
            // disjunction implies soft variable
            long long soft_lit = *(cl->begin());
            std::vector<long long> lits;
            for (long long component : disjunction) {
                lits.push_back(-soft_lit);
                lits.push_back(-component);
                add_hard_clause(lits);
                if (m_debug)
                    print_clause(std::cerr, &lits);
                lits.clear();
            }
            ++k;        
            // soft variable implies disjunction -> It is not necessary
            /*disjunction.push_back(soft_lit);
            add_hard_clause(disjunction);*/
        }
    }

    void Encoder::generate_soft_clauses(int i)
    {
        std::vector<long long> lits;
        // if m_num_objectives is 1 then this is a single objective problem
        if (m_num_objectives == 1) {
            std::vector<long long> *objective = m_objectives[0];
            for (size_t j (0); j < objective->size(); ++j) {
                lits.push_back(-(objective->at(j)));
                add_soft_clause(lits);
                lits.clear();
            }
                return;
        }
        // find size of largest objective function
        size_t largest = largest_obj();
        long long first_soft = m_id_count + 1;
        m_id_count += largest; // create the variables of the soft clauses of this iteration
        if (m_debug) {
            std::cerr << "------------ Soft variables of iteration " << i << " ------------\n";
            for (size_t j = 0; j < largest; ++j)
                std::cerr << first_soft + j << '\n';
        }
        for (size_t j = 0; j < largest; ++j) {
            lits.push_back(-(first_soft + j));
            add_soft_clause(lits);
            lits.clear();
        }
    }

    int Encoder::solve()
    {
        std::vector<std::vector<long long>> true_ys; // for debugging
        std::vector<long long> y_vector; // for debugging
        // if there is only one objective function then it is a simple single objective problem
        if (m_num_objectives == 1) {
            generate_soft_clauses(0);
            // call solver and read output 
            int retv = external_solve(0);
            m_sat = !(m_solution.empty());
            return retv;
        }
        // encode sorted vectors with sorting network
        encode_sorted();
        // iteratively call (MaxSAT or PBO) solver       
        for (int i = 0; i < m_num_objectives; ++i) {
            clear_soft_clauses();
            generate_soft_clauses(i);
            if (m_debug)
                std::cerr << "------------------ ITERATION " << i << " ------------------\n";
            if (i != 0 && m_num_objectives != 1) // in the first iteration i == 0 there is no relaxation
                encode_relaxation(i);
            // encode the componentwise OR between sorted_relax vectors except in the last iteration
            if (i != m_num_objectives - 1)
                componentwise_OR(i);
            // call solver
            if (external_solve(i) == -1)
                return -1;
            // if unsat end computation
            if (!m_sat)
                return 0;
            // fix value of current maximum; in the end of last iteration there is no need for this
            if (i != m_num_objectives - 1) {
                for (Clause *cl : m_soft_clauses) {
                    long long soft_var = -(*(cl->begin()));
                    std::vector<long long> lits (1, m_solution[soft_var]);
                    add_hard_clause(lits);
                }
            }
            if (m_debug) {
                std::vector<long long> current_true_ys;
                for (Clause *cl : m_soft_clauses) {
                    long long soft_var = -(*(cl->begin()));
                    if (m_solution[soft_var] > 0)
                        current_true_ys.push_back(soft_var);
                }
                y_vector.push_back(current_true_ys.size());
                true_ys.push_back(current_true_ys);
            }
        }
        if (m_debug) {
            if (!m_solution.empty()){
                // compare y_vector with objective vector and print true sorted vector variables
                std::cerr << "Y vector: ";
                for (long long v : y_vector)
                    std::cerr << v << ' ';
                std::cerr << std::endl;
                std::vector<std::vector<long long>> true_input;
                for (std::vector<long long> *obj : m_objectives) {
                    std::vector<long long> true_obj_vars;
                    for (long long var : *obj) {
                        if (m_solution[var] > 0)
                            true_obj_vars.push_back(var);
                    }
                    true_input.push_back(true_obj_vars);
                }
                std::vector<long long> obj_vector (m_num_objectives, 0);
                std::cerr << "Input vector: ";
                for (const std::vector<long long> &true_obj_vars : true_input)
                    std::cerr << true_obj_vars.size() << ' ';
                std::cerr << std::endl;      
                std::cerr << "Y true variables:" << std::endl;
                int j (0);
                for (const std::vector<long long> &true_vars : true_ys) {
                    std::cerr << "Maximum " << j << ": ";
                    for (long long var : true_vars)
                        std::cerr << var << ' ';
                    std::cerr << std::endl;
                    ++j;
                }
                std::cerr << "Input true variables:" << std::endl;
                j = 0;
                for (const std::vector<long long> &true_vars : true_input) {
                    std::cerr << "Function " << j << ": ";
                    for (long long var : true_vars)
                        std::cerr << var << ' ';
                    std::cerr << std::endl;
                    ++j;
                }
                std::cerr << "Sorted vecs true variables:" << std::endl;
                j = 0;
                for (std::vector<long long> *sorted_vec : m_sorted_vecs) {
                    std::cerr << "Sorted vec " << j << ": ";
                    for (long long var : *sorted_vec) {
                        if (m_solution[var] > 0)
                            std::cerr << var << ' ';
                    }
                    std::cerr << std::endl;
                    ++j;
                }
                j = 1;
                for (std::vector<std::vector<long long>*> &sorted_relax_vecs : m_sorted_relax_collection) {
                    std::cerr << "Sorted Relax Vecs true variables of iteration " << j << ":" << std::endl;
                    int k (0);
                    for (std::vector<long long> *sorted_relax : sorted_relax_vecs) {
                        if (sorted_relax != nullptr) {
                            std::cerr << "Sorted Relax vec " << k << ": ";
                            for (long long var : *sorted_relax) {
                                if (m_solution[var] > 0)
                                    std::cerr << var << ' ';
                            }
                            std::cerr << std::endl;
                            ++k;
                        }
                    }
                    ++j;
                }
            }
        }
        return 0;
    }

}/* namespace leximaxIST */
