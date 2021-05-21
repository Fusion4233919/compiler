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
    struct AST *node;
}


%token <num> Number
%token <str> ID Fun_ID
%token <token> L_OR L_AD L_EQ L_LE L_GE L_NE '<' '>'
%token <token> '='
%token <token> '(' ')' '[' ']' ',' ';'
%token <token> '+' '-' '*' '/' '%'
%token <token> IF LOOP DO DONE FUNCTION BREAK CONTINUE RETURN
%token <token> INT FLOAT VOID

%type <node> Program Block Fun_Def Def_Exp
%type <node> TYPE FUN_TYPE Var Var_List Fun_Var Fun_Var_List Fun_Value
%type <node> Exp_List Exp As_Exp If_Stmt Lop_Stmt Op_Exp Cond_Exp
%type <node> Cond_Term Cond_Factor Cond_Op
%type <node> Op_Term Op_Factor Add_op Mul_op

%left '+' '-' '*' '/' '%'
%left L_OR L_AD L_EQ L_LE L_GE L_NE '<' '>'

%start Program

%%
Program : Program Block {;}
        | Block {;}
        ;

Block : Fun_Def {;}
      | Def_Exp ';' {;}
      ;

As_Exp : Var '=' Op_Exp {;}
       ;

Def_Exp : TYPE Var_List {;}
        ;

Fun_Def : FUNCTION FUN_TYPE Fun_ID '(' Fun_Var_List ')' DO Exp_List DONE {;}
        ;

TYPE : INT {;}
     | FLOAT {;}
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
             ;

Fun_Var : TYPE ID {;}
        | VOID {;}
        ;

Fun_Value : Fun_ID '(' Var_List ')' {;}
          ;

Exp_List : Exp_List Exp {;}
         | Exp {;}
         ;

Exp : Def_Exp ';' {;} 
    | As_Exp ';' {;}
    | Op_Exp ';' {;}
    | Cond_Exp ';' {;}
    | If_Stmt {;}
    | Lop_Stmt {;}
    | BREAK ';' {;}
    | CONTINUE ';' {;}
    | RETURN Op_Exp ';' {;}
    ;

Op_Exp : Op_Exp Add_op Op_Term {;}
       | Op_Term {;}
       ;

Op_Term : Op_Term Mul_op Op_Factor {;}
        | Op_Factor {;}
        ;

Op_Factor : '(' Op_Exp ')' {;}
          | Var {;}
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

int main(void)
{
    return yyparse();
}