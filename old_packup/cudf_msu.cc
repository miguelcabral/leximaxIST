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
    output << "\t[OPTIONS] instance_file_name [output_file_name]"  << endl;
    output << "General Options:" << endl;
    output << "--help | -h \t print this message" << endl;
    output << "--external-solver <string>\t\t command for the external solver" << endl;
    output << "\t\t\t\t\t default 'minisat+ -ansi -cs'"<< endl;
    output << "-f wcnf|opb|lp\t\t external solver formalism" << endl;
    output << "\t\t\t\t\t default 'wcnf'"<< endl;
    output << "--mstr | --multiplication-string <string>\t string used in the opb format"<< endl;
    output << "\t\t\t\t\t default '*'"<< endl;
    output << "--leave-temporary-files\t\t do not delete temporary files" << endl;
    output << "--leximax\t\t\t Optimisation with the leximax order;" << endl;
    output << "\t\t\t\t\t default: lexicographic order"<< endl;
    output << "User criteria:" << endl;
    output << "-t \t\t trendy" << endl;
    output << "-p \t\t paranoid" << endl;
    output << "-u cs \t\t user criteria" << endl;
    output << "Lexicographic Options:"<< endl;
    output << "--temporary-directory DIR\t where temporary files should be placed"<< endl;
    output << "\t\t\t\t\t default '/tmp'"<< endl;
    output << "Leximax Options:"<< endl;
    output << "--lpsolver <solver>\t\t name of lp solver" << endl;
    output << "\t\t\t\t\t default 'gurobi'"<< endl;
    output << "-v <int>\t\t verbosity level of leximax library" << endl;
    output << "\t 0 - print nothing (default)"<< endl;
    output << "\t 1 - print general info"<< endl;
    output << "\t 2 - debug"<< endl;
    output << "--simplify-last\t\t use a simplified encoding in last iteration"<< endl;
    output << "\t\t\t\t\t default off"<< endl;
    output << "--opt-mode external|bin\t\t optimisation mode"<< endl;
    output << "\t 'external' - Call external MaxSAT/PBO/LP solver"<< endl;
    output << "\t 'bin' - binary search with incremental SAT solver (default)"<< endl;
    output << "\t 'linear-su' - linear SAT-UNSAT search with incremental SAT solver"<< endl;
    output << "--ub-enc <int>\t\t upper bound presolve"<< endl;
    output << "\t 0 - presolve using one call to SAT solver (default)"<< endl;
    output << "\t 1 - presolve using greedy sequential minimisation"<< endl;
    output << "\t 2 - presolve using greedy maximum minimisation"<< endl;
    output << "--maxsat-presolve\t\t upper bound AND lower bound presolve" << endl;
    output << "\t\t\t\t\t default off"<< endl;
    output << "--maxsat-psol-cmd <string>\t\t external MaxSAT solver command in presolve" << endl;
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
    signal(SIGABRT, SIG_handler);
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
    solver.set_ub_encoding(options.get_ub_encoding());
    solver.set_verbosity(options.get_verbosity());
    solver.set_opt_mode(options.get_opt_mode());
    solver.set_maxsat_psol_cmd(options.get_maxsat_psol_cmd());
    if (!options.get_multiplication_string().empty())  solver.set_multiplication_string(options.get_multiplication_string());
    if (options.get_leave_temporary_files())           solver.set_leave_temporary_files();
    if (options.get_leximax())                         solver.set_leximax();
    if (options.get_simplify_last())                   solver.set_simplify_last();
    if (options.get_maxsat_presolve())                 solver.set_maxsat_presolve();
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

