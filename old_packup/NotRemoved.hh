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
 * File:   NotRemoved.hh
 * Author: mikolas
 *
 * Created on October 11, 2010, 6:51 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef NOTREMOVED_HH
#define	NOTREMOVED_HH
#include "EncoderTypes.hh"

class NotRemoved {
public:
    NotRemoved(const PackageUnits& units);
    void add(CONSTANT string& package_name);
    inline bool is_not_removed (CONSTANT string& package_name) const
    { return CONTAINS (not_removed, package_name); }
    virtual ~NotRemoved();
    inline StringSet::const_iterator begin() const {return not_removed.begin();} 
    inline StringSet::const_iterator end() const  {return not_removed.end();} 
private:
    CONSTANT PackageUnits& units;
    StringSet not_removed;
};

#endif	/* NOTREMOVED_HH */

