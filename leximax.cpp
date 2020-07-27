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
/* using std::make_pair; */

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

void insert_comparator(ReadCNF &hard, LINT el1, LINT el2, LINT &id_count, std::vector<LINT> *objective, SNET &sorting_network)
{
    if(el1 == 0 || el2 == 0)
        std::cout << "Inserting comparator in entry 0" << std::endl;
    // if the entry is empty, then it is the first comparator for that wire
    LINT var_in1 = (sorting_network[el1] == nullptr) ? objective->at(el1) : sorting_network[el1]->second;
    LINT var_in2 = (sorting_network[el2] == nullptr) ? objective->at(el2) : sorting_network[el2]->second;
    LINT var_out_min = id_count + 1;
    id_count++;
    LINT var_out_max = id_count + 1;
    id_count++;
    // encode outputs, if el1 > el2 then el1 is the largest, that is, the or. Otherwise, el1 is the smallest, i.e. the and.
    encode_max(hard, var_out_max, var_in1, var_in2);
    encode_min(hard, var_out_min, var_in1, var_in2);
    std::pair<LINT,LINT> *comp1;
    std::pair<LINT,LINT> *comp2;
    if(el1 > el2){
        comp1 = new std::pair<LINT,LINT>(el2,var_out_max);
    }
    else
        comp1 = new std::pair<LINT,LINT>(el2,var_out_min);
    sorting_network[el1] = comp1;
    if(el2 > el1){
        comp2 = new std::pair<LINT,LINT>(el1,var_out_max);
    }
    else
        comp2 = new std::pair<LINT,LINT>(el1,var_out_min);;
    sorting_network[el2] = comp2;
}

void encode_fresh(ReadCNF &hard, BasicClause *cl, LINT fresh_var)
{
    // fresh_var implies cl
    std::vector<LINT> lits;
    lits.push_back(-fresh_var);
    for(Literator it = cl->begin(); it != cl->end(); ++it){
        lits.push_back(*it);
    };
    hard.get_clause_vector().push_back(hard.get_clauses().create_clause(lits));
    // not fresh_var implies not cl
    for(Literator it = cl->begin(); it != cl->end(); ++it){
        lits.clear();
        lits.push_back(-*it);
        lits.push_back(fresh_var);
        hard.get_clause_vector().push_back(hard.get_clauses().create_clause(lits));
    }  
}

void odd_even_merge(ReadCNF &hard, std::pair<std::pair<LINT,LINT>,LINT> seq1, std::pair<std::pair<LINT,LINT>,LINT> seq2, LINT &id_count, std::vector<LINT> *objective, SNET &sorting_network)
{
    LINT el1;
    LINT el2;
    LINT var_in1;
    LINT var_in2;
    LINT var_out_min;
    LINT var_out_max;
    LINT size1 = seq1.first.second;
    LINT size2 = seq2.first.second;
    if(size1 == 0 || size2 == 0){
        // nothing to merge
    }
    else if(size1 == 1 && size2 == 1){
        // merge two elements with a single comparator.
        el1 = seq1.first.first;
        el2 = seq2.first.first;
        insert_comparator(hard, el1, el2, id_count, objective, sorting_network);
    }
    else {
        // merge odd subsequences
        // size of odd subsequence is the ceiling of half of the size of the original sequence
        LINT new_size = size1/2;
        if(size1 % 2 != 1)
            new_size++;
        std::pair<LINT,LINT> p1(seq1.first.first,new_size);
        LINT offset = 2*seq1.second;
        std::pair<std::pair<LINT,LINT>,LINT> odd1(p1,offset);
        new_size = size2/2;
        if(size2 % 2 != 1)
            new_size++;
        std::pair<LINT,LINT> p2(seq2.first.first,new_size);
        offset = 2*seq2.second;
        std::pair<std::pair<LINT,LINT>,LINT> odd2(p2,offset);
        odd_even_merge(hard, odd1, odd2, id_count, objective, sorting_network);
        // merge even subsequences
        // size of even subsequence is the floor of half of the size of the original sequence
        new_size = size1/2;
        offset = seq1.second;
        p1 = std::make_pair(seq1.first.first + offset, new_size);
        offset = 2*offset;
        std::pair<std::pair<LINT,LINT>,LINT> even1(p1,offset);
        new_size = size2/2;
        offset = seq2.second;
        p2 = std::make_pair(seq2.first.first + offset, new_size);
        offset = 2*offset;
        std::pair<std::pair<LINT,LINT>,LINT> even2(p2, offset);
        odd_even_merge(hard, even1, even2, id_count, objective, sorting_network);
        // comparison-interchange - suppose seq1 = a1 a2 a3. and seq2 = b1 b2 b3 b4. Then a1 a2-a3 b1-b2 b3-b4.
        LINT offset1 = seq1.second;
        LINT offset2 = seq2.second;
        LINT first1 = seq1.first.first;
        LINT first2 = seq2.first.first;
        for(LINT i{2}; i <= size1; i = i + 2){
            if(i == size1){
                // connect last of seq1 to first element of seq2
                el1 = first1 + offset1*(size1-1);
                el2 = first2;
                insert_comparator(hard, el1, el2, id_count, objective, sorting_network);
            }
            else{
                // connect i-th of seq1 with i+1-th of seq1
                el1 = first1 + offset1*(i-1);
                el2 = el1 + offset1;
                insert_comparator(hard, el1, el2, id_count, objective, sorting_network);
            }
        }
        LINT init = (size1 % 2 == 0) ? 2 : 1 ;
        for(LINT i{init}; i < size2; i = i + 2){
                // connect i-th of seq2 with i+1-th of seq2
                el1 = first2 + offset2*(i-1);
                el2 = el1 + offset2;
                insert_comparator(hard, el1, el2, id_count, objective, sorting_network);            
        }
    }
}

