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
 * Version: $Id: macros.hh 73 2007-07-26 15:16:48Z jpms $
 *
 * Author: jpms
 * 
 * Description: Macro definitions
 *
 *                                    Copyright (c) 2006, Joao Marques-Silva
\*----------------------------------------------------------------------------*/

#ifndef _MACROS_HH_
#define _MACROS_HH_ 1


/*----------------------------------------------------------------------------*\
 * Utility macros (minof, maxof, ...)
\*----------------------------------------------------------------------------*/

#define minof(x,y) ((x)<(y))?(x):(y)
#define maxof(x,y) ((x)>(y))?(x):(y)


/*----------------------------------------------------------------------------*\
 * Swap utility
\*----------------------------------------------------------------------------*/

// Swap utility
#define tswap(T, x, y) { T tmp = y; y = x; x = tmp; }

template <typename T>
string convert(T val)
{
   stringstream ss;  // Create a stringstream
   ss << val;        // Add value to the stream
   return ss.str();  // Return string with the contents of the stream
}

/*----------------------------------------------------------------------------*\
 * Macros for gathering statistics about msu algorithms
\*----------------------------------------------------------------------------*/
#ifdef STATISTICS
#define STATS(x) x
#else
#define STATS(x)
#endif

/*----------------------------------------------------------------------------*\
 * Macros for printing generic debug info anf for controlling debug actions
 * (Some parts are based on examples from Eckel's book)
\*----------------------------------------------------------------------------*/

#ifndef DBGMACROS
#define DBGMACROS
#define NDBG(x)
#define NDBGPRT(x)
#ifdef FULLDEBUG
#define DBG(x) x
#define CDBG(v,x) if(v,=TRACEVERB) { x }
#define DBGPRT(x) std::cout << x << std::endl; std::cout.flush()
#define DEBUG(x) std::cout << #x " = " << x << std::endl; std::cout.flush()
#define TRACEP(x) std::cout << x << std::endl;
#define TRACEX(x) std::cout << #x << std::endl; x
#else
#define DBG(x)
#define CDBG(v,x)
#define DBGPRT(x)
#define DEBUG(x)
#define TRACEP(x)
#define TRACEX(x) x
#endif
#ifndef NCHECK
#define CHK(x) x
#define CCHK(v,x) if(v<=TRACEVERB) { x }
#define CHKPRT(x) std::cout<<#x<<((x)?" passes":" fails")<<std::endl;std::cout.flush()
#define CHKPRT2(x) std::cout << x << std::endl; std::cout.flush()
#else
#define CHK(x)
#define CCHK(v,x) 
#define CHKPRT(x)
#define CHKPRT2(x)
#endif
#endif


/*----------------------------------------------------------------------------*\
 * A more versatile (and verbose) assert macro; also a way to use assert code
\*----------------------------------------------------------------------------*/

#ifndef NDEBUG
#define verbassert(cond, message) if(!(cond)) {cerr<<message<<endl; assert(0);}
#define ASSERTCODE(x) x
#else
#define verbassert(cond, message) 
#define ASSERTCODE(x)
#endif

#define CHKCODE(x)  ASSERTCODE(x)


/*----------------------------------------------------------------------------*\
 * Macros for memory management
\*----------------------------------------------------------------------------*/

#define NEW(t,p) ((p) = (t*)malloc((long)sizeof *(p)))
#define NEWV(t,n,p) ((p) = (t*)malloc((long)(n*sizeof(*(p)))))
#define DEL(p) free(p);


/*----------------------------------------------------------------------------*\
 * Logging utilities
\*----------------------------------------------------------------------------*/

//#define report(x) 

#define read_cputime() RUSAGE::read_cpu_time_self()

#define prt_std_cputime(x,y) \
  std::cout<<x<<y<<"CPU Time: "<<read_cputime()<<endl

#define prt_cfg_cputime(y) prt_std_cputime(config.get_prefix(),y)

#endif /* _MACROS_HH_ */

/*----------------------------------------------------------------------------*/
