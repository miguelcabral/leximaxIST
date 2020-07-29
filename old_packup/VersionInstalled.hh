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
 * File:   VesionInstalled.hh
 * Author: mikolas
 *
 * Created on September 16, 2010, 12:33 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef VESIONINSTALLED_HH
#define VESIONINSTALLED_HH
#include "common_types.hh"

class VersionInstalled {
public:
    inline VersionInstalled(Version version, bool installed)
    : _version(version), _installed(installed)  {}

    inline VersionInstalled(const VersionInstalled& orig)
    : _version(orig._version), _installed(orig._installed) {}

    VersionInstalled& operator =(const VersionInstalled& orig) {
        _version = orig._version;
        _installed = orig._installed;
        return *this;
    }

    inline Version version() const {return _version;}
    inline bool installed() const {return _installed;}
private:
    Version _version;
    bool _installed;
};

struct VersionInstalledCmp {
  inline bool operator()(const VersionInstalled& v1, const VersionInstalled& v2) const {
    return v1.version() < v2.version();
  }
};

#endif /*VESIONINSTALLED_HH*/

