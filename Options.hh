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
 * File:   Options.hh
 * Author: mikolas
 *
 * Created on April 23, 2011, 4:23 PM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef OPTIONS_HH
#define	OPTIONS_HH
#include <getopt.h>
#include <assert.h>
#include <string>
using std::string;

class Options {
public:
    Options();
    virtual ~Options();
    bool   parse(int count,char** arguments);
private:
    int    m_help;
    string  m_external_solver;
    string  m_input_files;
    string  m_multiplication_string;
    int    m_leave_temporary_files;
    int    m_pbo;
};

#endif	/* OPTIONS_HH */

