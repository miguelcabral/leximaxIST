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
 * File:   ConverterMem.cc
 * Author: mikolas
 * 
 * Created on September 25, 2010, 5:46 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#include "ConverterMem.hh"

ConverterMem::~ConverterMem() {}

#ifdef MAPPING 
void ConverterMem::set_maping_file (const char* filen) {encoder.get_mapping().open(filen);}
#endif

void ConverterMem::solution_check(const char* file_name) {
    sr.read(file_name);
    encoder.cs = true;
    encoder.stc=&(sr.installed_package_versions);
}

void ConverterMem::start_package() {
    current_unit = new InstallableUnit();
    current_unit->name=read_package_name;

    // default values for a unit
    current_unit->keep=KEEP_NONE;
    current_unit->installed=false;

    has_version = false;    
}

void ConverterMem::close_package()
{
//    current_unit->dump(cerr);
//    cerr << endl;
    encoder.add_unit(current_unit);
}

void ConverterMem::close_universe(){}

void ConverterMem::close_input () { solve(); }

void ConverterMem::package_provides()
{
    PackageVersionList &current_provides=current_unit->provides;
    current_provides.insert(current_provides.end(), read_feature_list.begin (), read_feature_list.end());
}

void ConverterMem::package_conflicts()
{
    PackageVersionsList& current_conflicts = current_unit->conflicts;
    current_conflicts.insert (current_conflicts.end(), read_package_versions_list.begin(), read_package_versions_list.end());
}

void ConverterMem::package_depends()
{
    PackageVersionsCNF &current_depends = current_unit->depends_cnf;
    current_depends.insert(current_depends.end(), read_CNF.begin(),read_CNF.end());
}

void ConverterMem::package_recommends()
{
    PackageVersionsCNF &current_recommends = current_unit->recommends_cnf;
    current_recommends.insert(current_recommends.end(), read_CNF.begin(),read_CNF.end());
}

void ConverterMem::package_keep() { current_unit->keep=read_keep_value; }

void ConverterMem::package_installed(bool installed_value)
{current_unit->installed=installed_value;}

void ConverterMem::end_processed_package_version ()
{
    current_unit->version = read_version;
    has_version = true;
}

void ConverterMem::request_install()
{
    encoder.add_install(read_package_versions_list);
}

void ConverterMem::request_remove()
{
    encoder.add_remove(read_package_versions_list);
}
void ConverterMem::upgrade()
{
    encoder.add_upgrade(read_package_versions_list);
}
Encoder& ConverterMem::get_encoder() {
    return encoder;
}
