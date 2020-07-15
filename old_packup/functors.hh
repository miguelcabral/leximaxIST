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
 * File:        functors.hh
 *
 * Description: 
 *
 * Author:      jpms
 * 
 * Revision:    $Id$.
 *
 *                                     Copyright (c) 2009, Joao Marques-Silva
\*----------------------------------------------------------------------------*/
//jpms:ec

#ifndef _FUNCTORS_H
#define _FUNCTORS_H 1

template<class T> struct defprint : public unary_function<T, void>
{
  defprint(ostream& out) : os(out) {}
  void operator() (T x) { os << x << ' '; }
  ostream& os;
};

template<class T> struct ptrprint : public unary_function<T, void>
{
  ptrprint(ostream& out) : os(out) {}
  void operator() (T x) { os << *x << endl; }
  ostream& os;
};


#endif /* _FUNCTORS_H */

/*----------------------------------------------------------------------------*/
