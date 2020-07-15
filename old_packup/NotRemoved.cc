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
 * File:   NotRemoved.cc
 * Author: mikolas
 * 
 * Created on October 11, 2010, 6:51 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#include "NotRemoved.hh"
#include "InstallableUnit.hh"

NotRemoved::NotRemoved(const PackageUnits& units)
: units (units)
{
}

NotRemoved::~NotRemoved() {
}

void NotRemoved::add(CONSTANT string& package_name) {
    if (CONTAINS (not_removed, package_name)) return;
#ifdef CONV_DBG
    cerr << "not removed: " << package_name << endl;
#endif
    not_removed.insert(package_name);
    StringSet required;
    PackageUnits::const_iterator i = units.find(package_name);
    if (i==units.end()) return;
    CONSTANT UnitVector & unit_vector =*(i->second);
    bool first=true;
    FOR_EACH (UnitVector::const_iterator,  unit_index, unit_vector)
    {
        CONSTANT InstallableUnit&  unit=**unit_index;
        StringSet new_required;
        FOR_EACH (PackageVersionsCNF::const_iterator,clause_index,unit.depends_cnf)
        {
            CONSTANT PackageVersionsList& literals = **clause_index;
            if (literals.size() != 1) continue;//considering only unit requirements
            CONSTANT string& requirement_name = literals[0].name();
            if (first || CONTAINS(required,requirement_name)) new_required.insert(requirement_name);
        }
        required = new_required;
        first=false;
    }

    FOR_EACH (StringSet::const_iterator, requirement_index, required)
    {
        add (*requirement_index);
    }
}

