#include <Leximax_encoder.h>
#include <stdlib.h>

void Leximax_encoder::encode_sorted()
{
    if (m_num_objectives != 1) { // when there is only one objective function there is no need for this
        for (int i{0}; i < m_num_objectives; ++i) {   
            std::vector<LINT> *objective = m_objectives[i];
            size_t num_terms = objective->size();
            m_sorted_vecs[i] = new std::vector<LINT>(num_terms, 0);
            SNET sorting_network(num_terms, nullptr); // sorting_network is initialized to a vector of nullptrs
            // elems_to_sort is represented by a pair (first element, number of elements).
            std::pair<LINT,LINT> elems_to_sort(0, num_terms);
#ifdef DEBUG
            std::cerr << "--------------- Objective Function " << i << " --------------\n";
            for (size_t j = 0; j < num_terms; ++j) {
                std::cerr << objective->at(j);
                if (j != num_terms - 1)
                    std::cerr << " + ";
                else
                    std::cerr << '\n';
            }
            std::cerr << "-------------- Size of Sorting Network " << i << ": " << m_sorting_net_size << " --------------\n";
#endif
            size_t old_snet_size (m_sorting_net_size);
            m_sorting_net_size = 0;
            encode_network(elems_to_sort, objective, sorting_network);
            if (old_snet_size > m_sorting_net_size)
                m_sorting_net_size = old_snet_size; // in the end m_sorting_net_size is the size of the largest sorting network
            // sorted_vec variables are the outputs of sorting_network
            if (num_terms == 1) { // in this case the sorting network is empty
                std::vector<LINT> *sorted_vec = m_sorted_vecs[i];
                sorted_vec->at(0) = objective->at(0);
            }
            else {
                for (size_t j{0}; j < num_terms; j++) {
                    LINT output_j = sorting_network[j]->second;
                    std::vector<LINT> *sorted_vec = m_sorted_vecs[i];
                    sorted_vec->at(j) = output_j;
                }
            }
            // free memory allocated for the comparators of the sorting network
            delete_snet(sorting_network);
#ifdef DEBUG
            std::cerr << "---------------- m_sorted_vecs[" << i << "] -----------------\n";
            for(size_t j{0}; j < num_terms; j++)
                std::cerr << "sorted_vec[" << j << "]: " << m_sorted_vecs[i]->at(j) << '\n';
#endif
        }
    }
}

int list_size(std::forward_list<LINT> &mylist)
{
    int s = 0;
    for (LINT elem : mylist) {
        s++;
    }
    return s;
}

void Leximax_encoder::all_subsets(std::forward_list<LINT> set, int i, std::vector<LINT> &clause_vec)
{
    std::vector<LINT> lits;
    int size = clause_vec.size();
    // Base of recursion: 
    // when |set| == i, then set is the only subset of size i
    int set_size = list_size(set);
    if (set_size == i) {
        int j = size - i;
        for (LINT elem : set) {
            clause_vec[j] = -elem;
            j++;
        }
        // add clause to constraints
#ifdef DEBUG
        std::cerr << "Combination: ";
        print_clause(std::cerr, &clause_vec);
#endif
        lits.clear();
        for (LINT lit : clause_vec)
            lits.push_back(lit);
        add_hard_clause(lits);
    }
    // when i == 1, then each element of set is a subset of size 1
    else if (i == 1) {
        for (LINT elem : set) {
            clause_vec[size-1] = -elem;
            // add clause to constraints
#ifdef DEBUG
        std::cerr << "Combination: ";
        print_clause(std::cerr, &clause_vec);
#endif
            lits.clear();
            for (LINT lit : clause_vec)
                lits.push_back(lit);
            add_hard_clause(lits);
        }
    }
    else {
    // Step of recursion: the combinations that include the first element of set + those that don't include it
    LINT first_el = set.front();
    set.pop_front();
    clause_vec[size - i] = -first_el;
    all_subsets(set, i-1, clause_vec); // combinations that include first_el
    all_subsets(set, i, clause_vec); // combinations that don't include first_el
    }
}

void Leximax_encoder::at_most(std::forward_list<LINT> &set, int i)
{
    // implementation with naive encoding
    // for every combination of i + 1 vars, one of them must be false
    std::vector<LINT> clause_vec(i + 1, -1);
    all_subsets(set, i + 1, clause_vec);
}

