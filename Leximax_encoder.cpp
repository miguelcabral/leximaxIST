#include "Leximax_encoder.h"
#include "old_packup/ReadCNF.hh"

Leximax_encoder::Leximax_encoder(int num_objectives) :
    m_id_count(0),
    m_constraints(),
    m_soft_clauses(),
    m_objectives(num_objectives, nullptr),
    m_num_objectives(num_objectives),
    m_sorted_vecs(num_objectives, nullptr),
    m_solver("openwbo"),
    m_pbo(false)
{
// just initialization    
}

void Leximax_encoder::encode_fresh(BasicClause *cl, LINT fresh_var)
{
    // fresh_var OR cl
    std::vector<LINT> lits;
    lits.push_back(fresh_var);
    for (auto l : *cl) lits.push_back(l);
    m_constraints.create_clause(lits);
}


void Leximax_encoder::print_clause(BasicClause *cl)
{
    for (auto l : *cl)
        std::cout << l << " ";
    std::cout << "0" << std::endl;
}

void Leximax_encoder::print_cnf()
{
    std::cout << "p cnf " << m_id_count << " " << m_constraints.size() << std::endl;
    for(BasicClause *cl : m_constraints)
        print_clause(cl);
}

int Leximax_encoder::read(char *argv[])
{
    gzFile in = gzopen(argv[1], "rb");
    if (in == Z_NULL) {
       std::cerr << "Could not open file " << argv[1] << std::endl;
       return 1;
    }
    ReadCNF hard(in);
    hard.read();
    gzclose(in);
    // copy constraints from ReadCNF hard to m_constraints
    BasicClauseSet constraints = hard.get_clauses();
    std::vector<LINT> lits;
    for (BasicClause *cl : constraints) {
        lits.clear();
        for (LINT l : *cl)
            lits.push_back(l);
        m_constraints.create_clause(lits);
    }
    std::vector<ReadCNF*> read_objectives(m_num_objectives, nullptr);
    for (int i{2}; i < m_num_objectives + 2; ++i) {
        in = gzopen(argv[i], "rb");
        if (in == Z_NULL) {
            std::cerr << "Could not open file " << argv[i] << std::endl;
            return 1;
        }
        ReadCNF *obj = new ReadCNF(in);
        obj->read();
        gzclose(in);
        read_objectives[i-2] = obj;
    }
    m_id_count = hard.get_max_id();
    // Update m_id_count if necessary - check the obj functions
    for (ReadCNF *obj : read_objectives) {
        LINT id_obj = obj->get_max_id();
        if (id_obj > m_id_count)
            m_id_count = id_obj;
    }
    // convert soft clauses to obj functions - sum of vars. Add fresh variable for each clause
    for (int i = 0; i < m_num_objectives; ++i) {
        ReadCNF *obj = read_objectives[i];
        std::vector<BasicClause*> cls = obj->get_clause_vector();
        std::vector<LINT> *obj_conv = new std::vector<LINT>(cls.size(),0);
        for (size_t j = 0; j < cls.size(); ++j) {
            BasicClause *cl = cls[j];
            LINT fresh_var = m_id_count + 1;
            m_id_count++;
            encode_fresh(cl, fresh_var);
            obj_conv->at(j) = fresh_var;
        }
        m_objectives[i] = obj_conv;
        // delete ReadCNF of i-th objective function
        delete obj;
    }
    return 0;
}

void Leximax_encoder::encode_sorted()
{
    for(int i{0}; i < m_num_objectives; ++i){   
        std::vector<LINT> *objective = m_objectives[i];
        size_t num_terms = objective->size();
        m_sorted_vecs[i] = new std::vector<LINT>(num_terms, 0);
        SNET sorting_network(num_terms, nullptr); // sorting_network is initialized to a vector of nullptrs
        // elems_to_sort is represented by a pair (first element, number of elements).
        std::pair<LINT,LINT> elems_to_sort(0, num_terms);
        encode_network(elems_to_sort, objective, sorting_network);
        // sorted_vec variables are the outputs of sorting_network
        for(size_t j{0}; j < num_terms; j++){
            LINT output_j = sorting_network[j]->second;
            std::vector<LINT> *sorted_vec = m_sorted_vecs[i];
            sorted_vec->at(j) = output_j;
        }
    }
}

