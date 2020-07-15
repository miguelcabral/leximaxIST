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
 * File:   InstallableUnit.hh
 * Author: mikolas
 *
 * Created on September 25, 2010, 5:27 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef INSTALLABLEUNIT_HH
#define	INSTALLABLEUNIT_HH
#include "collections.hh"

class InstallableUnit
{
public:
    string name;
    bool installed;
    Version version;
    PackageVersionList provides;
    PackageVersionsList conflicts;
    PackageVersionsCNF depends_cnf; // cnf that must be true because of depends
    PackageVersionsCNF recommends_cnf; // cnf that is recommended to be true because of reccommends
    KeepValue keep;

    bool needed;
    Variable variable;

    inline void dump(ostream& out)
    {
        out << "package: " << name << endl;
        out << "version: " << version << endl;
        out << "installed: " << (installed ? "true" : "false") << endl;

        out << "provides: ";
        collection_printing::print(provides,out);
        out << endl;

        out << "conflicts: ";
        collection_printing::print(conflicts,out);
        out << endl;

        out << "depends: ";
        collection_printing::print(depends_cnf,out);
        out << endl;

        out << "recommends: ";
        collection_printing::print(recommends_cnf,out);
        out << endl;

        out << "keep: " << to_string(keep) << endl;
    }
};

struct InstallableUnitCmp {
 inline bool operator()(const InstallableUnit* u1,const InstallableUnit* u2) const
 {
     return u1->version < u2->version;
 }
};


#endif	/* INSTALLABLEUNIT_HH */

