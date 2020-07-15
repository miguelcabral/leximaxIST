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
/* Copyright (C) 2011, Mikolas Janota */
#include "EncoderTypes.hh"
#include "collections.hh"
void record_version (PackageToVersions& vvs, const string& name, Version version)
{
   PackageToVersions::iterator i=vvs.find(name);
   VersionVector* vs;
   if (i == vvs.end ()) {
       vs=new VersionVector();       
       vvs [name]=vs;
   } else {
       vs=i->second;
   }
   vs->push_back(version);
}

void record_version (FeatureToVersions& vvs, const string& name, Version version)
{
   FeatureToVersions::iterator i=vvs.find(name);
   VersionSet* vs;
   if (i == vvs.end ()) {
       vs=new VersionSet();       
       vvs[name]=vs;
   } else {
       vs=i->second;
   }
  vs->insert(version);
}


void record_unit(FeatureToUnits& ftu, CONSTANT PackageVersion& feature, InstallableUnit *unit)
{
        FeatureToUnits::iterator fi=ftu.find(feature);
        UnitVector *vector;
        if (fi==ftu.end())
        {
            vector = new UnitVector ();
            ftu[feature]=vector;
        } else {
            vector = fi->second;
        }
        vector->push_back(unit);
}

void record_unit (PackageUnits& pu, InstallableUnit *unit)
{
    CONSTANT string &package_name=unit->name;
    PackageUnits::const_iterator i = pu.find (package_name);
    UnitVector *vector;
    if (i!=pu.end()) vector = i->second;
    else {
        vector = new UnitVector();
        pu[package_name]=vector;
    }
    vector->push_back(unit);
}

