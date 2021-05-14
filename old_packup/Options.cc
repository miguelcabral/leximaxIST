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
/* 
 * File:   Options.cc
 * Author: mikolas
 * 
 * Created on April 23, 2011, 4:23 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#include <iostream>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <string>
#include <cctype>
#include "Options.hh"
using std::cerr;
using std::endl;

Options::Options()
: help (0)
, solving_disabled(0)
, paranoid(0)
, trendy(0)
, leave_temporary_files(0)
, leximax(0)
, maxsat_presolve(0)
, mss_presolve(0)
, pareto_presolve(0)
, mss_incremental(0)
, pareto_incremental(0)
, truly_pareto(0)
, mss_tolerance(0) // always choose the maximum
, mss_add_cls(1) // add equally
, mss_nb_limit(0) // no limit
, mss_timeout(86400) // 24 hours
, pareto_timeout(86400) // 24 hours
, simplify_last(0)
, verbosity(0)
, formalism("wcnf")
, opt_mode ("bin")
, lp_solver("gurobi")
{}

bool Options::read_integer(const char *optarg, const std::string &optname, int &member)
{
    std::string s (optarg);
    try {
        member = std::stoi(s);
    }
    catch (const std::invalid_argument&) {
        std::cerr << "Option '" << optname << "' must be an integer";
        std::cerr << std::endl;
        return false;
    }
    catch (const std::out_of_range&) {
        std::cerr << "Out of range integer for Option '" << optname << "'";
        std::cerr << std::endl;
        return false;
    }
    return true;
}

bool Options::read_double(const char *optarg, const std::string &optname, double &member)
{
    std::string s (optarg);
    try {
        member = std::stod(s);
    }
    catch (const std::invalid_argument&) {
        std::cerr << "Option '" << optname << "' must be a floating-point number";
        std::cerr << std::endl;
        return false;
    }
    catch (const std::out_of_range&) {
        std::cerr << "Out of range floating-point number in Option '" << optname << "'";
        std::cerr << std::endl;
        return false;
    }
    return true;
}

bool Options::read_digit(const char *optarg, const std::string &optname, int &member)
{
    if (!read_integer(optarg, optname, member))
        return false;
    if (member > 9 || member < 0) {
        std::cerr << "Option " << optname << " must be an integer between 0 and 9" << std::endl;
        return false;
    }    
    return true;
}

bool Options::parse(int argc,char **argv) {
    bool return_value = true;

    static struct option long_options[] = {
         {"help", no_argument,    &help, 1}
       , {"no-sol", no_argument,  &solving_disabled, 1}
       ,{"external-solver", required_argument,  0, 500}
       ,{"solution-check",  required_argument,  0, 501}
#ifdef MAPPING
       ,{"mapping-file",    required_argument,  0, 502}
#endif
       ,{"multiplication-string",  required_argument,  0, 503}
       ,{"mstr",  required_argument,  0, 503}
       ,{"temporary-directory",    required_argument,  0, 504}
       ,{"formalism",    required_argument,  0, 505}
       ,{"lpsolver",    required_argument,  0, 506}
       ,{"lpsol",    required_argument,  0, 506}
       ,{"mss-add-cls", required_argument,  0, 507}
       ,{"opt-mode", required_argument,  0, 508}
       ,{"maxsat-psol-cmd", required_argument,  0, 509}
       ,{"mss-tol", required_argument,  0, 510}
       ,{"mss-nb-lim", required_argument,  0, 511}
       ,{"mss-timeout", required_argument,  0, 512}
       ,{"pareto-timeout", required_argument,  0, 513}
       ,{"leave-temporary-files",  no_argument,  &leave_temporary_files, 1}
       ,{"ltf",  no_argument,  &leave_temporary_files, 1}
       ,{"maxsat-presolve",  no_argument,  &maxsat_presolve, 1}
       ,{"mss-presolve",  no_argument,  &mss_presolve, 1}
       ,{"pareto-presolve",  no_argument,  &pareto_presolve, 1}
       ,{"mss-incr",  no_argument,  &mss_incremental, 1}
       ,{"pareto-incr",  no_argument,  &pareto_incremental, 1}
       ,{"truly-pareto",  no_argument,  &truly_pareto, 1}
       ,{"leximax", no_argument,  &leximax, 1}
       ,{"simplify-last", no_argument,  &simplify_last, 1}
       ,{0, 0, 0, 0}
             };

    int c;
    while (1) {
       /* getopt_long stores the option index here. */
       int option_index = 0;
       c = getopt_long(argc, argv, "hu:ptf:v:", long_options, &option_index);
       opterr = 0;
       /* Detect the end of the options. */
       if (c == -1) break;
       switch (c) {
            case 0: if (long_options[option_index].flag != 0) break;
                    else return false;
            case 'p': paranoid = 1; break;
            case 't': trendy   = 1; break;
            case 'h': help     = 1; break;
            case 'u': user_criterion  = optarg; break;
            case 'v': if (read_digit(optarg, "v", verbosity) == -1) {
                        return_value = false;
                    }
                    break;
            case 'f': formalism = optarg;
                if (formalism != "wcnf" && formalism != "opb" && formalism != "lp") {
                    fprintf(stderr, "Invalid option! Available formalism options: 'wcnf', 'opb', 'lp'.\n");
                    return_value = false;
                }
                break;
            case 500: opt_solver = optarg; break;
            case 501: solution_check  = optarg; break;
            case 502: mapping_file    = optarg; break;
            case 503: multiplication_string = optarg; break;
            case 504: temporary_directory   = optarg; break;
            case 505: formalism = optarg;
                if (formalism != "wcnf" && formalism != "opb" && formalism != "lp") {
                    fprintf(stderr, "Invalid option! Available formalism options: 'wcnf', 'opb', 'lp'.\n");
                    return_value = false;
                }
                break;
            case 506: lp_solver = optarg;
                if (lp_solver != "gurobi" && lp_solver != "scip" && lp_solver != "cplex" && lp_solver != "cbc") {
                    fprintf(stderr, "Invalid option! Possible lp solvers: 'gurobi', 'scip', 'cplex', 'cbc'.\n");
                    return_value = false;
                }
                break;
            case 507: if (!read_digit(optarg, "mss-add-cls", mss_add_cls))
                        return_value = false;
                    break;
            case 508: opt_mode = optarg; break;
            case 509: maxsat_psol_cmd = optarg; break;
            case 510: if (!read_integer(optarg, "mss-tol", mss_tolerance))
                        return_value = false;
                    break;
            case 511: if (!read_integer(optarg, "mss-nb-lim", mss_nb_limit))
                        return_value = false;
                    break;
            case 512: if (!read_double(optarg, "mss-timeout", mss_timeout))
                        return_value = false;
                    break;
            case 513: if (!read_double(optarg, "pareto-timeout", pareto_timeout))
                        return_value = false;
                    break;
           case '?':
             if ( (optopt == 'u') )
               fprintf(stderr, "Option -%c requires an argument.\n", optopt);
             else if (isprint (optopt))
               fprintf (stderr, "Unknown option `-%c'.\n", optopt);
             else
               fprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);
             return_value = false;
             break;
           default:
               return false;
        }
    }

    if (!get_help()) {
        if (optind >= argc) {
            cerr << "cudf file expected" << endl;
            return_value = false;
        } else {
             input_file_name=argv[optind++];
        }

        if (optind < argc) {
            output_file_name = argv[optind++];
        }
    }

    if (optind < argc) {
        cerr << "WARNING: Unprocessed options at the end." << endl;
    }
    
    return return_value;
}

Options::~Options() {}

