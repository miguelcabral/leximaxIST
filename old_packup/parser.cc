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
#include <string>
#include <iostream>
#include <assert.h>
#include "parser.hh"

using std::string;
using std::cout;
using std::cerr;
using std::endl;


void Parser::action_package_name(char* cname) {
    PDBG(cerr << "Package Name Token '" << (cname) << "'"<< endl;)
    //string name(cname);
    Str2Str::const_iterator index = package_names_set.find(cname);
    if (index != package_names_set.end()) {
        PDBG(cerr << "already there" << endl;)
        read_package_name = index->second;
        delete[] cname;
    } else {
        PDBG(cerr << "fstime" << endl;)
        read_package_name = cname;
        package_names_set[cname] = read_package_name;
    }
}


void  Parser::action_first_disjunction () {
    read_CNF.clear();
    read_CNF.push_back(read_clause);
}
void  Parser::action_next_disjunction () {
    read_CNF.push_back(read_clause);
}

void  Parser::action_first_literal() {
   read_clause= new VersionsList;
   read_clause->push_back(read_package_versions);
}
void  Parser::action_next_literal() {
   read_clause->push_back(read_package_versions);
}

void Parser::action_CNF_true () {
    read_CNF.clear();
}
void Parser::action_CNF_false () {
    read_CNF.clear();
    read_CNF.push_back(new VersionsList());
}
