/************************************
/*    Name:        scanner.l 
/*    Version:     v1.5
/*    Modefied by: fusion
/*                 2021-5-26 16:59
/************************************/

%{
    #include <stdio.h>
    #include "AST.h"
    #include "parser.y.h"
%}

%option noyywrap
%option yylineno

%%

"/*"([^\*]|(\*)*[^\*/])*(\*)*"*/" ;     /* comment */ 
[ \t\v\n\f\r] ;                         /* blank char */

"||"                    return L_OR;    /* logic op */ 
"&&"                    return L_AD;
"=="                    return L_EQ;         
"<="                    return L_LE;
">="                    return L_GE;
"!="                    return L_NE;
"<"                     return '<';
">"                     return '>';

"&"                     return '&';
"="                     return '=';     /* assign op */

"("                     return '(';
")"                     return ')';
"["                     return '[';
"]"                     return ']';
","                     return ',';
";"                     return ';';

"+"                     return '+';     /* arith op */
"-"                     return '-';
"*"                     return '*';
"/"                     return '/';
"%"                     return '%';

"if"                    return IF;      /* ctrl */
"loop"                  return LOOP;
"do"                    return DO;
"done"                  return DONE;
"function"              return FUNCTION;
"declare"               return DECLARE;
"break"                 return BREAK;
"continue"              return CONTINUE;
"return"                return RETURN;

"int"                   return INT;     /* type */
"string"                return STRING;
"void"                  return VOID;

"_input"                return INPUT;
"_output"               return OUTPUT;  /* IO */

\"(\\.|[^"\\])*\" {
    yylval.str = new char[strlen(yytext)];
    strcpy(yylval.str, yytext);
    return String;
}

0|([1-9][0-9]*) {
    yylval.num = atoi(yytext);
    return Number;
}

([A-Z]|[a-z])+ {
    yylval.str = new char[strlen(yytext)];
    strcpy(yylval.str, yytext);
    return ID;
}

_([A-Z]|[a-z])+ {
    yylval.str = new char[strlen(yytext)];
    strcpy(yylval.str, yytext);
    return Fun_ID;
}

. {
    printf("Unknown Token!\n");
    yyterminate();
}
