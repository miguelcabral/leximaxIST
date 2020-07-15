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
 * File:        err_utils.hh
 *
 * Description: Utilities to handle errors.
 *
 * Author:      jpms
 * 
 *                                     Copyright (c) 2010, Joao Marques-Silva
\*----------------------------------------------------------------------------*/
//jpms:ec

#ifndef _ERR_UTILS_H
#define _ERR_UTILS_H 1

#include <iostream>

using namespace std;

template <class S>
void tool_abort(S msg) {
  std::cout << "ERROR: " << msg << std::endl; std::cout.flush(); exit(5); }

template <class S>
void tool_warn(S msg) {
  std::cout << "WARN: " << msg << std::endl; std::cout.flush(); }

template <class S>
void tool_info(S msg) {
  std::cout << "INFO: " << msg << std::endl; std::cout.flush(); }

#endif /* _ERR_UTILS_H */

/*----------------------------------------------------------------------------*/
