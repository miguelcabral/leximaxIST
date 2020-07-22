#include <string>
#include <iostream> // std::cout, std::cin
#include <forward_list>
#include <vector>
#include <utility> // std::pair
#include "old_packup/basic_clause.hh"
#include "old_packup/basic_clset.hh"
#include "old_packup/basic_types.h"
#include "old_packup/clause_utils.hh"
#include "old_packup/cl_registry.hh"
#include "old_packup/cl_globals.hh"
#include "old_packup/cl_types.hh"
#include "old_packup/ReadCNF.hh"

/* a sorting network is a vector with size equal to the number of wires.
 * Entry i contains the last comparator (in the construction of the sorting network) that connects to wire i.
 * The comparator is represented as a pair. The first component is the other wire j that the comparator connects to.
 * The second component is the variable representing
 * the smallest of the outputs, if i < j
 * the greatest of the outputs, if i > j.*/
typedef std::vector<std::pair<LINT, LINT>*> SNET;

void encode_max(ReadCNF &hard, LINT var_out_max, LINT var_in1, LINT var_in2)
{
    // encode var_out_max is equivalent to var_in1 OR var_in2
    std::vector<LINT> lits;
    lits.push_back(-var_out_max);
    lits.push_back(var_in1);
    lits.push_back(var_in2);
    hard.get_clause_vector().push_back(hard.get_clauses().create_clause(lits));
    lits.clear();
    lits.push_back(var_out_max);
    lits.push_back(var_in1);
    hard.get_clause_vector().push_back(hard.get_clauses().create_clause(lits));
    lits.clear();
    lits.push_back(var_out_max);
    lits.push_back(var_in2);
    hard.get_clause_vector().push_back(hard.get_clauses().create_clause(lits));
}

void encode_min(ReadCNF &hard, LINT var_out_min, LINT var_in1, LINT var_in2)
{
    // encode var_out_min is equivalent to var_in1 AND var_in2
    std::vector<LINT> lits;
    lits.clear();
    lits.push_back(-var_out_min);
    lits.push_back(var_in1);
    hard.get_clause_vector().push_back(hard.get_clauses().create_clause(lits));
    lits.clear();
    lits.push_back(-var_out_min);
    lits.push_back(var_in2);
    hard.get_clause_vector().push_back(hard.get_clauses().create_clause(lits));
    lits.clear();
    lits.push_back(-var_out_min);
    lits.push_back(var_in1);
    hard.get_clause_vector().push_back(hard.get_clauses().create_clause(lits));
}

void encode_fresh(ReadCNF &hard, BasicClause *cl, LINT fresh_var)
{
    // fresh_var implies cl
    vector<LINT> *lits;
    lits->push_back(-fresh_var);
    for(Literator it = cl->begin(); it != cl->end(); ++it){
        lits->push_back(*it);
    };
    hard.get_clause_vector().push_back(hard.get_clauses().create_clause(*lits));
    // not fresh_var implies not cl
    for(Literator it = cl->begin(); it != cl->end(); ++it){
        lits->clear();
        lits->push_back(-*it);
        lits->push_back(fresh_var);
        hard.get_clause_vector().push_back(hard.get_clauses().create_clause(*lits));
    };    
}

void encode_network(ReadCNF &hard, std::vector<LINT> &elems_to_sort, LINT &id_count, std::vector<LINT> *objective, SNET &sorting_network)
{    
    if(elems_to_sort.size() == 2){
        // single comparator.
        LINT el1 = elems_to_sort[0];
        LINT el2 = elems_to_sort[1];
        // if the entry is empty, then it is the first comparator for that wire
        LINT var_in1 = (sorting_network[el1] == nullptr) ? objective->at(el1) : sorting_network[el1].second;
        LINT var_in2 = (sorting_network[el2] == nullptr) ? objective->at(el2) : sorting_network[el2].second;
        LINT var_out_min = id_count + 1;
        id_count++;
        LINT var_out_max = id_count + 1;
        id_count++;
        // encode outputs, if el1 > el2 then el1 is the largest, that is, the or. Otherwise, el1 is the smallest, i.e. the and.
        encode_max(hard, var_out_max, var_in1, var_in2);
        encode_min(hard, var_out_min, var_in1, var_in2);
        std::pair<LINT,LINT> comp1 = (el1 > el2) ? &(std::make_pair(el2,var_out_max)) : &(std::make_pair(el2,var_out_min));
        sorting_network[el1] = &comp1;
        std::pair<LINT,LINT> comp2 = (el2 > el1) ? &(std::make_pair(el1,var_out_max)) : &(std::make_pair(el1,var_out_min));
        sorting_network[el2] = &comp2;
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
    std::vector<std::vector<LINT>*> objectives(num_objectives);
    LINT id_count{ hard.get_max_id() };
    std::vector<std::pair<LINT, LINT>> sorted_vecs(num_objectives);
    // convert objective function clauses to a sum of variables.
    for(short i{0}; i < num_objectives; ++i){
        ReadCNF *obj = read_objectives[i];
        vector<BasicClause*> cls = obj->get_clause_vector();
        std::vector<LINT> *obj_conv;
        for(LINT j{0}; j < cls.size(); ++j){
            BasicClause *cl = cls[j];
            if(cl->size() > 1 || cl->begin() < 0){
                LINT fresh_var = id_count + 1;
                id_count++;
                // encode fresh_var
                encode_fresh(hard, cl, fresh_var);
                obj_conv->push_back(fresh_var);
            }
            else
                obj_conv->push_back(cl->begin());
        }
        objectives[i] = obj_conv;        
    }

    // encode with odd even merge sorting network
    for(short i{0}; i < num_objectives; ++i){        
        LINT num_terms = objectives[i]->size();
        std::pair<LINT, LINT> sorted_vec (id_count + 1, id_count + num_terms);
        sorted_vecs[i]=sorted_vec;
        id_count += num_terms;
        SNET sorting_network(num_terms); // sorting_network is initialized to a vector of nullptrs
        std::vector<LINT> elems_to_sort;
        for(LINT j{0}; j < num_terms; ++j){
            elems_to_sort.push_back(j);
        };
        encode_network(hard, elems_to_sort, id_count, objective, sorting_network);
        // relate outputs of sorting_network with sorted_vec variables
        for(LINT j{0}; j < num_terms; j++){
            LINT output_j = sorting_network[j]->front().second;
            LINT o = sorted_vec.first + j;
            // encode o is equivalent to output_j
            std::vector<LINT> lits;
            lits.push_back(-o);
            lits.push_back(output_j);
            hard.get_clause_vector().push_back(hard.get_clauses().create_clause(lits));
            lits.clear();
            lits.push_back(o);
            lits.push_back(-output_j);
            hard.get_clause_vector().push_back(hard.get_clauses().create_clause(lits));
        };
        sorting_network.clear();
    };

    return 0;
}
