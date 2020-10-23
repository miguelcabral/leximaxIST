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
 * File:   collections.hh
 * Author: mikolas
 *
 * Created on September 19, 2010, 3:04 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef COLLECTIONS_HH
#define	COLLECTIONS_HH
#include "common_types.hh"
#include "PackageVersions.hh"
#include "package_version.hh"

typedef unordered_map<Variable, string> VariableToName;

typedef vector<PackageVersions> VersionsList;
typedef vector<PackageVersions>  PackageVersionsList;
typedef vector<PackageVersionsList*> PackageVersionsCNF;
typedef vector<PackageVersion> PackageVersionList;
typedef unordered_map<string, string> Str2Str;
/*typedef unordered_map<const char*, string, strhash, streq > Str2Str;*/
typedef unordered_map<PackageVersion, Variable, hash_package_version, eq_package_version> PackageVersionMap;
typedef unordered_map<Variable,PackageVersion> VariableToPackageVersion;
typedef vector<Version> VersionVector;

typedef vector<LINT> LiteralVector;
typedef unordered_set<Version> VersionSet;
typedef unordered_set<Variable>  VariableSet;
typedef unordered_map<string, VersionVector*> PackageToVersions;
typedef unordered_map<string, Version> PackageToVersion;
typedef unordered_map<PackageVersion, PackageVersionList*, hash_package_version, eq_package_version> FeaturesToPackages;
typedef unordered_map<PackageVersion, PackageVersionsList*, hash_package_version, eq_package_version> PackageToFeatureVersions;
typedef unordered_set<PackageVersion, hash_package_version, eq_package_version> PackageVersionSet;
typedef unordered_set<PackageVersions, hash_package_versions, eq_package_versions> PackageVersionsSet;
typedef unordered_map<string, VersionSet*> FeatureToVersions;
typedef unordered_set<string> StringSet;

namespace collection_printing {
void print(CONSTANT PackageVersionsList& pvl,ostream& out);
void print(CONSTANT PackageVersionList& pvl, ostream& out);
void print(CONSTANT PackageVersionsCNF& pvs_cnf, ostream& out);
}
#endif	/* COLLECTIONS_HH */

