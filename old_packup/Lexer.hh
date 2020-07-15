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
 * File:   Lexer.hh
 * Author: mikolas
 *
 * Created on September 2, 2010, 12:34 AM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef LEXER_HH
#define	LEXER_HH

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
using namespace std;
using std::cerr;
using std::cout;
using std::cin;
using std::endl;
#include "p.tab.hh"
#define BUFSIZE  124

int get_current_line();
int yylex();


class Lexer {
public:
    Lexer(istream& input_stream);
    int get_current_line();
    int yylex_internal();
private:
    istream& input;
    char buf[BUFSIZE];
 int current_line;

 int token;// Token that was just read
 int done; // Everything has been processed
 bool token_read; // There was a token read

 bool should_read;  // New buffer should be read
 bool last_buffer; // Last buffer is being processed

 // Ragel variables
 int cs, act, have;
 char *ts, *te;
 char *pe;
 char *p;
 char *eof;

 char* mkstring(const char* ts, unsigned int length);
};
#endif	/* LEXER_HH */

