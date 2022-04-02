#include <leximaxIST_Encoder.h>
#include <leximaxIST_Options.h>
#include <leximaxIST_printing.h>
#include <MaxSATFormula.h>
#include <ParserPB.h>
#include <string>
#include <iostream> // std::cout, std::cin

void print_header()
{
    std::cout << "c ----------------------------------------------------------------------\n";
    std::cout << "c leximaxIST - C++ Library for Boolean Leximax Optimisation\n";
    std::cout << "c Authors: Miguel Cabral, Mikolas Janota, Vasco Manquinho\n";
    std::cout << "c Contributors:\n";
    std::cout << "c     * From Open-WBO: João Cortes, Ruben Martins, Inês Lynce,\n";
    std::cout << "c       Miguel Neves, Norbert Manthey, Saurabh Joshi.\n"
    std::cout << "c ----------------------------------------------------------------------" << '\n';
}

int main(int argc, char *argv[])
{
    /* parse options */
    leximaxIST::Options options;
    if (!options.parse(argc, argv)) {
        leximaxIST::print_error_msg("Error parsing options. Exiting.");
        options.print_usage(std::cout);
        return 1;
    }
    if (options.get_help() == 1) {
        options.print_usage(std::cout);
        return 0;
    }
    
    leximaxIST::Encoder enc;

    // TODO: set parameters of the encoder, based on options
    
    enc.set_verbosity(options.get_verbosity());
    enc.set_opt_mode(options.get_optimise());
    
    if (options.get_verbosity() > 0 && options.get_verbosity() <= 2) {
        print_header();
        std::cout << "c Reading problem... file: " << options.get_input_file_name() << '\n';
    }
    
    // TODO: read pbmo file and encode PB constraints as clauses
    MaxSATFormula maxsat_formula;
    ParserPB parser_pb (&maxsat_formula);
    parser_pb.parsePBFormula(options.get_input_file_name().c_str());
    
    
    
    std::vector<std::vector<int>> hard_cls;
    std::vector<std::vector<std::vector<int>>> obj_funcs;
    
    hard_cls.resize(maxsat_formula.nHard());
    
    // add hard clauses
    for (size_t pos (0); pos < maxsat_formula.nHard(); ++pos) {
        std::vector<int> hc (maxsat_formula.getHardClause(pos).clause);
        enc.add_hard_clause(hc);
    }
    // use an encoder to encode the pseudo-boolean constraints to cnf and add the clauses to enc
    
    // add objective functions
    for (int i (0); i < maxsat_formula.nObjFunctions(); ++i) {
        std::vector<leximaxIST::Clause> soft_clauses;
        const PBObjFunction *obj (maxsat_formula.getObjFunction(i));
        for (size_t j (0); j < obj->get_lits().size(); ++j) {
            // repeat the clause according to its weight (for now this is how we do this)
            for (int k (1); k <= obj->get_coeffs().at(j); ++k) {
                leximaxIST::Clause sc;
                sc.push_back(-(objs->get_lits().at(j)));
                soft_clauses.push_back(sc);
            }            
        }
        enc.add_soft_clauses(soft_clauses);
    }
    
    
    
    // TODO: approximate
    
    // TODO: optimise
    
    // TODO: print solution to standard output

    return 0;
}