void encode_network(ReadCNF &hard, std::pair<LINT,LINT> elems_to_sort, LINT &id_count, std::vector<LINT> *objective, SNET &sorting_network)
{   
    LINT size = elems_to_sort.second;
    LINT first_elem = elems_to_sort.first;
    if(size == 1){
        // do nothing - a single element is already sorted.
    }
    if(size > 1){
        LINT m = size/2;
        LINT n = m;
        /* sera? const LINT n = size - m; */
        if(size % 2 != 0)
            n++;
        std::pair<LINT,LINT> split1(first_elem, m);
        std::pair<LINT,LINT> split2(first_elem + m, n);
        // recursively sort the first m elements and the last n elements
        encode_network(hard, split1, id_count, objective, sorting_network);
        encode_network(hard, split2, id_count, objective, sorting_network);
        // merge the sorted m elements and the sorted n elements
        std::pair<std::pair<LINT,LINT>,LINT> seq1(split1,1);
        std::pair<std::pair<LINT,LINT>,LINT> seq2(split2,1);
        odd_even_merge(hard, seq1, seq2, id_count, objective, sorting_network);
    }
}

void print_clause(BasicClause *cl)
{
    for(Literator it = cl->begin(); it != cl->end(); ++it){
        std::cout << *it << " ";
    };
    std::cout << "0" << std::endl;
}

void print_cnf(ReadCNF &hard, LINT id_count)
{
    std::vector<BasicClause*> cls = hard.get_clause_vector();
    std::cout << "p cnf " << id_count << " " << cls.size() << std::endl;
    for(LINT i{0}; i < cls.size(); ++i){
        print_clause(cls.at(i));
    }
}

int main(int argc, char *argv[])
{
    // read input problem: hard.cnf f_1.cnf f_2.cnf ...
    short num_objectives = argc-2;
    gzFile in = gzopen(argv[1], "rb");
    if (in == Z_NULL) {
       exit(0);
    }
    ReadCNF hard(in);
    hard.read();
    gzclose(in);
    std::vector<ReadCNF*> read_objectives;
    for(short i{2}; i < argc; ++i){
        in = gzopen(argv[i], "rb");
        if (in == Z_NULL) {
            exit(0);
        }
        ReadCNF *obj = new ReadCNF(in);
        obj->read();
        gzclose(in);
        read_objectives.push_back(obj);
    };
    std::vector<std::vector<LINT>*> objectives(num_objectives);
    LINT id_count{ hard.get_max_id() };
    // convert objective function clauses to a sum of variables.
    for(short i{0}; i < num_objectives; ++i){
        ReadCNF *obj = read_objectives[i];
        std::vector<BasicClause*> cls = obj->get_clause_vector();
        std::vector<LINT> *obj_conv = new std::vector<LINT>();
        for(LINT j{0}; j < cls.size(); ++j){
            BasicClause *cl = cls[j];
            if(cl == nullptr)
                std::cout << "clause is null" << std::endl;
            if(cl->size() > 1 || *(cl->begin()) < 0){
                LINT fresh_var = id_count + 1;
                id_count++;
                // encode fresh_var
                encode_fresh(hard, cl, fresh_var);
                obj_conv->push_back(fresh_var);
            }
            else
                obj_conv->push_back(*(cl->begin()));
        }
        objectives[i] = obj_conv;        
    }
    // encode with odd even merge sorting network
    std::vector<std::pair<LINT, LINT>> sorted_vecs(num_objectives);
    for(short i{0}; i < num_objectives; ++i){   
        std::vector<LINT> *objective = objectives[i];
        LINT num_terms = objective->size();
        sorted_vecs[i]= std::make_pair(id_count + 1, id_count + num_terms);
        id_count += num_terms;
        SNET sorting_network(num_terms, nullptr); // sorting_network is initialized to a vector of nullptrs
        // elems_to_sort is represented by a pair (first element, number of elements).
        std::pair<LINT,LINT> elems_to_sort(0,num_terms);
        encode_network(hard, elems_to_sort, id_count, objective, sorting_network);
        // relate outputs of sorting_network with sorted_vec variables
        for(LINT j{0}; j < num_terms; j++){
            if(sorting_network[j]==nullptr)
                std::cout << "Entry " << j << " of the sorting network is null" << std::endl;
            LINT output_j = sorting_network[j]->second;
            LINT o = sorted_vecs[i].first + j;
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

    };
    // check if sorting_network is working: print clauses and send to sat solver.
    print_cnf(hard, id_count);
    return 0;
}
