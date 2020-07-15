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
 * File:   Printer.cc
 * Author: mikolas
 * 
 * Created on October 4, 2010, 2:04 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#include "Printer.hh"

Printer::Printer(CONSTANT VariableToName& variable_names)
: variable_names (variable_names)
{}

Printer::~Printer() {}


void Printer::print(BasicClause* cl, ostream& o, bool nice) const
{
    Literator lpos = cl->begin();
    Literator lend = cl->end();
    for (; lpos != lend; ++lpos) {
        LINT l=*lpos;
        if (nice) o<< print_literal(l);
        else  o << l;
        o << " ";
    }
    o << "0";
}

void Printer::print(const LiteralVector& vs,ostream& o,bool nice/*=true*/)  const
{
    for (UINT i=0;i<vs.size();++i) {
        if (i>0)  o<< " ";
        if (nice) o<< print_literal(vs[i]);
        else  o << vs[i];
    }
}

void Printer::print_solution(const LiteralVector& vs,ostream& o,bool nice/*=true*/)  const
{
    for (size_t i=1;i<vs.size();++i) {
        LINT literal =vs[i] < 0 ? neg(i) : i;
        if (i>1)  o<< " ";
        if (nice) o<< print_literal(literal);
        else  o << literal;
    }
}

string Printer::print_literal (LINT literal) const
{
#ifdef CONV_DBG
    unordered_map<Variable, string>::const_iterator i= variable_names.find(abs(literal));
    if (i!=variable_names.end()) {
       if (literal>0) return i->second;
       else return  "-" + i->second;
    } else {
       std::stringstream outs;
       //       outs << "<<" << literal << ">>";
       const bool sign = literal>0;
       outs << "<<" << (sign ? "+" : "-") << (sign ? literal : -literal) << ">>";
       return outs.str();
    }
#else
    std::stringstream outs;
    outs << literal;
    return outs.str();
#endif
}
