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
 * File:   common_types.hh
 * Author: mikolas
 *
 * Created on August 23, 2010, 8:53 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef COMMON_TYPES_H
#define	COMMON_TYPES_H

#include <unordered_set>
#include <unordered_map>
#include <string.h>
#include "types.hh"
using std::string;
using std::vector;
using std::ostream;

#define CNF_OUT(c)
#define CONSTANT const
#define CONTAINS(s,e) ( ((s).find(e))!=(s).end() )
#define SAME_PACKAGE_NAME(n1,n2) ((n1) == (n2))
#define unordered_set unordered_set
#define unordered_map unordered_map

typedef unsigned int UINT;
typedef UINT         Version;
typedef set< XLINT, greater<XLINT> >  WeightSet;

#ifdef CONV_DBG
#define MAPPING 
#undef NDEBUG
#define OUT(c) { c }
#define __PL (cerr << __FILE__ << ":" << __LINE__ << endl).flush();
#else
#define OUT(c)
#define __PL
#endif

#define FOR_EACH(iterator_type,index,iterated)\
  for (iterator_type index = (iterated).begin(); index != (iterated).end();++index)

#define FOR_EACH_REV(iterator_type,index,iterated)\
  for (iterator_type index = (iterated).rbegin(); index != (iterated).rend();++index)


enum Criterion
{
    TRENDY,
    PARANOID,
    NO_OPTIMIZATION
};

enum OBJECTIVE_FUNCTION
{
    COUNT_NEW,
    COUNT_UNMET_RECOMMENDS,
    COUNT_REMOVED,
    COUNT_NOT_UP_TO_DATE,
    COUNT_CHANGED
};

typedef pair<OBJECTIVE_FUNCTION, bool> Objective;
const string to_string(const Objective& o);
const char* to_string(OBJECTIVE_FUNCTION f);
void print(const vector<Objective>& ls, ostream& o);

namespace version_operators {
enum Operator
{
    VERSIONS_NONE,
    VERSIONS_EQUALS,
    VERSIONS_NOT_EQUALS,
    VERSIONS_GREATER_EQUALS,
    VERSIONS_GREATER,
    VERSIONS_LESS_EQUALS,
    VERSIONS_LESS
};
bool evaluate (Version version_1, Operator op, Version version_2);
const char* to_string (Operator op);
} /*end of namespace version_operators */

enum KeepValue
{
 KEEP_NONE,
 KEEP_VERSION,
 KEEP_PACKAGE,
 KEEP_FEATURE
};

typedef UINT Variable;

struct streq {
  bool operator()(const char* s1, const char* s2) const
  {return strcmp(s1, s2) == 0;}
};

const char* to_string (KeepValue value);
const char* to_string (Criterion value);
inline LINT neg(Variable v) { return -((LINT)v); }
#endif	/* COMMON_TYPES_H */

