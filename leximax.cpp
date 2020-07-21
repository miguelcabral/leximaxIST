#include <string>
#include <iostream> // std::cout, std::cin
#include <forward_list>
#include <vector>
#include <utility> // std::pair
#include <old_packup/basic_clause.hh>
#include <old_packup/basic_clset.hh>
#include <old_packup/basic_types.h>
#include <old_packup/clause_utils.hh>
#include <old_packup/cl_registry.hh>
#include <old_packup/cl_globals.hh>
#include <old_packup/cl_types.hh>
#include <old_packup/ReadCNF.hh>

typedef std::vector<std::forward_list<std::pair<LINT, LINT>>> SNET;

void encode_fresh(ReadCNF &hard, BasicClause *cl, LINT fresh_var)
{
    // fresh_var implies cl
    vector<LINT> *lits;
    lits->push_back(-fresh_var);
    for(Literator it = cl->begin(); it != cl->end(); ++it){
        lits->push_back(*it);
    };
    hard.get_clause_vector().push_back(hard.get_clauses().create_clause(lits));
    // not fresh_var implies not cl
    for(Literator it = cl->begin(); it != cl->end(); ++it){
        lits.clear();
        lits.push_back(-*it);
        lits.push_back(fresh_var);
        hard.get_clause_vector().push_back(hard.get_clauses().create_clause(lits));s
    };    
}

void encode_network(std::forward_list<std::forward_list<long>*> &hard_clauses, std::vector<LINT> &elems_to_sort, LINT *id_count, std::forward_list<LINT> *objective, SNET &sorting_network)
{    
    if(elems_to_sort.size() == 2){
        // single comparator.
        
    }
}

int main(int argc, char *argv[])
{
    // read input problem
    short num_objectives = argc-2;
    ReadCNF hard = ReadCNF(argv[1]);
    std::vector<ReadCNF*> read_objectives;
    for(short i{2}; i < argc; ++i){
        read_objectives.push_back(ReadCNF(argv[i]));
    };
    std::vector<std::forward_list<LINT>*> objectives(num_objectives);
    LINT id_count{ hard.get_max_id() };
    std::vector<std::pair<LINT, LINT>> sorted_vecs(num_objectives);
    // convert objective function clauses to a sum of variables.
    for(short i{0}; i < num_objectives; ++i){
        ReadCNF *obj = read_objectives[i];
        vector<BasicClause*> cls = obj->get_clause_vector();
        std::forward_list<LINT> *obj_conv;
        for(LINT j{0}; j < cls.size(); ++j){
            BasicClause *cl = cls[j];
            if(cl->size() > 1 || cl->begin() < 0){
                LINT fresh_var = id_count + 1;
                id_count++;
                // encode fresh_var
                encode_fresh(hard, cl, fresh_var);
                obj_conv->push_front(fresh_var);
            }
            else
                obj_conv->push_front(cl->begin());
        }
        objectives[i] = obj_conv;        
    }

    // encode with odd even merge sorting network
    for(short i{0}; i < num_objectives; ++i){
        ReadCNF *objective = objectives[i];
        
        LINT num_terms = objective->get_clause_vector().size();
        std::pair<LINT, LINT> sorted_vec (id_count + 1, id_count + num_terms);
        sorted_vecs[i]=sorted_vec;
        id_count += num_terms;
        SNET sorting_network(num_terms);
        std::vector<LINT> elems_to_sort(objective->begin(),objective->end());
        encode_network(hard_clauses, elems_to_sort, &id_count, objective, sorting_network);
        ++it1;
        ++it2;
    };
    
    return 0;
}