void Leximax_encoder::debug()
{
    // create variables equivalent to the sorted vectors to easily check if they are sorted
    for (int i = 0; i < m_num_objectives; ++i) {
        std::vector<LINT> *sorted_vec = m_sorted_vecs[i];
        for (size_t j = 0; j < sorted_vec->size(); ++j) {
            LINT s = sorted_vec->at(j);
            LINT fresh = m_id_count + 1;
            m_id_count++;
            std::vector<LINT> lits;
            lits.push_back(fresh);
            lits.push_back(-s);
            m_constraints.create_clause(lits);
            lits.clear();
            lits.push_back(-fresh);
            lits.push_back(s);
            m_constraints.create_clause(lits);
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
    int size = clause_vec.size();
    // Base of recursion: 
    // when |set| == i, then set is the only subset of size i
    int set_size = list_size(set);
    if (set_size == i) {
        int j = size - i;
        for (LINT elem : set) {
            clause_vec[j] = elem;
            j++;
        }
        // add clause to constraints
        std::cout << "Combination: ";
        for (LINT lit : clause_vec) {
            std::cout << lit << " ";
        }
        std::cout << '\n';
        m_constraints.create_clause(clause_vec);
    }
    // when i == 1, then each element of set is a subset of size 1
    else if (i == 1) {
        for (LINT elem : set) {
            clause_vec[size-1] = elem;
            // add clause to constraints
            std::cout << "Combination: ";
            for (LINT lit : clause_vec) {
                std::cout << lit << " ";
            }
            std::cout << '\n';
            m_constraints.create_clause(clause_vec);
        }
    }
    else {
    // Step of recursion: the combinations that include the first element of set + those that don't include it
    LINT first_el = set.front();
    set.pop_front();
    clause_vec[size - i] = first_el;
    all_subsets(set, i-1, clause_vec); // combinations that include first_el
    all_subsets(set, i, clause_vec); // combinations that don't include first_el
    }
}

void Leximax_encoder::at_most(std::forward_list<LINT> &set, int i)
{
    // implementation with naive encoding
    // for every combination of i vars, one of them must be false
    std::vector<LINT> clause_vec(i, -1);
    all_subsets(set, i, clause_vec);
}

void Leximax_encoder::encode_relaxation(int i, std::vector<*std::vector<LINT>> &sorted_relax_vecs)
{
    // TODO -> sorted_relax_vecs has different representation
    LINT first_relax_var = m_id_count + 1;
    m_id_count += m_num_objectives; // create the remaining relaxation vars
    for (int j = 0; j < m_num_objectives; ++j) {
        // encode relaxation variable of the j-th objective
        std::vector<LINT> *sorted_relax = sorted_relax_vecs[j];
        std::vector<LINT> *sorted_vec = m_sorted_vecs[j];
        for (size_t k = 0; k < sorted_relax->size(); ++k) {
            // create sorted_relax variables
            sorted_relax->at(k) = m_id_count + 1;
            m_id_count++;
            // relax_j implies not sorted_relax_j_k
            std::vector<LINT> lits;
            lits.push_back(-(first_relax_var + j));
            lits.push_back(-sorted_relax->at(k));
            m_constraints.create_clause(lits);
            lits.clear();
            // not relax_j implies sorted_relax_j_k equals sorted_j_k
            lits.push_back(first_relax_var + j);
            lits.push_back(-sorted_relax->at(k));
            lits.push_back(sorted_vec->at(k));
            m_constraints.create_clause(lits);
            lits.clear();
            lits.push_back(first_relax_var + j);
            lits.push_back(sorted_relax->at(k));
            lits.push_back(-(sorted_vec->at(k)));
            m_constraints.create_clause(lits);
        }
    }
    // cardinality constraint TODO
    // at most i constraint 
    std::forward_list<LINT> relax_vars;
    for (int j = 0; j < m_num_objectives; ++j)
        relax_vars.push_front(first_relax_var + j);
    at_most(relax_vars, i);
    // at least i constraint -> should I put this one?
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

void Leximax_encoder::componentwise_OR(std::vector<*std::vector<LINT>> &sorted_vecs)
{
    for (size_t i = 0; i < largest; ++i) {
        std::vector<LINT> disjunction;
        for (int j = 0; j < m_num_objectives; ++j) {
            std::vector<LINT> *sorted_vec = sorted_vecs[j];
            // padding with zeros to the left
            if (j >= largest - sorted_vec->size()) {
                // add component of sorted_vec to the disjunction
                size_t position = j - (largest - sorted_vec->size());
                LINT component = sorted_vec->at(position);
                disjunction.push_back(component);
            }
        }
        // disjunction implies soft variable
    }
}

int Leximax_encoder::solve()
{
    // setup for sorted_relax_vecs
    std::vector<*std::vector<LINT>> sorted_relax_vecs(m_num_objectives, nullptr);
    for (int i = 0; i < m_num_objectives; ++i) {
        std::vector<LINT> *sorted_vec = m_sorted_vecs[i];
        sorted_relax_vecs[i] = new std::vector<LINT>(sorted_vec->size(), 0);
    }
    // iteratively call (MaxSAT or PBO) solver
    for (int i = 0; i < m_num_objectives; ++i) {
        if (i == m_num_objectives) {
            // last iteration is done differently
        }
        else {
            if (i != 0) { // in the first iteration i == 0 there is no relaxation
                encode_relaxation(i, sorted_relax_vecs); // create the vars in sorted_relax_vecs and encode the relax vars
            }
            
            // soft clauses
            // find size of largest objective function
            /*
            size_t largest = largest_obj();
            LINT first_soft = m_id_count + 1;
            m_id_count += largest; // create the variables of the soft clauses of this iteration
            // encode the componentwise OR between sorted_relax vectors
            if (i == 0) {
                // the OR is between sorted vecs
                componentwise_OR(i);
            }
            else {
                // the OR is between sorted vecs after relaxation
                componentwise_OR(i);
            }
            */
            // call solver
            
            if (m_pbo)
                solve_pbo();
            else
                solve_maxsat();
            
            // read output file and fix values of current objective function
        }
    }
    
    // print solution
    return 0;
}

int Leximax_encoder::solve_maxsat()
{
    // write input file of the solver
    
    // call solver and write its output to a file
   
    return 0;
}

int Leximax_encoder::solve_pbo()
{
    // write input file of the solver
    
    // call solver and write its output to a file
    
    return 0;
}

