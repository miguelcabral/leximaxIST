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
 * File:   SolutionReader.hh
 * Author: mikolas
 *
 * Created on September 19, 2010, 1:46 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef SOLUTIONREADER_HH
#define	SOLUTIONREADER_HH

#include <fstream>
#include "common_types.hh"
#include "collections.hh"
using std::ifstream;

class SolutionReader
{
public:
    SolutionReader(Str2Str& pns) : _pns(pns) {}
    void read (const char* file_name);
    PackageVersionSet installed_package_versions;
private:
    Str2Str& _pns;
    ifstream file;
};

#endif	/* SOLUTIONREADER_HH */