void Leximax_encoder::encode_relaxation(int i)
{
    // free dynamic memory in m_sorted_relax_vecs of previous iteration (i-1)
#ifndef DEBUG
    clear_sorted_relax();
#endif
    LINT first_relax_var = m_id_count + 1;
    //m_relax_vars.clear(); // clear from previous iteration
    std::forward_list<LINT> relax_vars;
    for (int j = 0; j < m_num_objectives; ++j)
        relax_vars.push_front(first_relax_var + j);
    m_id_count += m_num_objectives; // create the remaining relaxation vars
    m_all_relax_vars.push_back(relax_vars);
#ifdef DEBUG
    std::cerr << "------------ Relaxation variables of iteration " << i << " ------------\n";
    for (int j = 0; j < m_num_objectives; ++j)
        std::cerr << first_relax_var + j << '\n';
    if (i != m_num_objectives - 1)
        std::cerr << "------------ Sorted vecs after relax of iteration " << i << " ------------\n";
#endif
    if (i != m_num_objectives - 1) {
        for (int j = 0; j < m_num_objectives; ++j) {
            // encode relaxation variable of the j-th objective
            std::vector<LINT> *sorted_vec = m_sorted_vecs[j];
            std::vector<LINT> *sorted_relax = new std::vector<LINT>(sorted_vec->size(), 0);
            m_sorted_relax_vecs[j] = sorted_relax;
#ifdef DEBUG
            std::cerr << "--------------- m_sorted_relax_vecs[" << j << "] ---------------\n";
#endif
            for (size_t k = 0; k < sorted_relax->size(); ++k) {
                // create sorted_relax variables
                sorted_relax->at(k) = m_id_count + 1;
                m_id_count++;
                // relax_j implies not sorted_relax_j_k
                std::vector<LINT> lits;
                lits.push_back(-(first_relax_var + j));
                lits.push_back(-sorted_relax->at(k));
                add_hard_clause(lits);
#ifdef DEBUG
                std::cerr << "sorted_relax[" << k << "]: " << sorted_relax->at(k) << "\n";
                std::cerr << "-------------- relax_var " <<  first_relax_var + j << " implies not sorted_relax["<< k << "] ------\n";
                print_clause(std::cerr, &lits);
                std::cerr << "------- not relax_var " <<  first_relax_var + j << " implies sorted_relax["<< k << "] equals sorted[" << k << "] ------\n";
#endif
                lits.clear();
                // not relax_j implies sorted_relax_j_k equals sorted_j_k
                lits.push_back(first_relax_var + j);
                lits.push_back(-sorted_relax->at(k));
                lits.push_back(sorted_vec->at(k));
                add_hard_clause(lits);
#ifdef DEBUG
                print_clause(std::cerr, &lits);
#endif
                lits.clear();
                lits.push_back(first_relax_var + j);
                lits.push_back(sorted_relax->at(k));
                lits.push_back(-(sorted_vec->at(k)));
                add_hard_clause(lits);
#ifdef DEBUG
                print_clause(std::cerr, &lits);
#endif
            }
        }
        // at most i constraint 
        if(m_solver_format == "wcnf") {
#ifdef DEBUG
            std::cerr << "---------------- At most " << i << " Constraint ----------------\n";
#endif
            at_most(relax_vars, i);
            // at least i constraint -> is not necessary
        }
    }
    else { // last iteration
        // choose exactly one obj function to minimise
        int k = 0;
        std::vector<LINT> lits;
        for (LINT relax_var : relax_vars) {
            std::vector<LINT> *objective = m_objectives[k];
            size_t j = 0;
            for (Clause *cl : m_soft_clauses) {
                LINT soft_var = -(*(cl->begin()));
                lits.clear();
                if (j >= objective->size()) {
                    // relax_vars[k] implies neg soft_var[j]
                    lits.push_back(-relax_var);
                    lits.push_back(-soft_var);
#ifdef DEBUG
                    std::cerr << "relax_var " << k << " implies neg soft_var " << j << ": ";
                    print_clause(std::cerr, &lits);
#endif
                    add_hard_clause(lits);
                }
                else {
                    // relax_vars[k] implies objective[j] implies soft_var[j]
                    lits.push_back(-relax_var);
                    lits.push_back(-(objective->at(j)));
                    lits.push_back(soft_var);
#ifdef DEBUG
                    std::cerr << "relax_var " << k << " implies obj " << j << " implies soft_var " << j << ": ";
                    print_clause(std::cerr, &lits);
#endif
                    add_hard_clause(lits);
                    lits.clear();
                    // let's check: m_relax_vars[k] implies soft_var[j] implies objective[j]
                    lits.push_back(-relax_var);
                    lits.push_back(objective->at(j));
                    lits.push_back(-soft_var);
#ifdef DEBUG
                    std::cerr << "relax_var " << k << " implies soft_var " << j << " implies obj " << j << ": ";
                    print_clause(std::cerr, &lits);
#endif
                    add_hard_clause(lits);
                }
                ++j;
            }
            ++k;
        }
        if(m_solver_format == "wcnf") { // when solving with pbo or lp, this constraint is written before calling the solver
            // at most 1 constraint
#ifdef DEBUG
            std::cerr << "---------------- At most " << 1 << " Constraint ----------------\n";
#endif
            at_most(relax_vars, 1);
            // at least 1 constraint
            lits.clear();
            for (LINT relax_var : relax_vars)
                lits.push_back(relax_var);
#ifdef DEBUG
            std::cerr << "---------------- At least " << 1 << " Constraint ----------------\n";
            print_clause(std::cerr, &lits);
#endif
            add_hard_clause(lits);
        }
    }
#ifdef DEBUG
    m_sorted_relax_collection.push_back(m_sorted_relax_vecs);
#endif
}

