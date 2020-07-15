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
 * File:   SolverWrapper.hh
 * Author: mikolas
 *
 * Created on October 4, 2010, 1:32 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef SOLVERWRAPPER_HH
#define	SOLVERWRAPPER_HH
#include "collections.hh"
#include "Printer.hh"

template<class ClausePtr>
class SolverWrapper
{
public:
    virtual ~SolverWrapper() {}
    virtual void init () = 0;
    virtual void set_top(XLINT top)=0;
    virtual XLINT get_top()=0;
    virtual void output_clause (LiteralVector& literals) = 0;
    virtual ClausePtr record_clause (LiteralVector& literals) = 0;
    virtual void output_binary_clause (LINT l1, LINT l2) = 0;
    virtual void output_unary_clause (LINT l) = 0;
    virtual void output_weighted_clause(/*const*/ LiteralVector& literals,XLINT weight) = 0;
    virtual void output_unary_weighted_clause (LINT l, XLINT weight) = 0;
    virtual void output_binary_weighted_clause(LINT l1, LINT l2, XLINT weight) = 0;
    virtual void increase_weight(ClausePtr clause, XLINT weight) = 0;
    virtual bool solve() = 0;
    virtual IntVector& get_model() = 0;                   // Access computed model
    virtual bool has_solution() = 0;

    /**
     * This is used to produce single clauses from one clauses that resulted from summing up some of the weights.
     * This occurs when the same clause appears in multiple sometimes in the optimization function.
     * Summing the weights complicates the lexicographic algorithm for optimization.
     * @param weight this weight is being singled out.
     * @return weight was actually inserted
     */
    virtual bool register_weight(XLINT weight) = 0;

    virtual UINT get_clause_count () =0;
    virtual XLINT get_soft_clauses_weight () =0;
    virtual XLINT get_min_unsat_cost() =0;

    virtual void dump(ostream& out) = 0;
#ifdef CONV_DBG
    virtual void set_printer (Printer* clause_printer) = 0;
#endif
};

#endif	/* SOLVERWRAPPER_HH */

