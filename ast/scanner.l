/************************************
/*    Name:        scanner.l 
/*    Version:     v1.8
/*    Modefied by: fusion
/*                 2021-6-5 14:23
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
"void"                  return VOID;

"_input"                return INPUT;
"_output"               return OUTPUT;  /* IO */

\"(\\.|[^"\\])*\" {
    yylval.str = new char[yyleng - 1];
    strcpy(yylval.str, yytext + 1);
    yylval.str[yyleng-2] = '\0';
    return String;
}

\'([A-Z]|[a-z]|[\+\-\*\/\%\=\<\>\!\@\#\$\^\&\(\)\[\]\,\;\\\|])\' {
    yylval.num = yytext[1];
    return Number;
}

\'\\n\' {
    yylval.num = '\n';
    return Number;
}

\'\\t\' {
    yylval.num = '\t';
    return Number;
}

\'\\r\' {
    yylval.num = '\r';
    return Number;
}

-?(0|[1-9][0-9]*) {
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
