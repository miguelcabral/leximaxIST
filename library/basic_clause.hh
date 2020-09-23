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
 * File:        basic_clause.hh
 *
 * Description: A basic clause is a vector of literals.
 *
 * Author:      jpms
 * 
 * Revision:    $Id$.
 *
 *                                     Copyright (c) 2009, Joao Marques-Silva
\*----------------------------------------------------------------------------*/
//jpms:ec

#ifndef _BASIC_CLAUSE_H
#define _BASIC_CLAUSE_H 1

#include<algorithm>
#include "globals.hh"


//jpms:bc
/*----------------------------------------------------------------------------*\
 * Basic types for class BasicClause.
\*----------------------------------------------------------------------------*/
//jpms:ec

typedef vector<LINT> LitVector;
typedef vector<LINT>::iterator Literator;
typedef vector<LINT>::reverse_iterator RLiterator;


//jpms:bc
/*----------------------------------------------------------------------------*\
 * Class: BasicClause
 *
 * Purpose: New organization of clauses, to be used in future tools
\*----------------------------------------------------------------------------*/
//jpms:ec

class BasicClause {
  friend class ClauseRegistry;
  friend class BasicClauseSet;
protected:

  explicit BasicClause(const vector<LINT>& lits) : clits(lits), weight(0), id(0), grp_id(0) {
    assert(adjacent_find(lits.begin(),lits.end(),AbsLitGreater())==lits.end());
    //sort_lits();
    //N
    DBG(cout << "Creating clause: [" << *this << "]" << endl;);
  }

  //BasicClause(int nlits, const long int lits[]) :
  //clits(lits, lits+nlits), weight(0) {
  //sort_lits();
  ////N
  //DBG(cout << "Creating clause: " << *this << endl;);
  //}

  virtual ~BasicClause() { clits.clear(); }

public:

  /* ULINT size() { return clits.size(); } */
  size_t size() { return clits.size(); }

  Literator begin() { return clits.begin(); }

  Literator end() { return clits.end(); }

protected:

  void add_lit(LINT lit) {
    clits.push_back(lit);    // Currently sorts; it is simpler to insert & shift
    assert(clits.size() > 0);
    if (abs(lit) < abs(clits[clits.size()-1])) {
      sort_lits();    // alternatively: 
      /*  => Seems to be less efficient
	  Literator pos2 = clits.end();
	  Literator pos1 = clits.end();
	  --pos1;
	  for (--pos1; abs(*pos1) > abs(lit); --pos1, --pos2) {
	  *pos2 = *pos1;
	  if (pos1 == clits.begin()) { break; }
	  }
	  *pos1 = lit;
	  */
      //update_internal_data((unsigned)lit);
    }
  }

  void del_lit(LINT lit) {
    for (Literator pos = clits.begin(); pos != clits.end(); ++pos) {
      if (*pos == lit) { *pos = clits.back(); break; }
    }
    clits.pop_back();    // Currently sorts; it is simpler to insert & shift
    //sort_lits();  // alternatively: 
    //update_internal_data((unsigned)lit);   // hash XOR removes bits due to lit
  }

  vector<LINT>& cl_lits() { return clits; }

  //ULINT clhash() { return hashval; }

  ULINT get_min_lit() { ULINT minid=abs(clits[0]); return minid; }

  ULINT get_max_lit() { ULINT maxid=abs(clits[clits.size()-1]); return maxid; }

public:    // Weight/group/id functions

  void set_weight(XLINT nweight) { weight = nweight; }

  XLINT get_weight() { return weight; }

  void set_grp_id(ULINT grp) { grp_id = grp; } // ANTON: store it properly

  ULINT get_grp_id() { return grp_id; }        // ANTON: store it properly

  void set_id(ULINT nid) { id = nid; }

  ULINT get_id() { return id; }

public:    // Comparison functions

  friend bool operator > (BasicClause& ptr1, BasicClause& ptr2) {
    return
      (ptr1.size() > ptr2.size()) ||
      (ptr1.size() == ptr2.size() && ptr1.get_id() > ptr2.get_id());
  }

  friend bool operator < (BasicClause& ptr1, BasicClause& ptr2) {
    return
      (ptr1.size() < ptr2.size()) ||
      (ptr1.size() == ptr2.size() && ptr1.get_id() < ptr2.get_id());
  }

public:    // Output functions

  void dump(ostream& outs=cout) {
    // +ANTON added group ID
    // outs << "[" << get_grp_id() << "] ";
    // -ANTON
    Literator lpos = clits.begin();
    Literator lend = clits.end();
    for (; lpos != lend; ++lpos) {
      outs << *lpos << " ";
    }
    outs << "0";
    //outs << endl;
  }

  friend ostream & operator << (ostream& outs, BasicClause& cl) {
    cl.dump(outs);
    return outs;
  }

protected:

  void sort_lits() {
    //cout << "Lits A: ";
    //copy(clits.begin(), clits.end(), ostream_iterator<int>(cout, " "));
    //cout << endl;
    //if (!is_sorted(clits.begin(), clits.end(), AbsLitLess())) { }
    std::sort(clits.begin(), clits.end(), AbsLitLess());
    //cout << "Lits B: ";
    //copy(clits.begin(), clits.end(), ostream_iterator<int>(cout, " "));
    //cout << endl;
  }

  ULINT compute_hash() {    // Not being used: see below new hash functions
    exit(0);
    ULINT hashv = 0;
    for(vector<LINT>::iterator pos = clits.begin(); pos != clits.end(); ++pos) {
      hashv ^= (*pos>0) ? *pos : -*pos;
    }
     cout << "Hash value: " << hashv << endl; cout.flush();
    return hashv;
  }

  //void update_internal_data(ULINT uval) { sort_lits(); hashval ^= uval; }

protected:
  vector<LINT> clits;
  //ULINT hashval;
  XLINT weight;
  ULINT id;
  ULINT grp_id; // ANTON: the actual group ID, rather than using id
};

#endif /* _BASIC_CLAUSE_H */
/*----------------------------------------------------------------------------*/
