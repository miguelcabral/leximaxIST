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
/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2006, 2009-2010 Free Software
   Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     LEXER_END = 0,
     PACKAGE_NAME = 258,
     IDENTIFIER = 259,
     NUMBER = 260,
     PREAMBLE = 261,
     PACKAGE = 262,
     PROPERTY = 263,
     DEPENDS = 264,
     VERSION = 265,
     NEW_LINE = 266,
     COMMA = 267,
     EQUALS = 268,
     NOT_EQUALS = 269,
     GREATER_EQUALS = 270,
     GREATER = 271,
     LESS_EQUALS = 272,
     LESS = 273,
     KEEP = 274,
     REQUEST = 275,
     OPEN_SQUARE = 276,
     CLOSE_SQUARE = 277,
     COLON = 278,
     CONFLICTS = 279,
     INSTALLED = 280,
     PROVIDES = 281,
     INSTALL = 282,
     REMOVE = 283,
     UPGRADE = 284,
     PIPE = 285,
     KEEP_NONE_TOKEN = 286,
     KEEP_VERSION_TOKEN = 287,
     KEEP_PACKAGE_TOKEN = 288,
     KEEP_FEATURE_TOKEN = 289,
     TRUE_BANG = 290,
     FALSE_BANG = 291,
     BOOL_TRUE = 292,
     BOOL_FALSE = 293,
     RECOMMENDS = 294,
     STRING_VALUE = 295
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1685 of yacc.c  */
#line 29 "p.bison"

  char* str;
  bool Boolean;



/* Line 1685 of yacc.c  */
#line 99 "p.tab.hh"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


