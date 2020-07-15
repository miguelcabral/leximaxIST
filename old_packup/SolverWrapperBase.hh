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
 * File:   SolverWrapperBase.hh
 * Author: mikolas
 *
 * Created on October 4, 2010, 2:24 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef SOLVERWRAPPERBASE_HH
#define	SOLVERWRAPPERBASE_HH
#include "globals.hh"
#include "common_types.hh"
#include "Printer.hh"
#include "SolverWrapper.hh"

template<class ClausePointer>
class SolverWrapperBase : public SolverWrapper<ClausePointer> {
public:
    SolverWrapperBase()
    : total_soft_weight(0),
      output_clause_count(0)
    {}

    virtual ~SolverWrapperBase() {};
#ifdef CONV_DBG
    void set_printer (Printer* clause_printer) {printer = clause_printer;}
#endif

    void output_weighted_clause(LiteralVector& literals, XLINT weight)
    {
        unique_literals(literals);
         total_soft_weight+=weight;
         ++output_clause_count;

        OUT(cerr <<  weight << " "; printer->print (literals,cerr,true); cerr << " 0" << endl;)
        CNF_OUT(cout <<  weight << " "; printer->print (literals,cout,false); cout << " 0" << endl;)
        _output_weighted_clause (literals, weight);
    }

    void output_unary_weighted_clause(LINT l, XLINT weight)
    {
        total_soft_weight+=weight;
         ++output_clause_count;
        OUT (cerr << weight << " " << printer->print_literal(l) << " 0" << endl;)
        CNF_OUT (cout << weight << " " << l <<" 0" << endl;)
        _output_unary_weighted_clause (l, weight);
    }

    void output_binary_weighted_clause(LINT l1, LINT l2, XLINT weight)
    {
        if (l1==l2) output_unary_weighted_clause(l1, weight);
        total_soft_weight+=weight;
         ++output_clause_count;
        OUT (cerr  << weight << " " << printer->print_literal (l1) << " " << printer->print_literal (l2)  <<" 0" << endl;)
        CNF_OUT (cout << weight << " " << l1 << " " << l2 <<" 0" << endl;)
        _output_binary_weighted_clause (l1, l2, weight);
    }

    void output_unary_clause(LINT l)
    {
         ++output_clause_count;
        OUT (cerr <<  "T " <<printer->print_literal (l) <<" 0" << endl;)
        CNF_OUT (cout <<  "T " << l <<" 0" << endl;)
        _output_unary_clause (l);
    }

    void output_binary_clause(LINT l1, LINT l2)
    {
        if (l1==l2) output_unary_clause(l1);
        OUT (cerr <<  "T " <<  printer->print_literal (l1) <<" "<<printer->print_literal(l2) <<  " 0" << endl;)
        CNF_OUT (cout <<  "T " << l1 <<" "<<l2 <<  " 0" << endl;)
         ++output_clause_count;
        _output_binary_clause (l1,l2);
    }

    void output_clause (/*const*/ LiteralVector& literals)
    {
        unique_literals(literals);
        OUT(cerr <<  "T "; printer->print (literals,cerr,true); cerr << " 0" << endl;)
        CNF_OUT(cout <<  "T "; printer->print (literals,cout,false); cout << " 0" << endl;)
        ++output_clause_count;
        _output_clause (literals);
    }

    void increase_weight(BasicClause* clause, XLINT weight)
    {
        OUT(cerr << weight << " "; printer->print (clause,cerr,true); cerr << endl;)
        CNF_OUT(cout <<  weight << " "; printer->print (clause,cout,false); cout << endl;)
        ++output_clause_count;
        total_soft_weight+=weight;    
        _increase_weight(clause, weight);
    }

    virtual ClausePointer record_clause (LiteralVector& literals)
    {
        unique_literals(literals);
        return _record_clause (literals);
    }

    virtual UINT get_clause_count ()  {return output_clause_count;}
    virtual XLINT get_soft_clauses_weight ()  {return total_soft_weight;}
    
    virtual void _output_clause (LiteralVector& literals) = 0;
    virtual ClausePointer _record_clause (LiteralVector& literals) = 0;
    virtual void _output_binary_clause (LINT l1, LINT l2) = 0;
    virtual void _output_unary_clause (LINT l) = 0;
    virtual void _output_weighted_clause(LiteralVector& literals,XLINT weight) = 0;
    virtual void _output_unary_weighted_clause (LINT l, XLINT weight) = 0;
    virtual void _output_binary_weighted_clause(LINT l1, LINT l2, XLINT weight) = 0;
    virtual void _increase_weight(ClausePointer clause, XLINT weight) = 0;
protected:
    XLINT total_soft_weight;
    UINT output_clause_count;
#ifdef CONV_DBG
    Printer* printer;
#endif
    void unique_literals (LiteralVector& literals)
    {
        if (literals.empty()) return;
        sort(literals.begin(),literals.end(),AbsLitLess());
        UINT insert_position = 1;
        UINT literal_position = 1;
        LINT last = literals[0];
        while (literal_position <literals.size())
        {
            if (last == literals [literal_position]) ++literal_position;
            else {
                literals[insert_position]=literals[literal_position];
                last = literals[literal_position];
                ++insert_position;
                ++literal_position;
            }
        }
        literals.resize(insert_position);
    }

};
#endif	

