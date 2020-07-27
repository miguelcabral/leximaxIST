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
 * Version: $Id: types.hh 73 2007-07-26 15:16:48Z jpms $
 *
 * Author: jpms
 * 
 * Description: Type definitions used throughout.
 *
 *                               Copyright (c) 2005-2006, Joao Marques-Silva
\*----------------------------------------------------------------------------*/

#ifndef _TYPES_HH_
#define _TYPES_HH_ 1

#include <iostream>
#include <cstddef>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cmath>

#include "basic_types.h"
 
using namespace std;


/*----------------------------------------------------------------------------*\
 * Values besides 0 and 1
\*----------------------------------------------------------------------------*/

typedef enum logic_values {
  NoValue = 2
} LogicValues;

#define NONE -1


/*----------------------------------------------------------------------------*\
 * Functor utilities
\*----------------------------------------------------------------------------*/

class AbsLitLess {
public:
  bool operator()(LINT a, LINT b) const { return fabs(a) < fabs(b); }
};

class AbsLitGreater {
public:
  bool operator()(LINT a, LINT b) const { return fabs(a) > fabs(b); }
};

class PtrLess {
public:
  bool operator()(const void* pa, const void* pb) const {
    return (ULINT)pa < (ULINT)pb; }
};


/*----------------------------------------------------------------------------*\
 * Additional functors to be used in the definition of hash sets and maps
\*----------------------------------------------------------------------------*/

class PtrHash {
public:
  ULINT operator()(const void* ptr) const { return (ULINT)ptr; }
};
 
class PtrEqual {
public:
  bool operator()(const void* ptr1, const void* ptr2) const {
    return ptr1 == ptr2;
  }
};

 
class IntHash {
public:
  LINT operator()(LINT val) const { return val; }
};
 
class IntEqual {
public:
  bool operator()(LINT v1, LINT v2) const { return v1 == v2; }
};
 
class ULIntHash {
public:
  ULINT operator()(ULINT val) const { return val; }
};
 
class ULIntEqual {
public:
  bool operator()(ULINT v1, ULINT v2) const { return v1 == v2; }
};
 
class XLIntHash {
public:
  LINT operator()(XLINT val) const { return ToLint(val); }
//#ifdef GMPDEF
//    return val.get_si();
//#else
//    return (LINT)val;
//#endif
//  }
};
 
class XLIntEqual {
public:
  bool operator()(XLINT v1, XLINT v2) const { return v1 == v2; }
};
 

class StrHash {
public:
  ULINT operator()(string str) const {
    ULINT res = 0;
    string::const_iterator p = str.begin();
    string::const_iterator end = str.end();
    while (p!=end) { res = (res<<1) ^ *p++; }
    return res;
  }
};


struct eqstr {
  bool operator()(const string s1, const string s2) const {
    return strcmp((char*)(s1.c_str()), (char*)(s2.c_str())) == 0;
  }
};

struct eqint {
  bool operator()(int v1, int v2) const { return v1 == v2; }
};


/*----------------------------------------------------------------------------*\
 * Type definitions for maps and sets used throughout, most of which will be
 * based on int's and ptr's
\*----------------------------------------------------------------------------*/

#include <vector>
#include <list>
#include <map>
#include <set>
/* #include <ext/slist>          // Location of STL list extensions */
#include <unordered_map>       // Location of STL hash extensions
#include <unordered_set>       // Location of STL hash extensions

using namespace std;
/* using namespace __gnu_cxx;    // Required for STL hash extensions */


#ifdef USE_RBTREE_SETS

template <class T>
class StdSet : public set<T> {
public:
  void dump(ostream& outs=std::cout) {
    typename StdSet<T>::iterator npos = this->begin();
    typename StdSet<T>::iterator nend = this->end();
    for (; npos != nend; ++npos) { outs << *npos << " "; }
  }
  friend ostream & operator << (ostream& outs, StdSet<T>& ss) {
    ss.dump(outs); return outs; }
  inline bool exists(T v) { return this->find(v) != this->end(); }
};

