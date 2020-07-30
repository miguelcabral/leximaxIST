#ifndef SORTING_NET
#define SORTING_NET
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
#include "old_packup/ReadCNF.hh"

typedef std::vector<std::pair<LINT, LINT>*> SNET;

void encode_max(ReadCNF &hard, LINT var_out_max, LINT var_in1, LINT var_in2);

void encode_min(ReadCNF &hard, LINT var_out_min, LINT var_in1, LINT var_in2);

void insert_comparator(ReadCNF &hard, LINT el1, LINT el2, LINT &id_count, std::vector<LINT> *objective, SNET &sorting_network);

void odd_even_merge(ReadCNF &hard, std::pair<std::pair<LINT,LINT>,LINT> seq1, std::pair<std::pair<LINT,LINT>,LINT> seq2, LINT &id_count, std::vector<LINT> *objective, SNET &sorting_network);

void encode_network(ReadCNF &hard, std::pair<LINT,LINT> elems_to_sort, LINT &id_count, std::vector<LINT> *objective, SNET &sorting_network);

#endif
