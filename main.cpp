#include <string>
#include <iostream> // std::cout, std::cin
#include <vector> // std::vector
#include <utility> // std::pair
#include "Leximax_encoder.h"

void print_usage(ostream &output) {
    output << "Usage: ./leximax [--help | -h] [--leave-temporary-files] [--pbo] [--multiplication-string str] [--external-solver command] HARD SOFT [SOFT]..." << std::endl;
    output << "Computes a leximax optimal solution (if one exists) of the problem with constraints HARD and objective functions SOFT."  << std::endl;
    output << "Example: ./leximax hard.cnf obj1.cnf obj2.cnf" << std::endl;
    output << std::endl;
    output << "HARD and SOFT must be a file in DIMACS format." << std::endl;
    output << std::endl;
    output << "Options:" << std::endl;
    output << "  --help,-h\t\t\t print this message" << std::endl;
    output << "  --leave-temporary-files\t do not delete temporary files" << std::endl;
    output << "  --pbo\t\t\t\t external solver is a PBO solver" << std::endl;
    output << "\t\t\t\t default: MaxSAT solver"<< std::endl;
    output << "  --multiplication-string\t string between coefficients and variables of PBO solver"<< std::endl;
    output << "\t\t\t\t default '*'"<< std::endl;
    output << "  --external-solver\t\t command for the external solver;" << std::endl;
    output << "\t\t\t\t can be PBO or MaxSAT solver" << std::endl;
    output << "\t\t\t\t default '??? what should we put?'"<< std::endl;
    output << std::endl;
    output << "NOTE" << std::endl;
    output << "Output is produced to the standard output." << std::endl;
}

int main(int argc, char *argv[])
{
    /* parse options */
    Options options;
    if (!options.parse(argc, argv)) {
        std::cerr << "Error parsing options. Exiting." << std::endl;
        print_usage(std::cerr);
        exit(1);
    }
    if (options.m_help) {
        print_usage(std::cout);
        exit(0);
    }
    Leximax_encoder enc(options);
    // read input problem: hard.cnf f_1.cnf f_2.cnf ...
    int retv = enc.read();
    if (retv == 1)
        return 1;
    // encode sorted vectors with sorting network
    enc.encode_sorted();
    enc.solve();
    //enc.verify();
    return 0;
}
