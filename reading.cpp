#include "Leximax_encoder.h"
#include "ReadCNF.hh"

void Leximax_encoder::encode_fresh(BasicClause *cl, LINT fresh_var)
{
    // fresh_var OR cl
    std::vector<LINT> lits;
    lits.push_back(fresh_var);
    for (auto l : *cl) lits.push_back(l);
    m_constraints.create_clause(lits);
}

int Leximax_encoder::read()
{
    gzFile in = gzopen(m_input_files[0].c_str(), "rb");
    if (in == Z_NULL) {
       std::cerr << "Could not open file " << m_input_files[0] << std::endl;
       return 1;
    }
    ReadCNF hard(in);
    hard.read();
    gzclose(in);
    // copy constraints from ReadCNF hard to m_constraints
    BasicClauseSet &constraints = hard.get_clauses();
    std::vector<LINT> lits;
    for (BasicClause *cl : constraints) {
        lits.clear();
        for (LINT l : *cl)
            lits.push_back(l);
        m_constraints.create_clause(lits);
    }
    std::vector<ReadCNF*> read_objectives(m_num_objectives, nullptr);
    for (int i{1}; i < m_num_objectives + 1; ++i) {
        in = gzopen(m_input_files[i].c_str(), "rb");
        if (in == Z_NULL) {
            std::cerr << "Could not open file " << m_input_files[i] << std::endl;
            return 1;
        }
        ReadCNF *obj = new ReadCNF(in);
        obj->read();
        gzclose(in);
        read_objectives[i-1] = obj;
    }
    m_id_count = hard.get_max_id();
    // Update m_id_count if necessary - check the obj functions
    for (ReadCNF *obj : read_objectives) {
        size_t id_obj = obj->get_max_id();
        if (id_obj > m_id_count)
            m_id_count = id_obj;
    }
    // convert soft clauses to obj functions - sum of vars. Add fresh variable for each clause
    for (int i = 0; i < m_num_objectives; ++i) {
        ReadCNF *obj = read_objectives[i];
        std::vector<BasicClause*> &cls = obj->get_clause_vector();
        std::vector<LINT> *obj_conv = new std::vector<LINT>(cls.size(), 0);
        for (size_t j = 0; j < cls.size(); ++j) {
            BasicClause *cl = cls.at(j);
            LINT fresh_var = m_id_count + 1;
            m_id_count++;
            encode_fresh(cl, fresh_var);
            obj_conv->at(j) = fresh_var;
        }
        m_objectives[i] = obj_conv;
        // delete ReadCNF of i-th objective function
        delete obj;
    }
    // verification -------------------------
    // write input constraints + objective function variables to pienum input
    ofstream pienum_file;
    pienum_file.open(m_pienum_file_name.c_str());
    print_cnf(pienum_file);
    pienum_file.close();
    // verification -------------------------
    return 0;
}
