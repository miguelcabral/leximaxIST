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

typedef std::vector<std::forward_list<std::pair<unsigned long, unsigned long>>> SNET;

void encode_network(std::forward_list<std::forward_list<long>*> &hard_clauses, std::vector<unsigned long> &elems_to_sort, unsigned long *id_count, std::forward_list<unsigned long> *objective, SNET &sorting_network)
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
    std::vector<std::forward_list<unsigned long>*> objectives(num_objectives);
    unsigned long id_count{ hard.get_max_id() };
    std::vector<std::pair<unsigned long, unsigned long>> sorted_vecs(num_objectives);
    // convert objective function clauses to a sum of variables
    for(short i{0}; i < num_objectives; ++i){
        
    }

    // encode with odd even merge sorting network
    for(short i{0}; i < num_objectives; ++i){
        ReadCNF *objective = objectives[i];
        
        unsigned long num_terms = objective->get_clause_vector().size();
        std::pair<unsigned long, unsigned long> sorted_vec (id_count + 1, id_count + num_terms);
        sorted_vecs[i]=sorted_vec;
        id_count += num_terms;
        SNET sorting_network(num_terms);
        std::vector<unsigned long> elems_to_sort(objective->begin(),objective->end());
        encode_network(hard_clauses, elems_to_sort, &id_count, objective, sorting_network);
        ++it1;
        ++it2;
    };
    
    return 0;
}
