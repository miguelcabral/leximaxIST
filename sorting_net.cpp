#include "sorting_net.h"

/* a sorting network is a vector with size equal to the number of wires.
 * Entry i contains the last comparator (in the construction of the sorting network) that connects to wire i.
 * The comparator is represented as a pair. The first component is the other wire j that the comparator connects to.
 * The second component is the variable representing
 * the smallest of the outputs, if i < j
 * the greatest of the outputs, if i > j.*/

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
    lits.push_back(-var_in1);
    hard.get_clause_vector().push_back(hard.get_clauses().create_clause(lits));
    lits.clear();
    lits.push_back(var_out_max);
    lits.push_back(-var_in2);
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
    lits.push_back(var_out_min);
    lits.push_back(-var_in1);
    lits.push_back(-var_in2);
    hard.get_clause_vector().push_back(hard.get_clauses().create_clause(lits));
}

void insert_comparator(ReadCNF &hard, LINT el1, LINT el2, LINT &id_count, std::vector<LINT> *objective, SNET &sorting_network)
{
    //std::cout << "Inserting comparator between wires " << el1 << " and " << el2 << std::endl;
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
    if(el1 > el2)
        comp1 = new std::pair<LINT,LINT>(el2, var_out_max);
    else
        comp1 = new std::pair<LINT,LINT>(el2, var_out_min);
    sorting_network[el1] = comp1;
    if(el2 > el1)
        comp2 = new std::pair<LINT,LINT>(el1, var_out_max);
    else
        comp2 = new std::pair<LINT,LINT>(el1, var_out_min);
    sorting_network[el2] = comp2;
}

LINT ceiling_of_half(LINT number)
{
    // floor
    LINT result = number/2;
    // if number is odd then add 1
    if(number % 2 != 0)
        result++;
    return result;
}

void odd_even_merge(ReadCNF &hard, std::pair<std::pair<LINT,LINT>,LINT> seq1, std::pair<std::pair<LINT,LINT>,LINT> seq2, LINT &id_count, std::vector<LINT> *objective, SNET &sorting_network)
{
    LINT el1;
    LINT el2;
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
        LINT offset1 = seq1.second;
        LINT offset2 = seq2.second;
        LINT first1 = seq1.first.first;
        LINT first2 = seq2.first.first;
        std::pair<LINT,LINT> p1(first1, ceiling_of_half(size1));
        std::pair<std::pair<LINT,LINT>,LINT> odd1(p1, 2*offset1);
        std::pair<LINT,LINT> p2(first2, ceiling_of_half(size2));
        std::pair<std::pair<LINT,LINT>,LINT> odd2(p2, 2*offset2);
        odd_even_merge(hard, odd1, odd2, id_count, objective, sorting_network);
        // merge even subsequences
        // size of even subsequence is the floor of half of the size of the original sequence
        p1 = std::make_pair(first1 + offset1, size1/2);
        std::pair<std::pair<LINT,LINT>,LINT> even1(p1, 2*offset1);
        p2 = std::make_pair(first2 + offset2, size2/2);
        std::pair<std::pair<LINT,LINT>,LINT> even2(p2, 2*offset2);
        odd_even_merge(hard, even1, even2, id_count, objective, sorting_network);
        // comparison-interchange
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
        LINT n = size - m;
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
