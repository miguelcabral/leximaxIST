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
/*----------------------------------------------------------------------------*\
 * Version: $Id: globals.hh 73 2007-07-26 15:16:48Z jpms $
 *
 * Author: jpms
 * 
 * Description: Include file that includes all other key include files.
 *              This is the include file to be included by the header
 *              files of libraries and tools.
 *
 *                               Copyright (c) 2005-2006, Joao Marques-Silva
\*----------------------------------------------------------------------------*/

#ifndef _GLOBALS_HH_
#define _GLOBALS_HH_ 1

#include <iostream>
#include <sstream>
#include <fstream>
#include <cstddef>
#include <cstdlib>
#include <cassert>
#include <climits>

using namespace std;


/*----------------------------------------------------------------------------*\
 * System configuration
\*----------------------------------------------------------------------------*/

#include "config.hh"


/*----------------------------------------------------------------------------*\
 * Macro definition
\*----------------------------------------------------------------------------*/

#include "macros.hh"


/*----------------------------------------------------------------------------*\
 * Utils for printing debug information
\*----------------------------------------------------------------------------*/

#include "dbg_prt.hh"


/*----------------------------------------------------------------------------*\
 * Definition of types used throughout
\*----------------------------------------------------------------------------*/

#include "basic_types.h"
#include "types.hh"


/*----------------------------------------------------------------------------*\
 * Definition of functors used throughout
\*----------------------------------------------------------------------------*/

#include "functors.hh"


/*----------------------------------------------------------------------------*\
 * Utils for terminating program execution
\*----------------------------------------------------------------------------*/

#include "err_utils.hh"


/*----------------------------------------------------------------------------*\
 * Utils for printing resourse usage information
\*----------------------------------------------------------------------------*/

#include "rusage.hh"


#endif /* _GLOBALS_HH_ */

/*----------------------------------------------------------------------------*/
