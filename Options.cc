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
#include "Options.hh"
using std::cerr;
using std::endl;

Options::Options()
: m_help(0)
, m_leave_temporary_files(0)
, m_pbo(0)
, m_num_objectives(0)
, m_multiplication_string("*")
, m_solver("~/thesis/default-solver/open-wbo/open-wbo_static")
{}

bool Options::parse(int argc,char **argv) {
    bool return_value = true;

    static struct option long_options[] = {
         {"help", no_argument,    &m_help, 1}
       ,{"external-solver", required_argument,  0, 500}
       ,{"multiplication-string",  required_argument,  0, 501}
       ,{"leave-temporary-files",  no_argument,  &m_leave_temporary_files, 1}
       ,{"pbo", no_argument,  &m_pbo, 1}
       ,{0, 0, 0, 0}
             };

    int c;
    while (1) {
       /* getopt_long stores the option index here. */
       int option_index = 0;
       c = getopt_long(argc, argv, "h", long_options, &option_index);
       opterr = 0;
       /* Detect the end of the options. */
       if (c == -1) break;
       switch (c) {
            case 0: if (long_options[option_index].flag != 0) break;
                    else return false;
            case 'h': m_help     = 1; break;
            case 500: m_solver = optarg; break;
            case 501: m_multiplication_string = optarg; break;
            case '?':
             if (isprint (optopt))
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
            cerr << "cnf files expected" << endl;
            return_value = false;
        } else {
            while (optind < argc) {
                // store name of input files
                m_input_files.push_back(argv[optind++]);
            }
        }
    }

    if (optind < argc) {
        cerr << "WARNING: Unprocessed options at the end." << endl;
    }
    
    return return_value;
}

