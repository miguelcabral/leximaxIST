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
/* Copyright (C) 2011, Mikolas Janota*/
#include "collections.hh"

void collection_printing::print(CONSTANT PackageVersionsList &pvl,ostream& out)
{
    for (UINT i=0;i<pvl.size();++i)
    {
        if (i>0) out << ", ";
        pvl[i].print(out);
    }
}

void collection_printing::print(CONSTANT PackageVersionList &pvl,ostream& out)
{
    for (UINT i=0;i<pvl.size();++i)
    {
        if (i>0) out << ", ";
        out << pvl[i].to_string();
    }
}

void collection_printing::print(CONSTANT PackageVersionsCNF &pvs_cnf, ostream& out)
{
    for (UINT i=0;i<pvs_cnf.size();++i)
    {
        if (i>0) out << " & ";
        print(*(pvs_cnf[i]), out);
    }
}

