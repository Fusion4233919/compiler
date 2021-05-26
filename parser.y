/************************************
    Name:        parser.y 
    Version:     v1.4
    Modefied by: fusion
                 2021-5-25 10:47
************************************/

%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "AST.h"
    
    extern int yylex();
    extern int yylineno;
    extern char *yytext;
    void yyerror(const char*);
%}

%union {
    char *str;
    int num;
    int token;
    AST *node;
}


%token <num> Number
//%token <fnum> Fnumber
%token <str> ID Fun_ID String
%token <token> L_OR L_AD L_EQ L_LE L_GE L_NE '<' '>'
%token <token> '=' '&'
%token <token> '(' ')' '[' ']' ',' ';'
%token <token> '+' '-' '*' '/' '%'
%token <token> IF LOOP DO DONE FUNCTION DECLARE BREAK CONTINUE RETURN
%token <token> INT VOID STRING 

%type <node> Program Def_List Fun_Def_List Fun_Def Def
%type <node> TYPE FUN_TYPE Var Var_List Fun_Var Fun_Var_List LValue Fun_Value List
%type <node> Exp_List Exp As_Exp If_Stmt Lop_Stmt Op_Exp Cond_Exp
%type <node> Cond_Term Cond_Factor Cond_Op
%type <node> Op_Term Op_Factor Add_op Mul_op

%left '+' '-' '*' '/' '%'
%left L_OR L_AD L_EQ L_LE L_GE L_NE '<' '>'

%start Program

%%
Program : Def_List Fun_Def_List {;}
        ;

Def_List : Def_List Def ';' {;}
         | {;}
         ;

Fun_Def_List : Fun_Def_List Fun_Def {;}
             | {;}
             ;

Def : TYPE Var_List {;}
    ;

Fun_Def : FUNCTION FUN_TYPE Fun_ID '(' Fun_Var_List ')' DECLARE Def_List DO Exp_List DONE {;}
        ;

TYPE : INT {;}
     | STRING {;}
     ;

FUN_TYPE : TYPE {;}
         | VOID {;}
         ;

Var_List : Var_List ',' Var {;}
         | Var {;}
         ;

Var : Var '[' Number ']' {;}
    | ID {;}
    ;

Fun_Var_List : Fun_Var_List ',' Fun_Var {;}
             | Fun_Var {;}
             | VOID {;}
             ;

Fun_Var : TYPE ID {;}
        ;

LValue : LValue '[' LValue ']' {;}
       | LValue '[' Number ']' {;}
       | ID {;}
       ;

Fun_Value : Fun_ID '(' List ')' {;}
          ;

List : List ',' LValue {;}
     | List ',' Number {;}
     | List ',' String {;}
     | LValue {;}
     | Number {;}
     | String {;}
     | {;}
     ;

Exp_List : Exp_List Exp {;}
         | Exp {;}
         ;

Exp : As_Exp ';' {;}
    | Op_Exp ';' {;}
    | Cond_Exp ';' {;}
    | If_Stmt {;}
    | Lop_Stmt {;}
    | BREAK ';' {;}
    | CONTINUE ';' {;}
    | RETURN Op_Exp ';' {;}
    | RETURN ';' {;}
    ;


As_Exp : LValue '=' Op_Exp {;}
       | LValue '=' String {;}
       ;

Op_Exp : Op_Exp Add_op Op_Term {;}
       | Op_Term {;}
       ;

Op_Term : Op_Term Mul_op Op_Factor {;}
        | Op_Factor {;}
        ;

Op_Factor : '(' Op_Exp ')' {;}
          | LValue {;}
          | Fun_Value {;}
          | Number {;}
          ;

Add_op : '+' {;}
       | '-' {;}
       ;

Mul_op : '*' {;}
       | '/' {;}
       | '%' {;}
       ;

Cond_Exp : Cond_Exp L_OR Cond_Term {;}
         | Cond_Term {;}
         ;

Cond_Term : Cond_Term L_AD Cond_Factor {;}
          | Cond_Factor {;}
          ;

Cond_Factor : '(' Cond_Exp ')' {;}
            | Op_Exp Cond_Op Op_Exp {;}
            ;

Cond_Op : L_EQ {;}
        | L_GE {;}
        | L_LE {;}
        | L_NE {;}
        | '<' {;}
        | '>' {;}
        ;

If_Stmt : IF '(' Cond_Exp ')' DO Exp_List DONE {;}
        ;

Lop_Stmt : LOOP '(' Cond_Exp ')' DO Exp_List DONE {;}
        ;

%%

void yyerror(const char*s)
{
    printf("%s\n", s);
    printf("line %d:", yylineno);
    printf("text \"%s\" \n", yytext);
    exit(1);
}
