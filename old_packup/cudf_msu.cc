/******************************************************************************\
 *    This file is part of packup.                                            *
 *                                                                            *
 *    packup is free software: you can redistribute it and/or modify          *
 *    it under the terms of the GNU General Public License as published by    *
 *    the Free Software Foundation, either version 3 of the License, or       *
 *    (at your option) any later version.                                     *
 *                                                                            *
 *    packup is distributed in the hope that it will be useful,               *
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *    GNU General Public License for more details.                            *
 *                                                                            *
 *    You should have received a copy of the GNU General Public License       *
 *    along with packup.  If not, see <http://www.gnu.org/licenses/>.         *            
\******************************************************************************/           
/* Copyright (C) 2011, Mikolas Janota */
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <signal.h>
#include "common_types.hh"
#include "Lexer.hh"
#include "ConverterMem.hh"
#include "SolutionReader.hh"
#include "SolverWrapperTypes.hh"
#include "Options.hh"
#include <leximaxIST_Encoder.h>
using std::ifstream;

static const char* dist_date = ""DISTDATE"";
static const char* changeset = ""CHANGESET"";
static const char* release = "0.6";

IDManager    id_manager;
SolverType   solver(id_manager);
ConverterMem parser(solver,id_manager);

extern int    yyparse (void);
extern Lexer* lexer;

bool parse_lexicographic_specification (const char* specification,vector<Objective>& lexicographic) {
    std::stringstream ss(specification);
    std::string item;
    while(std::getline(ss, item, ',')) {
        CONSTANT bool      maximize(item[0]=='+');
        CONSTANT string    sfunction(item.substr(1));
        OBJECTIVE_FUNCTION function;
        if (sfunction.compare("new")==0) function = COUNT_NEW;
        else if (sfunction.compare("unmet_recommends")==0) function = COUNT_UNMET_RECOMMENDS;
        else if (sfunction.compare("removed")==0) function = COUNT_REMOVED;
        else if (sfunction.compare("notuptodate")==0) function = COUNT_NOT_UP_TO_DATE;
        else if (sfunction.compare("changed")==0) function = COUNT_CHANGED;
        else { cerr << "unknown criterion " << sfunction << endl ; return false; }
        lexicographic.push_back(Objective(function, maximize));
    }
    return true;
}

//#ifdef EXTERNAL_SOLVER
static void SIG_handler(int signum) {
  cerr << "# received external signal " << signum << '\n'; 
  leximaxIST::Encoder *leximax_enc (solver.get_leximax_enc());
  if (leximax_enc != nullptr) {
      //leximax_enc->terminate(); // terminate is not ready yet
      if (leximax_enc->get_status() == 's')
        solver.set_leximax_model(leximax_enc->get_solution());
      else
          solver.set_leximax_model(std::vector<int>());
      solver.print_leximax_info();
  }
  parser.get_encoder().print_time();
  parser.get_encoder().print_solution();
  cerr << "# Terminating ..." << '\n';
  exit(0);
}
/*#else
static void SIG_handler(int signum) {
  cerr << "# received external signal " << signum << '\n';
  parser.get_encoder().print_time();
  parser.get_encoder().print_solution();
  cerr << "Terminating ..." << '\n';
  exit(0);
}
#endif*/ /* !EXTERNAL_SOLVER */

