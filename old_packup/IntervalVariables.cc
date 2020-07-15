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
 * File:   IntervalVariables.cc
 * Author: mikolas
 * 
 * Created on September 26, 2010, 7:41 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#include "IntervalVariables.hh"
#include "EncoderTypes.hh"

Variable IntervalVariables::get_variable(const PackageVersion& pv, PackageVersionMap& map,
        PackageToVersions& vvs,
        const string& map_name, bool& is_fresh) {    
    PackageVersionMap::const_iterator index = map.find(pv);
    if (index != map.end()) {
        is_fresh = false;
        return index->second;
    }
    //cerr << "adding " << pv.to_string() << " to " << map_name << endl;
    is_fresh = true;
    Variable nv = new_variable();
    map[pv] = nv;
#ifdef MAPPING 
    string nm= map_name + pv.to_string();
    variable_names[nv] = nm;
    mapping << nv << "->" << nm << endl;
#endif
    record_version(vvs, pv.name(), pv.version());
    return nv;
}



