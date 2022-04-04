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
    
    leximaxIST::Solver solver;

    // TODO: set parameters of the solver, based on options
    
    solver.set_verbosity(options.get_verbosity());
    
    if (options.get_verbosity() > 0 && options.get_verbosity() <= 2) {
        print_header();
        std::cout << "c Reading problem... file: " << options.get_input_file_name() << '\n';
    }
    
    // TODO: read pbmo file and encode PB constraints as clauses
    MaxSATFormula maxsat_formula;
    ParserPB parser_pb (&maxsat_formula);
    parser_pb.parsePBFormula(options.get_input_file_name().c_str());
    
    
    // add hard clauses
    for (size_t pos (0); pos < maxsat_formula.nHard(); ++pos) {
        std::vector<int> hc (maxsat_formula.getHardClause(pos).clause);
        solver.add_hard_clause(hc);
    }
    // use an encoder to encode the pseudo-boolean constraints to cnf and add the clauses to solver
    
    
    
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
        solver.add_soft_clauses(soft_clauses);
    }
    
    int Options::get_disjoint_cores() const {return m_disjoint_cores.get_data();}
    std::string Options::get_optimise() const {return m_optimise.get_data();}
    std::string Options::get_approx() const {return m_approx.get_data();}
    std::string Options::get_input_file_name() const {return m_input_file_name.get_data();}
    double Options::get_timeout() const {return m_timeout.get_data();}
    int Options::get_mss_tol() const {return m_mss_tol.get_data();}
    int Options::get_mss_add_cls() const {return m_mss_add_cls.get_data();}
    int Options::get_mss_incr() const {return m_mss_incr.get_data();}
    int Options::get_gia_pareto() const {return m_gia_pareto.get_data();}
    int Options::get_gia_incr() const {return m_gia_incr.get_data();}
    
    // TODO: approximate
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
    if (!options.get_optimise().empty() && solver.get_status() != 'u') {
        
    }
    
    // TODO: optimise
    
    // TODO: print solution to standard output

    return 0;
}
