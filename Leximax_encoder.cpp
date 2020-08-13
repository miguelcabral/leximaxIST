#include "Leximax_encoder.h"
#include "old_packup/ReadCNF.hh"

Leximax_encoder::Leximax_encoder(int num_objectives) :
    m_id_count(0),
    m_constraints(),
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

void Leximax_encoder::encode_relaxation(int i, std::vector<LINT> &sorted_relax_vecs)
{
    LINT first_relax_var = m_id_count + 1; // We only store the first relaxation variable
    m_id_count += m_num_objectives; // create the remaining relaxation vars
    for (int j = 0; j < m_num_objectives; ++j) {
        // encode relaxation variable of the j-th objective
        std::vector<LINT> *objective = m_objectives[j];
        LINT first_sorted_relax(m_id_count + 1); // first component
        sorted_relax_vecs[j] = first_sorted_relax;
        m_id_count += objective->size(); // create remaining sorted_relax_vec components
        std::vector<LINT> *sorted_vec = m_sorted_vecs[j];
        for (size_t k = 0; k < objective->size(); ++k) {
            // relax_j implies not sorted_relax_j_k
            std::vector<LINT> lits;
            lits.push_back(-(first_relax_var + j));
            lits.push_back(-(first_sorted_relax + k));
            m_constraints.create_clause(lits);
            lits.clear();
            // not relax_j implies sorted_relax_j_k equals sorted_j_k
            lits.push_back(first_relax_var + j);
            lits.push_back(-(first_sorted_relax + k));
            lits.push_back(sorted_vec->at(k));
            m_constraints.create_clause(lits);
            lits.clear();
            lits.push_back(first_relax_var + j);
            lits.push_back(first_sorted_relax + k);
            lits.push_back(-(sorted_vec->at(k)));
            m_constraints.create_clause(lits);
            lits.clear();
        }
    }
    // cardinality constraint TODO
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

int Leximax_encoder::solve()
{
    // iteratively call (MaxSAT or PBO) solver
    for (int i = 0; i < m_num_objectives; ++i) {
        // in each iteration i there are sorted vectors after the i-th relaxation
        std::vector<LINT> sorted_relax_vecs(m_num_objectives, 0); // In each sorted_relax vector we only store the first component
        encode_relaxation(i, sorted_relax_vecs);
        // soft clauses
        // find size of largest objective function
        size_t largest = largest_obj();
        LINT first_soft = m_id_count + 1;
        m_id_count += largest; // create the variables of the soft clauses of this iteration
        // encode the componentwise OR between sorted_relax vectors
        
        // call solver
        
        if (m_pbo)
            solve_pbo();
        else
            solve_maxsat();
        
        // read output file and fix values of current objective function
    }
    
    // print solution
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

