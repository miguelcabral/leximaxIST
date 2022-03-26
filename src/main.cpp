#include <string>
#include <iostream> // std::cout, std::cin
#include <vector> // std::vector
#include <utility> // std::pair
#include "Leximax_encoder.h"

void print_usage(ostream &output) {
    output << "Usage: ./leximax [--help | -h] [--leave-temporary-files] [--pbo] [--multiplication-string str] [--external-solver command] HARD SOFT [SOFT]..." << std::endl;
    output << "Computes a leximax optimal solution (if one exists) of the problem with constraints HARD and objective functions SOFT."  << std::endl;
    output << "Example: ./leximax hard.cnf obj1.cnf obj2.cnf" << std::endl;
    output << "HARD and SOFT must be a file in DIMACS format." << std::endl;
    output << "Options:" << std::endl;
    output << "  --help,-h\t\t\t print this message" << std::endl;
    output << "  --leave-temporary-files\t do not delete temporary files" << std::endl;
    output << "  --pbo\t\t\t\t external solver is a PBO solver" << std::endl;
    output << "\t\t\t\t default: MaxSAT solver"<< std::endl;
    output << "  --multiplication-string\t string between coefficients and variables of PBO solver"<< std::endl;
    output << "\t\t\t\t default '*'"<< std::endl;
    output << "  --external-solver\t\t command for the external solver;" << std::endl;
    output << "\t\t\t\t can be PBO or MaxSAT solver" << std::endl;
    output << "\t\t\t\t default 'rc2.py -vv'"<< std::endl;
    output << std::endl;
    output << "NOTE" << std::endl;
    output << "Output is produced to the standard output." << std::endl;
}


int main(int argc, char *argv[])
{
    /* parse options */
    Options options;
    if (!options.parse(argc, argv)) {
        print_leximax_error("Error parsing options. Exiting.");
        print_usage(std::cout);
        return 1;
    }
    if (options.m_help.get_data()) {
        print_usage(std::cout);
        return 0;
    }
    Encoder enc();

    // TODO: set parameters of the encoder, based on options
    
    enc.set_verbosity(options.get_verbosity());
    enc.set_opt_mode(options.get_optimise());
    
    // TODO: read pbmo file and encode PB constraints as clauses
    
    // TODO: approximate
    
    // TODO: optimise
    
    // TODO: print solution to standard output

    return 0;
}
