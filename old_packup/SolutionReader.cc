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
 * File:   SolutionReader.cc
 * Author: mikolas
 * 
 * Created on September 19, 2010, 1:45 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#include "SolutionReader.hh"
#include <iostream>
#include <string>
#include <fstream>
#include "common_types.hh"
#include "package_version.hh"

using namespace std;

inline bool begins_with (const string &s, const string & prefix)
{
    if (s.length() < prefix.length ()) return false;
    return s.compare(0, prefix.length(), prefix)==0;
}
string remove_blank(const string &s, UINT pos1)
{
    string return_value;
    UINT epos=s.length();
    for (UINT i=pos1; i < epos; ++i)
    {
        char c=s[i];
        if (c!= ' ') return_value+=c;
    }
    return return_value;
}

void SolutionReader::read (const char* file_name)
{
    file.open(file_name);
    
    const string package="package: ";
    const string version="version: ";
    const string installed="installed: ";

    bool package_read=false, version_read=false, installed_read = false;
    string package_name;
    bool package_installed=false;
    UINT package_version=-1;

    while (file.good() && !file.eof())
    {
        string line;
        getline(file, line);
        if (begins_with(line,package))
        {
            package_read = true;version_read=false; installed_read = false;
            package_name=remove_blank(line, package.length());
#ifdef PARS_DBG 
            cerr << "Package Name Token [" << package_name << "]"<< endl;
#endif    
            Str2Str::const_iterator index = _pns.find(package_name.c_str());
            if (index != _pns.end()) {
#ifdef PARS_DBG
            cerr << "already there" << endl;
#endif
                package_name=index->second;
            } else {
#ifdef PARS_DBG
        cerr << "fstime" << endl;
#endif
            _pns[package_name.c_str()]=package_name;
            }
        } else if (begins_with(line,version))
        {
            version_read = true;
            string version_str=remove_blank(line, version.length());
            package_version=atoi(version_str.c_str());
        } else if (begins_with(line,installed))
        {
            installed_read = true;
            package_installed = line.find("true")!=line.npos;
        }

        if (package_read&&version_read&&installed_read)
        {
            package_read=false; version_read=false; installed_read = false;            
            if (package_installed) {
                PackageVersion pv(package_name, package_version);
                installed_package_versions.insert(pv);

#ifdef PARS_DBG
                cerr << "sr: " << pv.to_string();// << endl;
                cerr << ( CONTAINS(installed_package_versions,pv) ? "  inserted" : " not inserted"  ) << endl;
#endif
            }
        }

    }
}



