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
 * File:   Printer.hh
 * Author: mikolas
 *
 * Created on October 4, 2010, 2:04 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef PRINTER_HH
#define	PRINTER_HH

#include "collections.hh"
#include "basic_clause.hh"
class Printer {
public:
    Printer(CONSTANT VariableToName& variable_names);
    virtual ~Printer();
    void print (const LiteralVector& literals,ostream& o,bool nice=true) const;
    void print (BasicClause* cl,ostream& o,bool nice=true) const;
    void print_solution(const LiteralVector& vs,ostream& o,bool nice=true) const;
    string print_literal (LINT literal) const;
private:
    CONSTANT VariableToName& variable_names;

};

#endif	/* PRINTER_HH */