size_t Leximax_encoder::largest_obj()
{
    size_t largest = 0;
    for (std::vector<LINT> *objective : m_objectives) {
        if (objective->size() > largest)
            largest = objective->size();        
    }
    return largest;
}

void Leximax_encoder::componentwise_OR(int i)
{
#ifdef DEBUG
    std::cerr << "------------ Componentwise OR ------------\n";
#endif
    std::vector<std::vector<LINT>*> *sorted_vecs(nullptr);
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
        std::vector<LINT> disjunction;
        for (int j = 0; j < m_num_objectives; ++j) {
            std::vector<LINT> *sorted_vec = sorted_vecs->at(j);
            // padding with zeros to the left
            ULINT largest = m_soft_clauses.size();
            if (k >= largest - sorted_vec->size()) {
                // add component of sorted_vec to the disjunction
                size_t position = k - (largest - sorted_vec->size());
                LINT component = sorted_vec->at(position);
                disjunction.push_back(component);
            }
        }
        // disjunction implies soft variable
        LINT soft_lit = *(cl->begin());
        std::vector<LINT> lits;
        for (LINT component : disjunction) {
            lits.push_back(-soft_lit);
            lits.push_back(-component);
            add_hard_clause(lits);
#ifdef DEBUG
            print_clause(std::cerr, &lits);
#endif
            lits.clear();
        }
        ++k;        
        // soft variable implies disjunction -> It is not necessary
        /*disjunction.push_back(soft_lit);
        add_hard_clause(disjunction);*/
    }
}

void Leximax_encoder::generate_soft_clauses(int i)
{
    std::vector<LINT> lits;
    // if m_num_objectives is 1 then this is a single objective problem
    if (m_num_objectives == 1) {
        std::vector<LINT> *objective = m_objectives[0];
        for (size_t j (0); j < objective->size(); ++j) {
            lits.push_back(-(objective->at(j)));
            add_soft_clause(lits);
            lits.clear();
        }
            return;
    }
    // find size of largest objective function
    size_t largest = largest_obj();
    LINT first_soft = m_id_count + 1;
    m_id_count += largest; // create the variables of the soft clauses of this iteration
#ifdef DEBUG
    std::cerr << "------------ Soft variables of iteration " << i << " ------------\n";
    for (size_t j = 0; j < largest; ++j)
        std::cerr << first_soft + j << '\n';
#endif
    for (size_t j = 0; j < largest; ++j) {
        lits.push_back(-(first_soft + j));
        add_soft_clause(lits);
        lits.clear();
    }
}

