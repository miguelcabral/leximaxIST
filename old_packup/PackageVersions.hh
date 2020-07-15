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
 * File:   PackageVersions.h
 * Author: mikolas
 *
 * Created on August 27, 2010, 6:51 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef PACKAGEVERSIONS_H
#define	PACKAGEVERSIONS_H
#include <assert.h>
#include <iostream>
#include <sstream>
#include <string>

#include "common_types.hh"
using std::string;
using std::stringstream;
using std::cout;
using std::cerr;
using std::endl;
using version_operators::Operator;
using version_operators::VERSIONS_NONE;

class PackageVersions
{
public:
    inline PackageVersions()
          : _op(VERSIONS_NONE), _version(0)
    {}

    inline PackageVersions(const PackageVersions& pvs)
    : _name(pvs._name), _op(pvs._op), _version(pvs._version)
    { assert ((op()!= VERSIONS_NONE) || version()==0); }

    inline PackageVersions(const string& pname,Operator pop,Version pversion)
       : _name(pname), _op(pop), _version(pversion) {}

    inline Version version() const {return _version;}
    inline Operator op() const {return _op;}
    inline string name() const {return _name;}

    void print (ostream& out=cerr) const {
        out<< name();
        if (_op!=VERSIONS_NONE) out << version_operators::to_string(op()) << version();
    }

    string to_string() const {
        stringstream strstr;
        print (strstr);
        return strstr.str();
    }
private:
    string _name;
    Operator _op;
    Version _version;
};

struct eq_package_versions
{
  bool operator()(const PackageVersions& pvs1, const PackageVersions& pvs2) const
  {
      if (pvs1.op() != pvs2.op()) return false;
      if ((pvs1.op() != VERSIONS_NONE) && (pvs1.version() != pvs2.version()) ) return false;
      if (!SAME_PACKAGE_NAME(pvs1.name(),pvs2.name())) return false;
      return true;
  }
};

struct hash_package_versions
{
  size_t operator()(const PackageVersions& pvs) const
  {
      std::hash<string> h1;
      return h1(pvs.name()) ^ ((size_t)pvs.op()) ^ ((size_t)pvs.version());
  }
};


#endif	/* PACKAGEVERSIONS_H */

