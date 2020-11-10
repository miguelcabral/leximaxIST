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
 * File:        basic_types.h
 *	--> Edit: Miguel Cabral 2020 - File: leximax_types.h
 * Description: Basic types used by BOLT.
 *	--> Edit: Miguel Cabral 2020 - Descr.: integer types, sorting network
 *	and clauses used in leximax library.
 * Author:      jpms, Miguel Cabral (2020)
 * 
 * Revision:    $Id$.
 *
 *                                     Copyright (c) 2009, Joao Marques-Silva
\*----------------------------------------------------------------------------*/
//jpms:ec

#ifndef LEXIMAX_TYPES
#define LEXIMAX_TYPES


/*----------------------------------------------------------------------------*\
 * Values besides 0 and 1
\*----------------------------------------------------------------------------*/

#ifdef __LP64__
typedef unsigned long long int ULINT;
typedef long long int LINT;
#define MAXLINT LLONG_MAX;
#define MINLINT LLONG_MIN;
#define MAXULINT ULLONG_MAX;
#else
typedef unsigned long int ULINT;
typedef long int LINT;
#define MAXLINT LONG_MAX;
#define MINLINT LONG_MIN;
#define MAXULINT ULONG_MAX;
#endif

#ifdef GMPDEF
#include <gmpxx.h>
typedef mpz_class XLINT;
#define ToLint(x) x.get_si()
#else
typedef LINT XLINT;
#define ToLint(x) (LINT)x
#endif

// sorting network and clauses:
typedef std::vector<std::pair<LINT, LINT>*> SNET;
typedef std::vector<LINT> Clause;

#endif /* LEXIMAX_TYPES */

/*----------------------------------------------------------------------------*/
