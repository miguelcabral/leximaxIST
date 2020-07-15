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
 * File:   PackageVersionVariables.hh
 * Author: mikolas
 *
 * Created on September 27, 2010, 2:19 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef PACKAGEVERSIONVARIABLES_HH
#define	PACKAGEVERSIONVARIABLES_HH
#include "common_types.hh"
#include "id_manager.hh"
#include "collections.hh"

class PackageVersionVariables {
public:
    PackageVersionVariables(IDManager&  id_manager
#ifdef MAPPING 
   , unordered_map<Variable, string>& variable_names,
     ofstream& mapping
#endif
    );

    virtual ~PackageVersionVariables();

    /**
     * Look for a variable in {@code variables}.
     * @param pv Feature or package version whose corresponding variable is searched for.
     * @param variable Is the search succeeds, the variable is placed in this variable,
     *                 otherwise it is unchanged.
     * @return True iff the search was successful.
     */
    inline bool find_variable (CONSTANT PackageVersion &pv, Variable& variable)
    {
        PackageVersionMap::const_iterator i = variables.find(pv);
        if (i ==  variables.end ()) return false;
        variable = i->second;
        return true;
    }

    inline Variable add_variable_feature(CONSTANT PackageVersion& pv)
    {
#ifdef  MAPPING
        return add_variable (pv, "FV");
#else
        return add_variable (pv);
#endif
    }
    inline Variable add_variable_package(CONSTANT PackageVersion& pv)
    {
#ifdef  MAPPING
        return add_variable (pv, "PV");
#else
        return add_variable (pv);
#endif
    }

    inline bool has_variable (CONSTANT PackageVersion& pv)
    { return variables.find(pv) != variables.end(); }

private:
    inline Variable add_variable (CONSTANT PackageVersion& pv
#ifdef  MAPPING
                                 ,CONSTANT string mn
#endif
    )
    {
        assert(!has_variable (pv));
        CONSTANT Variable return_value = id_manager.new_id();
        variables[pv]=return_value;
#ifdef  MAPPING
        string nm= mn + pv.to_string();
        variable_names[return_value] = nm;
        mapping << return_value << "->" << nm << endl;
#endif
        return return_value;
    }
    
private:
    IDManager& id_manager;
#ifdef  MAPPING
   unordered_map<Variable, string>& variable_names;// Names of variables for debugging purposes.
   ofstream& mapping;
#endif
    PackageVersionMap variables;
};

#endif	/* PACKAGEVERSIONVARIABLES_HH */

