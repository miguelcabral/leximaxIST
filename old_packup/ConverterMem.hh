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
 * File:   ConverterMem.hh
 * Author: mikolas
 *
 * Created on September 25, 2010, 5:46 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef CONVERTERMEM_HH
#define	CONVERTERMEM_HH
#include "parser.hh"
#include "InstallableUnit.hh"
#include "Encoder.hh"
#include "SolutionReader.hh"

class ConverterMem : public Parser {
public:   
    ConverterMem(SolverWrapper<ClausePointer>& solver,IDManager &id_manager)
    : sr(package_names_set),encoder (solver, id_manager)
    {}

    virtual ~ConverterMem();
    inline void set_criterion(Criterion optimization_criterion)
    {encoder.set_criterion(optimization_criterion);}
    inline void disable_solving () {encoder.disable_solving ();}
    inline bool solve ()  {
        encoder.encode();
        return encoder.solution();
    }

    void solution_check(const char* file_name);
#ifdef MAPPING 
    void set_maping_file (const char* filen);
#endif

    /** Processing of a new package started.*/
    virtual void start_package();

    /** Processing of a package stopped.*/
    virtual void close_package();

    /**The version of currently processed package was read.*/
    virtual void end_processed_package_version();

    /** The package universe has been fully read.*/
    virtual void close_universe();

    /**The whole input was read.*/
    virtual void close_input ();

    //Package stanza
    virtual void package_provides();
    virtual void package_conflicts(); // A list of conflicting versions was read.
    virtual void package_depends();
    virtual void package_recommends();
    virtual void package_keep();
    virtual void package_installed(bool installed_value);

    //Request stanzas
    virtual void request_install();
    virtual void request_remove();
    virtual void upgrade();
    Encoder& get_encoder();
private:
    bool             has_version;
    InstallableUnit  *current_unit;
    SolutionReader   sr;
    Encoder          encoder;
};

#endif	/* CONVERTERMEM_HH */

