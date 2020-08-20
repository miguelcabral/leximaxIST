#include "Leximax_encoder.h"
#include <stdlib.h>
#include "old_packup/types.hh"

void write_clauses(ostream &output, BasicClauseSet &clauses, size_t weight);

void Leximax_encoder::encode_sorted()
{
    for (int i{0}; i < m_num_objectives; ++i) {   
        std::vector<LINT> *objective = m_objectives[i];
        size_t num_terms = objective->size();
        m_sorted_vecs[i] = new std::vector<LINT>(num_terms, 0);
        SNET sorting_network(num_terms, nullptr); // sorting_network is initialized to a vector of nullptrs
        // elems_to_sort is represented by a pair (first element, number of elements).
        std::pair<LINT,LINT> elems_to_sort(0, num_terms);
        if (m_debug) {
            std::cerr << "--------------- Objective Function " << i << " --------------\n";
            for (size_t j = 0; j < num_terms; ++j) {
                std::cerr << objective->at(j);
                if (j != num_terms - 1)
                    std::cerr << " + ";
                else
                    std::cerr << '\n';
            }
            std::cerr << "---------------- Sorting Network " << i << " ----------------\n";
        }
        encode_network(elems_to_sort, objective, sorting_network);
        // sorted_vec variables are the outputs of sorting_network
        for (size_t j{0}; j < num_terms; j++) {
            LINT output_j = sorting_network[j]->second;
            std::vector<LINT> *sorted_vec = m_sorted_vecs[i];
            sorted_vec->at(j) = output_j;
        }
        if (m_debug) {
            std::cerr << "---------------- m_sorted_vecs[" << i << "] -----------------\n";
            for(size_t j{0}; j < num_terms; j++) {
                std::cerr << "sorted_vec[" << j << "]: " << m_sorted_vecs[i]->at(j) << '\n';
            }
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
        if (m_debug) {
            std::cerr << "Combination: ";
            for (LINT lit : clause_vec) {
                std::cerr << lit << " ";
            }
            std::cerr << '\n';
        }
        lits.clear();
        for (LINT lit : clause_vec)
            lits.push_back(lit);
        m_constraints.create_clause(lits);
    }
    // when i == 1, then each element of set is a subset of size 1
    else if (i == 1) {
        for (LINT elem : set) {
            clause_vec[size-1] = -elem;
            // add clause to constraints
            if (m_debug) {
                std::cerr << "Combination: ";
                for (LINT lit : clause_vec) {
                    std::cerr << lit << " ";
                }
                std::cerr << '\n';
            }
            lits.clear();
            for (LINT lit : clause_vec)
                lits.push_back(lit);
            m_constraints.create_clause(lits);
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
    LINT first_relax_var = m_id_count + 1;
    m_relax_vars.clear(); // clear from previous iteration
    for (int j = 0; j < m_num_objectives; ++j)
        m_relax_vars.push_front(first_relax_var + j);
    m_id_count += m_num_objectives; // create the remaining relaxation vars
    if (m_debug) {
        std::cerr << "------------ Relaxation variables of iteration " << i << " ------------\n";
        for (int j = 0; j < m_num_objectives; ++j)
            std::cerr << first_relax_var + j << '\n';
        std::cerr << "------------ Sorted vecs after relax of iteration " << i << " ------------\n";
    }
    for (int j = 0; j < m_num_objectives; ++j) {
        // encode relaxation variable of the j-th objective
        std::vector<LINT> *sorted_vec = m_sorted_vecs[j];
        std::vector<LINT> *sorted_relax = new std::vector<LINT>(sorted_vec->size(), 0);
        m_sorted_relax_vecs[j] = sorted_relax;
        if (m_debug)
            std::cerr << "--------------- m_sorted_relax_vecs[" << j << "] ---------------\n";
        for (size_t k = 0; k < sorted_relax->size(); ++k) {
            // create sorted_relax variables
            sorted_relax->at(k) = m_id_count + 1;
            m_id_count++;
            if (m_debug)
                std::cerr << "sorted_relax[" << k << "]: " << sorted_relax->at(k) << "\n";
            // relax_j implies not sorted_relax_j_k
            std::vector<LINT> lits;
            lits.push_back(-(first_relax_var + j));
            lits.push_back(-sorted_relax->at(k));
            m_constraints.create_clause(lits);
            if (m_debug) {
                std::cerr << "-------------- relax_var " <<  first_relax_var + j << " implies not sorted_relax["<< k << "] ------\n";
                for (LINT l : lits)
                    std::cerr << l << " ";
                std::cerr << "0\n";
                std::cerr << "------- not relax_var " <<  first_relax_var + j << " implies sorted_relax["<< k << "] equals sorted[" << k << "] ------\n";
            }
            lits.clear();
            // not relax_j implies sorted_relax_j_k equals sorted_j_k
            lits.push_back(first_relax_var + j);
            lits.push_back(-sorted_relax->at(k));
            lits.push_back(sorted_vec->at(k));
            m_constraints.create_clause(lits);
            if (m_debug) {
                for (LINT l : lits)
                    std::cerr << l << " ";
                std::cerr << "0\n";
            }
            lits.clear();
            lits.push_back(first_relax_var + j);
            lits.push_back(sorted_relax->at(k));
            lits.push_back(-(sorted_vec->at(k)));
            m_constraints.create_clause(lits);
            if (m_debug) {
                for (LINT l : lits)
                    std::cerr << l << " ";
                std::cerr << "0\n";
            }
        }
    }
    // cardinality constraint
    // at most i constraint 
    if (m_debug)
        std::cerr << "---------------- At most " << i << " Constraint ----------------\n";
    if (!m_pbo)
        at_most(m_relax_vars, i);
    
    // at least i constraint -> should I put this one? -> experiment later to check if program runs faster
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
    if (m_debug)
        std::cerr << "------------ Componentwise OR ------------\n";
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
    for (BasicClause *cl : m_soft_clauses) {
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
            m_constraints.create_clause(lits);
            if (m_debug) {
                for (LINT l : lits)
                    std::cerr << l << " ";
                std::cerr << "0\n";
            }
            lits.clear();
        }
        ++k;        
        // soft variable implies disjunction -> Should I put this one? -> experiment later on
    }
}

void Leximax_encoder::solve()
{
    // iteratively call (MaxSAT or PBO) solver
    IntVector tmp_model(1, 0);
    for (int i = 0; i < m_num_objectives; ++i) {
        m_soft_clauses.clear();
        if (m_debug)
            std::cerr << "------------------ ITERATION " << i << " ------------------\n";
        if (i == m_num_objectives - 1) {
            // last iteration is done differently
            LINT first_relax = m_id_count + 1;
            LINT first_soft
        }
        else {
            if (i != 0) // in the first iteration i == 0 there is no relaxation
                encode_relaxation(i); // create the vars in sorted_relax_vecs and encode the relax vars
            // soft clauses
            // find size of largest objective function
            size_t largest = largest_obj();
            LINT first_soft = m_id_count + 1;
            m_id_count += largest; // create the variables of the soft clauses of this iteration
            if (m_debug) {
                std::cerr << "------------ Soft variables of iteration " << i << " ------------\n";
                for (size_t j = 0; j < largest; ++j)
                    std::cerr << first_soft + j << '\n';
            }
            std::vector<LINT> lits;
            for (size_t j = 0; j < largest; ++j) {
                lits.push_back(-(first_soft + j));
                m_soft_clauses.create_clause(lits);
                lits.clear();
            }
            // encode the componentwise OR between sorted_relax vectors
            componentwise_OR(i);
            // call solver 
            if (m_pbo)
                solve_pbo(i, tmp_model); // tmp_model[k] is k if k is true under tmp_model, and -k otherwise.
            else
                solve_maxsat(i, tmp_model);
            /*
            if (retv != 0) {
                open_wbo returns 7680 , wtf???
                std::cerr << "Something went wrong with the solver\n";
                exit(retv);
            }*/
            // read tmp_model and fix values of current objective function
            if (tmp_model.empty()){
                m_sat = false;
                break;
            }
            m_sat = true;
            for (BasicClause *cl : m_soft_clauses) {
                LINT soft_var = -(*(cl->begin()));
                LINT lit = tmp_model[soft_var];
                m_constraints.create_unit_clause(lit);
            }
        }
        // print i-th maximum
        print_optimum(tmp_model);
    }
    // print solution
    print_solution(tmp_model);
}
