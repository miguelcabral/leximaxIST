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
 * File:   parser_functions.h
 * Author: mikolas
 *
 * Created on August 10, 2010, 6:12 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef PARSER_FUNCTIONS_H
#define	PARSER_FUNCTIONS_H

#ifdef PARS_DBG
#define PDBG(t) t
#else
#define PDBG(t)
#endif


#include <string>
#include <vector>
#include <assert.h>
#include <iostream>
#include <stdio.h>
#include "common_types.hh"
#include "collections.hh"
#include "PackageVersions.hh"
#include "package_version.hh"

using namespace version_operators;

using std::string;
using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using std::exception;

class ReadException : public exception {
public:
    ReadException(char* str) : s(str) {};
    ~ReadException() throw()  { delete[] s; }
    const char* what() const throw() { return s; }    
private:
    char* s;
};



class Parser {
public:
    virtual ~Parser() {};
    /** Processing of a new package started.*/
    virtual void start_package() = 0;

    /** Processing of a package stopped.*/
    virtual void close_package() = 0;

    /**The version of currently processed package was read.*/
    virtual void end_processed_package_version() = 0;

    /** The package universe has been fully read.*/
    virtual void close_universe() = 0;

    /**The whole input was read.*/
    virtual void close_input () = 0;

    //Package stanza
    virtual void package_provides() = 0;
    virtual void package_conflicts() = 0; // A list of conflicting versions was read.
    virtual void package_depends() = 0;
    virtual void package_recommends() = 0;
    virtual void package_keep() = 0;
    virtual void package_installed(bool installed_value) = 0;

    //Request stanzas
    virtual void request_install() = 0;
    virtual void request_remove() = 0;
    virtual void upgrade() = 0;

    void package_versions_list_empty() {read_package_versions_list.clear();}

    void package_versions_list_first() {
        read_package_versions_list.clear();
        read_package_versions_list.push_back(read_package_versions);
    }

    void package_versions_list_next()
    {read_package_versions_list.push_back(read_package_versions);}

    void action_package_name(char* cname);
    
    void action_version_operator(Operator op) {read_operator=op;}

    Version action_version(char* version_string) {
        PDBG(cerr << "Version:" << (version_string) << endl;)
        const int r = sscanf(version_string, "%d", &read_version);
        if (r < 1) {
            static const char* ms = "Invalid version number: ";
            char* const message = new char [strlen(version_string)+strlen(ms)+1];
            strcpy(message, ms);
            strcat(message, version_string);
            delete[] version_string;
            throw ReadException(message);
        }
        delete[] version_string;        
        return read_version;
    }

    void action_package_versions() {
        read_package_versions = PackageVersions(
                read_package_name,
                read_operator,
                read_operator==VERSIONS_NONE?  0 : read_version);
    }
    
    void action_feature (){
        assert (read_operator==VERSIONS_EQUALS ||read_operator==VERSIONS_NONE);
        read_feature = PackageVersion (read_package_name,read_operator==VERSIONS_NONE?  0 : read_version);
    };

    void action_first_disjunction();
    void action_next_disjunction();
    void action_first_literal();
    void action_next_literal();
    void action_CNF_true();
    void action_CNF_false();

    void action_first_feature() {
        read_feature_list.clear ();
        read_feature_list.push_back(read_feature);
    }
    void action_next_feature(){
        read_feature_list.push_back(read_feature);
    }
    void action_empty_feature_list()
    {read_feature_list.clear();}

    void action_keep_value (KeepValue value){read_keep_value = value;}
protected:
    string read_package_name;
    Version read_version;
    Operator read_operator;
    KeepValue read_keep_value;
    vector <string> package_names;
    vector <string> feature_names;
    PackageVersions read_package_versions;
    PackageVersion read_feature;
    PackageVersionList read_feature_list;
    PackageVersionsCNF read_CNF;
    VersionsList* read_clause;
   
    VersionsList read_package_versions_list;
    Str2Str package_names_set;
};

#endif	/* PARSER_FUNCTIONS_H */