void print_usage(ostream &output) {
    output << "USAGE " << endl;
    output << "\t./packup [OPTIONS] instance_file_name [output_file_name]"  << endl;
    output << "General Options:" << endl;
    output << "--help | -h \t\t\t\t\t print this message" << endl;
    output << "--external-solver <string>\t\t\t command for the external solver" << endl;
    output << "\t\t\t\t\t\t default 'minisat+ -ansi -cs'"<< endl;
    output << "-f wcnf|opb|lp\t\t\t\t\t external solver formalism" << endl;
    output << "\t\t\t\t\t\t default 'wcnf'"<< endl;
    output << "--mstr | --multiplication-string <string>\t string used in the opb format"<< endl;
    output << "\t\t\t\t\t\t default '*'"<< endl;
    output << "--leave-temporary-files\t\t\t\t do not delete temporary files" << endl;
    output << "--leximax\t\t\t\t\t Optimisation with the leximax order" << endl;
    output << "\t\t\t\t\t\t default: lexicographic order"<< endl;
    output << "User criteria:" << endl;
    output << "-t \t\t\t\t\t\t trendy" << endl;
    output << "-p \t\t\t\t\t\t paranoid" << endl;
    output << "-u cs \t\t\t\t\t\t user criteria" << endl;
    output << "Lexicographic Options:"<< endl;
    output << "--temporary-directory DIR\t\t\t where temporary files should be placed"<< endl;
    output << "\t\t\t\t\t\t default '/tmp'"<< endl;
    output << "Leximax Options:"<< endl;
    output << "--lpsolver <solver>\t\t\t\t name of lp solver" << endl;
    output << "\t\t\t\t\t\t default 'gurobi'"<< endl;
    output << "-v <int>\t\t\t\t\t verbosity level of leximax library" << endl;
    output << "\t\t\t\t\t\t 0 - print nothing (default)"<< endl;
    output << "\t\t\t\t\t\t 1 - print general info"<< endl;
    output << "\t\t\t\t\t\t 2 - debug"<< endl;
    output << "--simplify-last\t\t\t\t\t use a simplified encoding in last iteration"<< endl;
    output << "\t\t\t\t\t\t (Can not use with internal optimisation)" << endl;
    output << "\t\t\t\t\t\t default: off"<< endl;
    output << "--opt-mode <string>\t\t\t\t optimisation mode"<< endl;
    output << "\t\t\t\t\t\t 'Original SAT-based Algorithm:"<< endl;
    output << "\t\t\t\t\t\t -> 'external' - Call external MaxSAT/PBO/LP solver"<< endl;
    output << "\t\t\t\t\t\t -> 'bin' - binary search with incremental SAT solver (default)"<< endl;
    output << "\t\t\t\t\t\t -> 'linear-su' - linear SAT-UNSAT search with incremental SAT solver"<< endl;
    output << "\t\t\t\t\t\t -> 'linear-us' - linear UNSAT-SAT search with incremental SAT solver"<< endl;
    output << "\t\t\t\t\t\t 'Core-guided SAT-based Algorithm:"<< endl;
    output << "\t\t\t\t\t\t -> 'core-dynamic' - Dynamic sorting networks that grow using sort and merge"<< endl;
    output << "\t\t\t\t\t\t -> 'core-dynamic-rebuild' - Dynamic sorting networks that grow by rebuild"<< endl;
    output << "\t\t\t\t\t\t -> 'core-dynamic-rebuild-incr' - Dynamic sorting networks that grow by rebuild - incremental"<< endl;
    output << "\t\t\t\t\t\t -> 'core-static' - Static sorting networks"<< endl;
    output << "--disjoint-cores\t\t\t\t find disjoint cores before the core-guided algorithm"<< endl;
    output << "--mss-presolve\t\t\t\t\t approximate the leximax-optimum with MSS enumeration"<< endl;
    output << "--mss-add-cls <int>\t\t\t\t how to add the clauses to the MSS in construction"<< endl;
    output << "\t\t\t\t\t\t 0 - add all satisfied clauses"<< endl;
    output << "\t\t\t\t\t\t 1 - add as much as possible while trying to even out the upper bounds (default)"<< endl;
    output << "\t\t\t\t\t\t 2 - add only the satisfied clause used in the SAT test"<< endl;
    output << "--mss-tol <int>\t\t\t\t\t tolerance for the choice of the next clause";
    output << " tested for satisfiability, in the MSS linear search"<< endl;
    output << "\t\t\t\t\t\t Must be a percentage (an integer between 0 and 100)"<< endl;
    output << "\t\t\t\t\t\t default: 50"<< endl;
    output << "--mss-timeout <float>\t\t\t\t Limit duration of the MSS enumeration"<< endl;
    output << "\t\t\t\t\t\t default: 86400 (seconds) (1 day)"<< endl;
    output << "--mss-incr\t\t\t\t\t use the same ipasir solver in every MSS linear search"<< endl;
    output << "--pareto-presolve\t\t\t\t approximate the leximax-optimum with a modified GIA"<< endl;
    output << "\t\t\t\t\t\t (Guided Improvement Algorithm)"<< endl;
    output << "--truly-pareto\t\t\t\t\t Guarantee intermediate Pareto-optimal solutions"<< endl;
    output << "--pareto-incr\t\t\t\t\t use the same ipasir solver in every Pareto-optimal solution search"<< endl;
    output << "--pareto-timeout <float>\t\t\t Limit duration of the Pareto presolving phase"<< endl;
    output << "\t\t\t\t\t\t default: 86400 (seconds) (1 day)"<< endl;
    output << "--maxsat-presolve\t\t\t\t Minimise the sum of the objectives"<< endl;
    output << "\t\t\t\t\t\t (Provides upper and lower bounds on the optimal maximum)"<< endl;
    output << "--maxsat-psol-cmd <string>\t\t\t external MaxSAT solver command in presolve" << endl;
    output << "NOTE" << endl;
    output << "If the input file is '-', input is read from the standard input." << endl;
    output << "If the output filename is omitted, output is produced to the standard output." << endl;
}

#define TIMETOL 1

void print_header() {
   cout << "# authors: Mikolas Janota, Joao Marques-Silva" << endl;
   cout << "# contributors: Ines Lynce, Vasco Manquinho" << endl;
   cout << "# email mikolas@sat.inesc-id.pt" << endl;
   cout << "# (C) 2011 Mikolas Janota" << endl;
   cout << "# release: " << release <<  " distribution date: " << dist_date <<  " change set: " << changeset << endl;
   cout << "# Released under the GPL license." << endl;
}

