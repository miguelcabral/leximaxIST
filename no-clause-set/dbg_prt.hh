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
 * Version: $Id: dbg_prt.hh 73 2007-07-26 15:16:48Z jpms $
 *
 * Author: jpms
 * 
 * Description: Utilities for printing containers.
 *
 *                               Copyright (c) 2005-2006, Joao Marques-Silva
\*----------------------------------------------------------------------------*/

#ifndef _DBG_PRT_HH_
#define _DBG_PRT_HH_ 1

#include <iostream>

template <class T>
inline void PRINT_ELEMENTS (const T& coll,
			    const char* optcstr="", const char* sepstr=" ")
{
  typename T::const_iterator pos;
  std::cout << optcstr;

  for(pos=coll.begin(); pos!=coll.end(); ++pos) {
    std::cout << *pos << sepstr;
  }
  std::cout << std::endl;
}

template <class T>
inline void PACK_PRINT_ELEMENTS (const T& coll, const char* optcstr="")
{
  typename T::const_iterator pos;
  std::cout << optcstr;

  for(pos=coll.begin(); pos!=coll.end(); ++pos) {
    std::cout << *pos;    //((bool)*pos)?1:0;  // -> This only works for bool's
  }
  std::cout << std::endl;
}

template <class T>
inline void PRINT_PTR_ELEMENTS (const T& coll,
				const char* optcstr="", const char* sepstr=" ")
{
  typename T::const_iterator pos;
  std::cout << optcstr;

  for(pos=coll.begin(); pos!=coll.end(); ++pos) {
    std::cout << **pos << sepstr;
  }
  std::cout << std::endl;
}

#endif /* _DBG_PRT_HH_ */

/*----------------------------------------------------------------------------*/