int Leximax_encoder::solve()
{
#ifdef DEBUG
    std::vector<std::vector<LINT>> true_ys;
    std::vector<LINT> y_vector;
#endif
    // if there is only one objective function then it is a simple single objective problem
    if (m_num_objectives == 1) {
        generate_soft_clauses(0);
        // call solver 
        int retv = external_solve(0);
        // read model returned by the solver
        m_sat = !(m_solution.empty());
        return retv;
    }
    // encode sorted vectors with sorting network
    encode_sorted();
    // iteratively call (MaxSAT or PBO) solver       
    for (int i = 0; i < m_num_objectives; ++i) {
        clear_soft_clauses();
        generate_soft_clauses(i);
#ifdef DEBUG
        std::cerr << "------------------ ITERATION " << i << " ------------------\n";
#endif
        if (i != 0 && m_num_objectives != 1) // in the first iteration i == 0 there is no relaxation
            encode_relaxation(i);
        // encode the componentwise OR between sorted_relax vectors except in the last iteration
        if (i != m_num_objectives - 1)
            componentwise_OR(i);
        // call solver
        if (external_solve(i) == -1)
            return -1;
        // read model returned by the solver and fix value of current maximum
        if (m_solution.empty()){
            m_sat = false;
            break;
        }
        m_sat = true;
        // fix value of current maximum; in the end of last iteration there is no need for this
        if (i != m_num_objectives - 1) {
            for (Clause *cl : m_soft_clauses) {
                LINT soft_var = -(*(cl->begin()));
                std::vector<LINT> lits (1, m_solution[soft_var]);
                add_hard_clause(lits);
            }
        }
#ifdef DEBUG
        std::vector<LINT> current_true_ys;
        for (Clause *cl : m_soft_clauses) {
            LINT soft_var = -(*(cl->begin()));
            if (m_solution[soft_var] > 0)
                current_true_ys.push_back(soft_var);
        }
        y_vector.push_back(current_true_ys.size());
        true_ys.push_back(current_true_ys);
#endif      
    }
#ifdef DEBUG
    if (!m_solution.empty()){
        // compare y_vector with objective vector and print true sorted vector variables
        std::cerr << "Y vector: ";
        for (LINT v : y_vector)
            std::cerr << v << ' ';
        std::cerr << std::endl;
        std::vector<std::vector<LINT>> true_input;
        for (std::vector<LINT> *obj : m_objectives) {
            std::vector<LINT> true_obj_vars;
            for (LINT var : *obj) {
                if (m_solution[var] > 0)
                    true_obj_vars.push_back(var);
            }
            true_input.push_back(true_obj_vars);
        }
        std::vector<LINT> obj_vector (m_num_objectives, 0);
        std::cerr << "Input vector: ";
        for (const std::vector<LINT> &true_obj_vars : true_input)
            std::cerr << true_obj_vars.size() << ' ';
        std::cerr << std::endl;      
        std::cerr << "Y true variables:" << std::endl;
        int j (0);
        for (const std::vector<LINT> &true_vars : true_ys) {
            std::cerr << "Maximum " << j << ": ";
            for (LINT var : true_vars)
                std::cerr << var << ' ';
            std::cerr << std::endl;
            ++j;
        }
        std::cerr << "Input true variables:" << std::endl;
        j = 0;
        for (const std::vector<LINT> &true_vars : true_input) {
            std::cerr << "Function " << j << ": ";
            for (LINT var : true_vars)
                std::cerr << var << ' ';
            std::cerr << std::endl;
            ++j;
        }
        std::cerr << "Sorted vecs true variables:" << std::endl;
        j = 0;
        for (std::vector<LINT> *sorted_vec : m_sorted_vecs) {
            std::cerr << "Sorted vec " << j << ": ";
            for (LINT var : *sorted_vec) {
                if (m_solution[var] > 0)
                    std::cerr << var << ' ';
            }
            std::cerr << std::endl;
            ++j;
        }
        j = 1;
        for (std::vector<std::vector<LINT>*> &sorted_relax_vecs : m_sorted_relax_collection) {
            std::cerr << "Sorted Relax Vecs true variables of iteration " << j << ":" << std::endl;
            int k (0);
            for (std::vector<LINT> *sorted_relax : sorted_relax_vecs) {
                if (sorted_relax != nullptr) {
                    std::cerr << "Sorted Relax vec " << k << ": ";
                    for (LINT var : *sorted_relax) {
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
#endif
    return 0;
}