typedef StdSet<int> IntSet;

template <class K>    // K should be pointer type
class RefSet : public StdSet<K> { };

template <class TK, class TV>
class StdMap : public multimap<TK,TV> {
public:
  inline bool exists(TK k) const { return this->find(k) != this->end(); }
  inline bool exists(TK k, TV v) const {
      const auto pp = equal_range(k);
      auto ppos = pp.first;
      const auto pend = pp.second;
      for(; ppos != pend; ++ppos) {
          if (ppos->second == v)
              return true;
      }
      return false;
  }
  inline void insert(pair<TK,TV> v) { multimap<TK,TV>::insert(v); }
  inline void insert(TK v1,TV v2) { this->insert(make_pair(v1,v2)); }
  inline TV lookup(TK k) const { assert(exists(k)); return this->find(k)->second; }
  void dump(ostream& outs=std::cout) {
    typename StdMap<TK,TV>::iterator npos = this->begin();
    typename StdMap<TK,TV>::iterator nend = this->end();
    for (; npos != nend; ++npos) { outs << npos->second << " "; }
  }
  friend ostream & operator << (ostream& outs, StdMap<TK,TV>& sm) {
    sm.dump(outs); return outs; }
};

template <class T>
class IntKeyMap : public StdMap<int,T> { };

template <class K, class V>    // K should be pointer type
class RefKeyMap : public StdMap<K,V> { };


#else

template <class T, class H, class E>
class StdSet : public unordered_set<T,H,E> {
public:
  inline bool exists(T v) { return this->find(v) != this->end(); }
  inline void insert(T v) {
    unsigned int sz = this->size();
    if (sz >= 2*this->bucket_count()) {  // Optimizing # resizes
      //DBG(
      cout<<"Resizing a ""T"" set "<<endl;
      //);
      this->resize(8*(sz+(sz==0)?1:0)+1); }
    unordered_set<T,H,E>::insert(v); }
  void dump(ostream& outs=std::cout) {
    typename StdSet<T,H,E>::iterator npos = this->begin();
    typename StdSet<T,H,E>::iterator nend = this->end();
    for (; npos != nend; ++npos) { outs << *npos << " "; }
  }
  friend ostream & operator << (ostream& outs, StdSet<T,H,E>& ss) {
    ss.dump(outs); return outs; }
};

typedef StdSet<int,IntHash,IntEqual> IntSet;

template <class K>    // K should be pointer type
class RefSet : public StdSet<K,PtrHash,PtrEqual> { };

template <class TK, class TV, class H, class E>
class StdMap : public unordered_multimap<TK,TV,H,E> {
public:
  inline bool exists(TK k) { return this->find(k) != this->end(); }
  inline bool exists(TK k, TV v) {
    pair<typename StdMap<TK,TV,H,E>::iterator,
      typename StdMap<TK,TV,H,E>::iterator> pp = this->equal_range(k);
    typename StdMap<TK,TV,H,E>::iterator ppos = pp.first;
    typename StdMap<TK,TV,H,E>::iterator pend = pp.second;
    for(; ppos != pend; ++ppos) { if (ppos->second == v) { return true; } }
    return false; }
  inline void insert(pair<TK,TV> v) {
    unsigned int sz = this->size();
    if (sz >= 2*this->bucket_count()) {  // Optimizing # resizes
      //DBG(
      cout<<"Resizing a ""TK"" map "<<endl;
      //);
      this->resize(8*(sz+(sz==0)?1:0)+1); }
    unordered_multimap<TK,TV,H,E>::insert(v); }
  inline void insert(TK v1, TV v2) {
    this->insert(make_pair(v1,v2)); }
  inline TV lookup(TK k) { assert(exists(k)); return find(k)->second; }
  void dump(ostream& outs=std::cout) {
    typename StdMap<TK,TV,H,E>::iterator npos = this->begin();
    typename StdMap<TK,TV,H,E>::iterator nend = this->end();
    for (; npos != nend; ++npos) {
      outs << npos->first << "->" << npos->second << endl; }
  }
  friend ostream & operator << (ostream& outs, StdMap<TK,TV,H,E>& sm) {
    sm.dump(outs); return outs; }
};

