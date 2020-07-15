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
 * File:   PackageVersionVariables.cc
 * Author: mikolas
 * 
 * Created on September 27, 2010, 2:19 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#include "PackageVersionVariables.hh"

PackageVersionVariables::PackageVersionVariables(
   IDManager& id_manager
#ifdef MAPPING 
   , unordered_map<Variable, string>& variable_names
   , ofstream& mapping
#endif
)
  : id_manager (id_manager)
#ifdef MAPPING 
   , variable_names (variable_names), mapping(mapping)
#endif
{}

PackageVersionVariables::~PackageVersionVariables(){}
