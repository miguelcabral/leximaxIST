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
//jpms:bc
/*----------------------------------------------------------------------------*\
 * File:        cl_globals.hh
 *
 * Description: Complete definitions for clauses and clause sets.
 *
 * Author:      jpms
 * 
 * Revision:    $Id$.
 *
 *                                     Copyright (c) 2009, Joao Marques-Silva
\*----------------------------------------------------------------------------*/
//jpms:ec

#ifndef _CL_GLOBALS_H
#define _CL_GLOBALS_H 1

#include <cmath>
#include <vector>

#include <unordered_map>       // Location of STL hash extensions
#include <unordered_set>       // Location of STL hash extensions

using namespace std;
using namespace __gnu_cxx;    // Required for STL hash extensions

#include "globals.hh"
#include "basic_clause.hh"
#include "cl_functors.hh"
#include "cl_types.hh"
#include "cl_registry.hh"

#endif /* _CL_GLOBALS_H */

/*----------------------------------------------------------------------------*/
