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
 * File:   Options.hh
 * Author: mikolas
 *
 * Created on April 23, 2011, 4:23 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef OPTIONS_HH
#define	OPTIONS_HH
#include <getopt.h>
#include <assert.h>
#include <string>
using std::string;

class Options {
public:
    Options();
    virtual ~Options();
    int    read_digit(const char *optarg, const std::string &optname, int &member);
    bool   parse(int count,char** arguments);
    int    get_solving_disabled()  const { return solving_disabled; }
    string get_opt_solver()   const { return opt_solver; }
    int    get_ub_encoding()     const { return ub_encoding; }
    int    get_paranoid()          const { return paranoid; }
    int    get_trendy()            const { return trendy; }
    string get_solution_check()    const { return solution_check; }
    string get_mapping_file()      const { return mapping_file; }
    string get_user_criterion()    const { return user_criterion; }
    string get_input_file_name()   const { return input_file_name; }
    string get_output_file_name()  const { return output_file_name; }
    string get_multiplication_string() const { return multiplication_string; }
    string get_temporary_directory()   const { return temporary_directory; }
    int    get_leave_temporary_files() const { return leave_temporary_files; }
    const string&    get_formalism()   const { return formalism; }
    const string&    get_lp_solver()   const { return lp_solver; }
    const string&    get_opt_mode()   const { return opt_mode; }
    int    get_verbosity()             const { return verbosity; }
    int    get_help() const { return help; }
    int    get_leximax() const { return leximax; }
    int    get_simplify_last() const { return simplify_last; }
    
    
private:
    int    help;
    int    solving_disabled;
    string opt_solver;
    int    ub_encoding;
    int    verbosity;
    int    paranoid;
    int    trendy;
    string solution_check;
    string mapping_file;
    string user_criterion;
    string input_file_name;
    string output_file_name;
    string multiplication_string;
    int    leave_temporary_files;
    string temporary_directory;
    string formalism;
    int    leximax;
    int    simplify_last;
    string lp_solver;
    string opt_mode;
};

#endif	/* OPTIONS_HH */

