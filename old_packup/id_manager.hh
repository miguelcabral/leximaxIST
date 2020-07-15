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
 * Version: $Id: id_manager.hh 73 2007-07-26 15:16:48Z jpms $
 *
 * Author: jpms
 * 
 * Description: Management of var IDs. Var ids *must* start at k>0.
 *
 *                               Copyright (c) 2005-2006, Joao Marques-Silva
\*----------------------------------------------------------------------------*/

#ifndef _ID_MANAGER_HH_
#define _ID_MANAGER_HH_ 1

#include "globals.hh"


/*----------------------------------------------------------------------------*\
 * Class: IDManager
 *
 * Purpose: Manager of var IDs.
\*----------------------------------------------------------------------------*/

class IDManager {

public:

  IDManager(ULINT _min_id) : id_count(_min_id-1), min_id(_min_id) {
    assert(_min_id>0); }

  IDManager() : id_count(0), min_id(1) { }

  ~IDManager() { }

public:
  inline ULINT new_id() { return ++id_count; }

  inline ULINT new_id(ULINT& id) { id = ++id_count; return 1; }

  inline ULINT new_ids(ULINT num, ULINT& lb, ULINT& ub) {
    lb = id_count + 1;
    ub = id_count + num;
    id_count += num;
    return 1;
  }

  inline void reg_ids(ULINT maxid) {
    if (id_count < maxid) { id_count = maxid; }
  }

  inline bool exists(ULINT id) { return id <= id_count && id >= min_id; }

  inline ULINT top_id() { return id_count; }

  inline ULINT range() { return id_count - min_id + 1; }

  inline void clear() { id_count = min_id-1; }

  void dump(ostream& outs=cout) {
    outs << "ID count: " << (id_count-min_id+1) << endl; }

  friend ostream & operator << (ostream& outs, IDManager& idmgr) {
    idmgr.dump(outs); return outs; }

private:
  ULINT id_count;    // number of ids used
  ULINT min_id;      // min id used

};

#endif /* _ID_MANAGER_HH_ */

/*----------------------------------------------------------------------------*/
