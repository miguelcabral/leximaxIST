#include "Leximax_encoder.h"
#include "old_packup/ReadCNF.hh"

Leximax_encoder::Leximax_encoder(int num_objectives) :
    m_id_count(0),
    m_constraints(),
    m_objectives(num_objectives, nullptr),
    m_num_objectives(num_objectives),
    m_sorted_vecs(num_objectives, nullptr)
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
        LINT num_terms = objective->size();
        m_sorted_vecs[i] = new std::vector<LINT>(num_terms, 0);
        SNET sorting_network(num_terms, nullptr); // sorting_network is initialized to a vector of nullptrs
        // elems_to_sort is represented by a pair (first element, number of elements).
        std::pair<LINT,LINT> elems_to_sort(0, num_terms);
        encode_network(elems_to_sort, objective, sorting_network);
        // sorted_vec variables are the outputs of sorting_network
        for(LINT j{0}; j < num_terms; j++){
            LINT output_j = sorting_network[j]->second;
            std::vector<LINT> *sorted_vec = m_sorted_vecs[i];
            sorted_vec->at(j) = output_j;
        }
    }
}

void Leximax_encoder::debug()
{
    // create variables equivalent to the sorted vectors to easily check if they are sorted
    
}

int Leximax_encoder::solve()
{
    /*
    // solve - iteratively call MaxSAT solver
    for(int i = 0; i < num_objectives; ++i){
        // in each iteration i there are sorted vectors after the i-th relaxation
        std::vector<std::pair<LINT, LINT>> sorted_relax_vecs(num_objectives);
        std::pair<LINT,LINT> relax_vars(id_count + 1, id_count + num_objectives); // (first var, last var)
        id_count += num_objectives;
        for(int j = 0; j < num_objectives; ++j){
            // encode relaxation variable of the j-th objective
            std::vector<LINT> *objective = objectives[i];
            std::pair<LINT, LINT> sorted_relax(id_count + 1; id_count + objective->size()); // (first var, last var)
            sorted_relax_vecs[]
            id_count += objective->size();
            for(int k = 0; k < objective->size(); ++k){
                // relax_j implies not sorted_relax_j_k
                std::vector<LINT> lits;
                lits.push_back(-(relax_vars.first + j));
                lits.push_back(-(sorted_relax.first + k));
                hard.get_clause_vector().push_back(hard.get_clauses().create_clause(lits));
                lits.clear();
                // not relax_j implies sorted_relax equals sorted
                lits.push_back();
                
            }
            
        }
        
        // soft clauses
        
        // call MaxSAT solver
    }
    */
    return 0;
}


