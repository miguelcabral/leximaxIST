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
    int retv = enc.read(argv);
    if (retv == 1)
        return 1;
    // encode sorted vectors with sorting network
    enc.encode_sorted();
    // enc.debug();
    enc.solve();
    /* at_most test
    std::forward_list<LINT> test_list({1,2,3,4,5,6});
    for (LINT elem : test_list)
        std::cout << "Element of list: " << elem << '\n';
    enc.at_most(test_list, 3);
    */
    enc.print_cnf();
    return 0;
}
