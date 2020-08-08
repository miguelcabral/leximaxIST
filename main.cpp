#include <string>
#include <iostream> // std::cout, std::cin
#include <vector> // std::vector
#include <utility> // std::pair
#include "old_packup/basic_clause.hh"
#include "old_packup/basic_clset.hh"
#include "old_packup/basic_types.h"
#include "old_packup/clause_utils.hh"
#include "old_packup/cl_registry.hh"
#include "old_packup/cl_globals.hh"
#include "old_packup/cl_types.hh"
#include "Leximax_encoder.h"

int main(int argc, char *argv[])
{
    
    int num_objectives = argc-2;
    Leximax_encoder enc(num_objectives);
    // read input problem: hard.cnf f_1.cnf f_2.cnf ...
    enc.read(argv);
    // encode sorted vectors with sorting network
    enc.encode_sorted();/*
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
    // check if sorting_network is working: print clauses and send to sat solver.
    enc.print_cnf();
    return 0;
}
