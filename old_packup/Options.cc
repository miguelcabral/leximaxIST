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
, ub_encoding(0)
, formalism("wcnf")
, lp_solver("gurobi")
{}

int Options::read_ub_enc(const char *optarg)
{
    std::string ub_enc_str (optarg);
    if (ub_enc_str.size() != 1 || !isdigit(ub_enc_str[0])) {
        std::cerr << "Option ub-enc accepts only one digit." << std::endl;
        return -1;
    }
    ub_encoding = std::stoi(ub_enc_str);
    return 0;
}

bool Options::parse(int argc,char **argv) {
    bool return_value = true;

    static struct option long_options[] = {
         {"help", no_argument,    &help, 1}
       , {"no-sol", no_argument,  &solving_disabled, 1}
       ,{"opt-solver", required_argument,  0, 500}
       ,{"solution-check",  required_argument,  0, 501}
#ifdef MAPPING
       ,{"mapping-file",    required_argument,  0, 502}
#endif
       ,{"multiplication-string",  required_argument,  0, 503}
       ,{"temporary-directory",    required_argument,  0, 504}
       ,{"formalism",    required_argument,  0, 505}
       ,{"lpsolver",    required_argument,  0, 506}
       ,{"sat-solver", required_argument,  0, 507}
       ,{"ub-enc", required_argument,  0, 508}
       ,{"leave-temporary-files",  no_argument,  &leave_temporary_files, 1}
       ,{"leximax", no_argument,  &leximax, 1}
       ,{0, 0, 0, 0}
             };

    int c;
    while (1) {
       /* getopt_long stores the option index here. */
       int option_index = 0;
       c = getopt_long(argc, argv, "hu:pt", long_options, &option_index);
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
            case 507: sat_solver = optarg; break;
            case 508: if (read_ub_enc(optarg) == -1) {
                        return_value = false;
                    }
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

