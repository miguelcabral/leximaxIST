#include "Leximax_encoder.h"
#include "old_packup/ReadCNF.hh"

void Leximax_encoder::encode_fresh(BasicClause *cl, LINT fresh_var)
{
    // fresh_var OR cl
    std::vector<LINT> lits;
    lits.push_back(fresh_var);
    for (auto l : *cl) lits.push_back(l);
    m_constraints.create_clause(lits);
}

void Leximax_encoder::set_input_name(char *argv[])
{
    m_input_files = argv[1];
    size_t position = m_input_files.find_last_of("/\\");
    m_input_files = m_input_files.substr(position + 1);
    for (int i{2}; i < m_num_objectives + 2; ++i) {
        m_input_files.append("_");
        std::string next_file = argv[i];
        position = next_file.find_last_of("/\\");
        next_file = next_file.substr(position + 1);
        m_input_files.append(next_file);
    }
}

int Leximax_encoder::read(char *argv[])
{
    set_input_name(argv);
    gzFile in = gzopen(argv[1], "rb");
    if (in == Z_NULL) {
       std::cerr << "Could not open file " << argv[1] << std::endl;
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
    return 0;
}
