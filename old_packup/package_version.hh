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
 * File:   package_version.h
 * Author: mikolas
 *
 * Created on August 23, 2010, 5:29 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef PACKAGE_VERSION_H
#define	PACKAGE_VERSION_H

#include <string>
#include <iostream>
#include <sstream>
#include "common_types.hh"
#include "PackageVersions.hh"

using std::string;
using std::cerr;
using std::stringstream;

extern hash<int>         h2;
extern hash<const char*> h1;

class PackageVersion {
public:
    PackageVersion() : _version(0) {};

    PackageVersion(const PackageVersion& pv)
    : _name (pv._name), _version (pv._version), _hash_code(pv._hash_code) {}

    PackageVersion(const string& pname, Version pversion)
    : _name(pname), _version(pversion),_hash_code(h1(pname.data()) ^ h2(pversion)) {}

    void print () const {cerr << "[" << (name()) << "," << version() << "]";}

    string to_string() const {
        string return_value =  string ("[");
        return_value+=name();
        return_value+=string (",");
       
        stringstream ss;
        ss << version();

        return_value+=ss.str();
        return_value+=string("]");
        return return_value;
    }

    size_t  hash_code() const {return _hash_code;}
    Version version()   const {return _version;}
    string  name()      const {return _name;}
private:
    string   _name;
    Version  _version;
    size_t   _hash_code;
};

struct eq_package_version {
  bool operator()(const PackageVersion& pv1, const PackageVersion& pv2) const {
      //cout << "comparing "; pv1.print();pv2.print();
      if (pv1.version() != pv2.version()) return false;
      if (!SAME_PACKAGE_NAME(pv1.name(),pv2.name())) return false;
      //bool rv=pv1.name.compare((pv2.name))==0;
      //cout << "true";
      return true;
  }
};

struct hash_package_version {
  size_t operator()(const PackageVersion& pv) const {return pv.hash_code();}
};
#endif	/* PACKAGE_VERSION_H */

