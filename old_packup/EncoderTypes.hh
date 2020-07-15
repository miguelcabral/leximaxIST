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
 * File:   EncoderTypes.hh
 * Author: mikolas
 *
 * Created on September 26, 2010, 7:35 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef ENCODERTYPES_HH
#define	ENCODERTYPES_HH
#include "collections.hh"
#include "InstallableUnit.hh"
typedef vector <InstallableUnit*> UnitVector;
typedef vector <CONSTANT InstallableUnit*> UnitConstantVector;
typedef unordered_map<string, UnitVector*> PackageUnits;
typedef unordered_map<string, UnitConstantVector*> PackageConstantUnits;
typedef unordered_map<PackageVersion, UnitVector*, hash_package_version, eq_package_version> FeatureToUnits;

void record_version (PackageToVersions& vvs, const string& name, Version version);
void record_version (FeatureToVersions& vvs, const string& name, Version version);
void record_unit(FeatureToUnits& ftu, CONSTANT PackageVersion& feature, InstallableUnit *unit);
void record_unit (PackageUnits& pu, InstallableUnit *unit);
#endif	/* ENCODERTYPES_HH */

