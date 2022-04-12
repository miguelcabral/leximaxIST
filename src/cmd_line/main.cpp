#include <leximaxIST_types.h>
#include <leximaxIST_Solver.h>
#include <leximaxIST_Options.h>
#include <leximaxIST_printing.h>
#include <MaxSATFormula.h>
#include <ParserPB.h>
#include <Encoder.h>
#include <string>
#include <iostream> // std::cout, std::cin

void print_header()
{
    std::cout << "c -------------------------------------------------------------------------\n";
    std::cout << "c leximaxIST - C++ Library for Boolean Leximax Optimisation\n";
    std::cout << "c Authors: Miguel Cabral, Mikolas Janota, Vasco Manquinho\n";
    std::cout << "c Contributors:\n";
    std::cout << "c     * From Open-WBO: João Cortes, Ruben Martins, Inês Lynce,\n";
    std::cout << "c       Miguel Neves, Norbert Manthey, Saurabh Joshi, Andreia P. Guerreiro.\n"
    std::cout << "c -------------------------------------------------------------------------\n";
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
    
    leximaxIST::Solver solver;
    solver.set_verbosity(options.get_verbosity());
    
    if (options.get_verbosity() > 0 && options.get_verbosity() <= 2) {
        print_header();
        std::cout << "c Reading problem... file: " << options.get_input_file_name() << '\n';
    }
    
    // read pbmo file
    leximaxIST::MaxSATFormula maxsat_formula;
    leximaxIST::ParserPB parser_pb (&maxsat_formula);
    parser_pb.parsePBFormula(options.get_input_file_name().c_str());
    
    // add hard clauses
    for (size_t pos (0); pos < maxsat_formula.nHard(); ++pos) {
        std::vector<int> hc (maxsat_formula.getHardClause(pos).clause);
        solver.add_hard_clause(hc);
    }
    
    // use an encoder to encode the pseudo-boolean constraints to cnf and add the clauses to solver
    leximaxIST::Encoder enc (leximaxIST::_INCREMENTAL_NONE_,
                             options.get_card_enc(),
                             leximaxIST::_AMO_LADDER_,
                             options.get_pb_enc());
    // pb constraints
    for (int i = 0; i < maxsat_formula.nPB(); i++) {
        // Make sure the PB is on the form <=
        if (!maxsat_formula.getPBConstraint(i)._sign)
            maxsat_formula.getPBConstraint(i).changeSign();
        enc.encodePB(solver, maxsat_formula.getPBConstraint(i)._lits,
                        maxsat_formula.getPBConstraint(i)._coeffs,
                        maxsat_formula.getPBConstraint(i)._rhs);
    }
    // cardinality and at most one
    for (int i = 0; i < maxsat_formula.nCard(); i++) {
        if (maxsat_formula.getCardinalityConstraint(i)._rhs == 1) {
            enc.encodeAMO(solver, maxsat_formula.getCardinalityConstraint(i)._lits);
        } else {
            enc.encodeCardinality(solver,
                                    maxsat_formula.getCardinalityConstraint(i)._lits,
                                    maxsat_formula.getCardinalityConstraint(i)._rhs);
        }
    }
    
    // add objective functions
    for (int i (0); i < maxsat_formula.nObjFunctions(); ++i) {
        std::vector<leximaxIST::Clause> soft_clauses;
        const PBObjFunction &obj (maxsat_formula.getObjFunction(i));
        for (size_t j (0); j < obj.get_lits().size(); ++j) {
            // repeat the clause according to its weight (for now this is how we do this)
            for (int k (1); k <= obj.get_coeffs().at(j); ++k) {
                leximaxIST::Clause sc;
                sc.push_back(-(objs.get_lits().at(j)));
                soft_clauses.push_back(sc);
            }            
        }
        solver.add_soft_clauses(soft_clauses);
    }
    
    // approximation
    if (!options.get_approx().empty()) {
        solver.set_approx(options.get_approx());
        solver.set_mss_incr(options.get_mss_incr());
        solver.set_mss_add_cls(options.get_mss_add_cls());
        solver.set_mss_tol(options.get_mss_tol());
        solver.set_gia_incr(options.get_gia_incr());
        solver.set_gia_pareto(options.get_gia_pareto());
        solver.set_approx_tout(options.get_timeout());
        solver.approximate();
    }
    // optimisation
    if (!options.get_optimise().empty() && solver.get_status() != 'u') {
        solver.set_disjoint_cores(options.get_disjoint_cores());
        solver.set_opt_mode(options.get_optimise());
        solver.optimise();
    }
    
    solver.print_solution();

    return 0;
}
