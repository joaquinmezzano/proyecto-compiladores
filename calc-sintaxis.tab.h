/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_CALC_SINTAXIS_TAB_H_INCLUDED
# define YY_YY_CALC_SINTAXIS_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 2 "calc-sintaxis.y"

    #include "ast.h"

#line 53 "calc-sintaxis.tab.h"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    ID = 258,                      /* ID  */
    NUMERO = 259,                  /* NUMERO  */
    INT = 260,                     /* INT  */
    BOOL = 261,                    /* BOOL  */
    VOID = 262,                    /* VOID  */
    TRUE = 263,                    /* TRUE  */
    FALSE = 264,                   /* FALSE  */
    RETURN = 265,                  /* RETURN  */
    MAIN = 266,                    /* MAIN  */
    PARA = 267,                    /* PARA  */
    PARC = 268,                    /* PARC  */
    LLAA = 269,                    /* LLAA  */
    LLAC = 270,                    /* LLAC  */
    CORA = 271,                    /* CORA  */
    CORC = 272,                    /* CORC  */
    PYC = 273,                     /* PYC  */
    COMA = 274,                    /* COMA  */
    OP_RESTA = 275,                /* OP_RESTA  */
    OP_SUMA = 276,                 /* OP_SUMA  */
    OP_MAYOR = 277,                /* OP_MAYOR  */
    OP_MENOR = 278,                /* OP_MENOR  */
    OP_AND = 279,                  /* OP_AND  */
    OP_OR = 280,                   /* OP_OR  */
    OP_DIV = 281,                  /* OP_DIV  */
    OP_MULT = 282,                 /* OP_MULT  */
    OP_IGUAL = 283,                /* OP_IGUAL  */
    OP_ASIGN = 284                 /* OP_ASIGN  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 16 "calc-sintaxis.y"

    Nodo *nodo;
    char *str;
    int num;

#line 105 "calc-sintaxis.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_CALC_SINTAXIS_TAB_H_INCLUDED  */