int main(int argc, char** argv) {
    print_header();
    
    /* set up signals */
/*#ifdef EXTERNAL_SOLVER
// Miguel: in this case nothing is done TODO
    // assuming external solver received as well
    struct sigaction new_act1;
    new_act1.sa_handler = SIG_IGN;
    sigaction(SIGTERM, &new_act1, 0);
    struct sigaction new_act2;
    new_act2.sa_handler = SIG_IGN;
    sigaction(SIGUSR1, &new_act2, 0);
    struct sigaction new_act3;     
    new_act3.sa_handler = SIG_IGN;
    sigaction(SIGUSR2, &new_act3, 0);
#else*/
    signal(SIGHUP, SIG_handler);
    signal(SIGTERM, SIG_handler);
    //signal(SIGABRT, SIG_handler);
    signal(SIGUSR1, SIG_handler);
    signal(SIGINT, SIG_handler);
    //signal(SIGALRM,SIG_handler);
    //alarm(290);  // Specify alarm interrupt given timeout
//#endif

    /* parse options */
    Options options;
    if (!options.parse(argc, argv)) {
        cerr << "Error parsing options. Exiting." << endl;
        print_usage(cerr);
        exit(1);
    }
    if (options.get_help()) {
        print_usage(cout);
        exit(0);
    }

#ifdef MAPPING
    if (!options.get_mapping_file().empty()) parser.set_maping_file(options.get_mapping_file().c_str());
#endif

    /* handling of input and output */
    ifstream input_file;
    ofstream output_file;
    const bool use_stdin  = options.get_input_file_name().compare("-")==0;
    const bool use_stdout = options.get_output_file_name().empty();
    if (!use_stdin) {
        input_file.open(options.get_input_file_name().c_str());
        if (input_file.fail()) {
            cerr<<"Failed to read the input file. Exiting." << endl;
            exit(1);
        }
    }
    if (!use_stdout) {
        output_file.open(options.get_output_file_name().c_str());
        if (output_file.fail()) {
            cerr<<"Failed to open the output file. Exiting." << endl;
            exit(1);
        }
    }

    istream& input_stream(use_stdin ? cin : input_file);
    ostream& output_stream(use_stdout ? cout : output_file);

    /* set up of the encoder */
    if (options.get_solving_disabled()) parser.disable_solving();
    parser.get_encoder().set_opt_edges(1);
    parser.get_encoder().set_iv(3);
    parser.get_encoder().set_opt_not_removed(true);    
#ifdef EXTERNAL_SOLVER
    if (!options.get_opt_solver().empty())             solver.set_opt_solver_cmd(options.get_opt_solver ());
    solver.set_mss_presolve(options.get_mss_presolve());
    solver.set_pareto_presolve(options.get_pareto_presolve());
    solver.set_mss_incremental(options.get_mss_incremental());
    solver.set_pareto_incremental(options.get_pareto_incremental());
    solver.set_mss_add_cls(options.get_mss_add_cls());
    solver.set_mss_tolerance(options.get_mss_tolerance());
    solver.set_mss_nb_limit(options.get_mss_nb_limit());
    solver.set_mss_timeout(options.get_mss_timeout());
    solver.set_pareto_timeout(options.get_pareto_timeout());
    solver.set_truly_pareto(options.get_truly_pareto());
    solver.set_verbosity(options.get_verbosity());
    solver.set_opt_mode(options.get_opt_mode());
    solver.set_maxsat_psol_cmd(options.get_maxsat_psol_cmd());
    if (!options.get_multiplication_string().empty())  solver.set_multiplication_string(options.get_multiplication_string());
    if (options.get_leave_temporary_files())           solver.set_leave_temporary_files();
    if (options.get_leximax())                         solver.set_leximax();
    if (options.get_simplify_last())                   solver.set_simplify_last();
    if (options.get_maxsat_presolve())                 solver.set_maxsat_presolve();
    if (options.get_disjoint_cores())                  solver.set_disjoint_cores();
    if (!options.get_lp_solver().empty())              solver.set_lp_solver(options.get_lp_solver());
    if (!options.get_formalism().empty())              solver.set_formalism(options.get_formalism());
    if (!options.get_temporary_directory().empty()) {
        solver.set_temporary_directory(options.get_temporary_directory());
    } else {
        char * pPath;
        pPath = getenv("TMPDIR");
        if (pPath!=NULL) {
            string s(pPath);
            solver.set_temporary_directory(s);
        }
    }
#endif
    
    if (options.get_trendy())  parser.get_encoder().set_criterion(TRENDY);
    if (options.get_paranoid())  parser.get_encoder().set_criterion(PARANOID);
    if (!options.get_user_criterion().empty()) {
        vector<Objective> lexicographic;
        if (!parse_lexicographic_specification(options.get_user_criterion().c_str(), lexicographic)) {
            cerr<<"Failed to parse the user defined criteria (" << options.get_user_criterion() << "). Exiting." << endl;
            exit(1);
        }
        //solver.set_pbmo_file_name(options.get_input_file_name(), lexicographic);
        parser.get_encoder().set_lexicographic(lexicographic);
    }


    /* run the whole thang */
    lexer=new Lexer(input_stream);
    parser.get_encoder().set_solution_file(&output_stream);
    try {
        yyparse();
    } catch(const ReadException &e) {
        cerr << "Error in parsing the input file:" << e.what() << endl;
        cerr << "Terminating." << endl;
    }
    exit(0);
}

