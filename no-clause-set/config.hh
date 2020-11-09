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
 * Version: $Id: config.hh 73 2007-07-26 15:16:48Z jpms $
 *
 * Author: jpms
 * 
 * Description: Defines used throughout
 *
 *                                    Copyright (c) 2006, Joao Marques-Silva
\*----------------------------------------------------------------------------*/

#ifndef _CONFIG_HH_
#define _CONFIG_HH_ 1


/*----------------------------------------------------------------------------*\
 * Data structure fine-tuning configuration
\*----------------------------------------------------------------------------*/

//#define NO_NOTOS_OPT 1   // **Uncomment** to use std map structures

#ifdef NO_NOTOS_OPT    // Define if goal is *not* to optimize data structures
#define USE_RBTREE_MAPS 1  // Use std RBTree-based maps (instead of hash maps)
#define USE_RBTREE_SETS 1  // Use std RBTree-based sets (instead of hash sets)
#else
#undef USE_RBTREE_MAPS     // Use (optimized and so preferred) hash maps
#undef USE_RBTREE_SETS     // Use (optimized and so preferred) hash sets
#endif

#define PRINTOUT 1         // **Comment** to prevent printing of msgs to stdout

#define LOGGING 1          // **Comment** to prevent logging to file

//#define RBC_NOCOLLAPSE 1 // **Uncomment** to prevent node collapsing in RBCs

#endif /* _CONFIG_HH_ */

/*----------------------------------------------------------------------------*/
