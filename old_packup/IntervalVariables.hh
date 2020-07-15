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
 * File:   IntervalVariables.hh
 * Author: mikolas
 *
 * Created on September 26, 2010, 7:41 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef INTERVALVARIABLES_HH
#define	INTERVALVARIABLES_HH

#include "collections.hh"
#include "id_manager.hh"
#include <iostream>

using std::ofstream;

class IntervalVariables {
public:
    IntervalVariables(IDManager& idm
#ifdef MAPPING 
    , unordered_map<Variable, string>& variable_names,
      ofstream& mapping
#endif
    ) : id_manager (idm)
#ifdef MAPPING 
      , variable_names(variable_names), mapping(mapping)
#endif
    {}

    bool _dummy;
    Variable get_require_up(const PackageVersion& pv)
    {return  get_variable(pv, require_package_versions_up,   require_up, "require_up", _dummy);}
    Variable get_require_down(const PackageVersion& pv)
    {return  get_variable(pv, require_package_versions_down, require_down, "require_down", _dummy);}
    Variable get_disable_up(const PackageVersion& pv)
    {return  get_variable(pv, disable_package_versions_up,   disable_up, "disable_up", _dummy);}
    Variable get_disable_down(const PackageVersion& pv)
    {return  get_variable(pv, disable_package_versions_down, disable_down, "disable_down", _dummy);}

    inline PackageToVersions get_require_up () const {  return require_up; } 
    inline PackageToVersions get_require_down () const {  return require_down; } 
    inline PackageToVersions get_disable_up () const {  return disable_up; } 
    inline PackageToVersions get_disable_down () const {  return disable_down; } 

private:
    IDManager& id_manager;
#ifdef MAPPING 
    unordered_map<Variable, string>& variable_names;// Names of variables for debugging purposes.
    ofstream& mapping;
#endif

    // Maps from intervals to the corresponding variables
    PackageVersionMap require_package_versions_up,require_package_versions_down;
    PackageVersionMap disable_package_versions_up,disable_package_versions_down;

    // For each package the following maps record the version intervals used in the dependencies
    PackageToVersions require_up;
    PackageToVersions require_down;
    PackageToVersions disable_up;
    PackageToVersions disable_down;

    inline Variable new_variable () { return id_manager.new_id(); }
    /**
     * Get a variable corresponding to the given pair package-version.
     * @param pv package-version pair
     * @param map a map storing variable IDs for package-version pairs
     * @return Variable corresponding to {@code pv}  in {@code map}.
     *         A fresh variable, if {@code pv}  is not in {@code map}.
     */
    Variable get_variable (const PackageVersion& pv, PackageVersionMap& map,
                           unordered_map <string,VersionVector*>& vvs,
                           const string& map_name, bool& is_fresh);


};

#endif	/* INTERVALVARIABLES_HH */

