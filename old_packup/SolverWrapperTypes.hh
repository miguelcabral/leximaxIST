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
 * File:   SolverWrapperTypes.hh
 * Author: mikolas
 *
 * Created on April 28, 2011, 4:38 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef SOLVERWRAPPERTYPES_HH
#define	SOLVERWRAPPERTYPES_HH

#ifdef EXTERNAL_SOLVER
#include "basic_clause.hh"
#include "ExternalWrapper.hh"
typedef BasicClause*    ClausePointer;
typedef ExternalWrapper SolverType  ;
#endif

#ifdef MULTI_LEVEL
#include "basic_clause.hh"
#include "xubmoWrapper.hh"
typedef BasicClause* ClausePointer;
typedef xubmoWrapper SolverType;
#endif

#ifdef SINGLE_LEVEL
#include "msuncore.hh"
#include "MSUnCoreWrapper.hh"
typedef ClPtr           ClausePointer;
typedef MSUnCoreWrapper SolverType;
#endif

#endif	/* SOLVERWRAPPERTYPES_HH */