template <class T>
class IntKeyMap : public StdMap<int,T,IntHash,IntEqual> { };

template <class K, class V>    // K should be pointer type
class RefKeyMap : public StdMap<K,V,PtrHash,PtrEqual> { };

#endif

typedef vector<bool> BoolVector;

//typedef IntKeyMap<int> OptInt2IntMap;        // Int 2 Int map
//typedef IntKeyMap<void*> Int2RefMap;      // Int 2 Ref map

//template <class K>
//class Ref2IntMap : public RefKeyMap<K,int> { };

typedef unordered_map<LINT,LINT,IntHash,IntEqual> Int2IntMap;

typedef unordered_map<XLINT,XLINT,XLIntHash,XLIntEqual> XLInt2XLIntMap;

typedef unordered_map<XLINT,LINT,XLIntHash,XLIntEqual> XLInt2IntMap;

//typedef map<LINT,LINT,IntHash,IntEqual> XLInt2IntMap;
//typedef map<string,LINT> XLStr2IntMap;

typedef unordered_set<ULINT,ULIntHash,ULIntEqual> ULINTSet;

typedef unordered_set<LINT,IntHash,IntEqual> LINTSet;

typedef vector<ULINT> UIntVector;

typedef vector<LINT> IntVector;

typedef vector<XLINT> XLIntVector;

typedef unordered_map<LINT,vector<LINT>*,IntHash,IntEqual> Int2IntVMap;

typedef unordered_map<LINT,bool,IntHash,IntEqual> Int2BoolMap;

typedef Int2IntMap IDMap;                 // Std map of ID's
typedef IntSet IDSet;                     // Std set of ID's

template <typename ClRef>
struct HashedSet { typedef unordered_set<ClRef,PtrHash,PtrEqual> Type; };

template <typename ClRef>
struct Ref2RefPMap { typedef unordered_map<ClRef,ClRef*,PtrHash,PtrEqual> Type; };

template <typename ClRef>
struct Ref2IntMap { typedef unordered_map<ClRef,LINT,PtrHash,PtrEqual> Type; };

template <typename ClRef>
struct Ref2BoolMap { typedef unordered_map<ClRef,bool,PtrHash,PtrEqual> Type; };

template <typename ClRef>
struct Int2RefMap { typedef unordered_map<LINT,ClRef,IntHash,IntEqual> Type; };


//jpms:bc
/*----------------------------------------------------------------------------*\
 * Extras for using GMP
\*----------------------------------------------------------------------------*/
//jpms:ec

namespace gmpcmp {

#ifdef GMPDEF
#include <gmpxx.h>

template <typename T, typename U>
bool le(T a, U b) { return cmp(a, b) <= 0; }

template <typename T, typename U>
bool lt(T a, U b) { return cmp(a, b) < 0; }

template <typename T, typename U>
bool ge(T a, U b) { return cmp(a, b) >= 0; }

template <typename T, typename U>
bool gt(T a, U b) { return cmp(a, b) > 0; }

template <typename T, typename U>
bool eq(T a, U b) { return cmp(a, b) == 0; }

template <typename T, typename U>
bool ne(T a, U b) { return cmp(a, b) != 0; }

#else

template <typename T, typename U>
bool le(T a, U b) { return a <= b; }

template <typename T, typename U>
bool lt(T a, U b) { return a < b; }

template <typename T, typename U>
bool ge(T a, U b) { return a >= b; }

template <typename T, typename U>
bool gt(T a, U b) { return a > b; }

template <typename T, typename U>
bool eq(T a, U b) { return a == b; }

template <typename T, typename U>
bool ne(T a, U b) { return a != b; }

#endif

}

#endif /* _TYPES_HH_ */

/*----------------------------------------------------------------------------*/
