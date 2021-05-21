%{
    #include <stdio.h>
    #include "parser.y.h"
%}

%option noyywrap
%option yylineno

%%

"/*"([^\*]|(\*)*[^\*/])*(\*)*"*/" ;     /* comment */ 
[ \t\v\n\f] ;                           /* blank char */

"||"                    return L_OR;    /* logic op */ 
"&&"                    return L_AD;
"=="                    return L_EQ;         
"<="                    return L_LE;
">="                    return L_GE;
"!="                    return L_NE;
"<"                     return '<';
">"                     return '>';

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
"break"                 return BREAK;
"continue"              return CONTINUE;
"return"                return RETURN;

"int"                   return INT;     /* type */
"float"                 return FLOAT;
"void"                  return VOID;

0|([1-9][0-9]*) {
    yylval.num = atoi(yytext);
    return Number;
}

([A-Z]|[a-z])+ {
    yylval.str = yytext;
    return ID;
}

_([A-Z]|[a-z])+ {
    yylval.str = yytext;
    return Fun_ID;
}

. {
    printf("Unknown Token!\n");
    yyterminate();
}
