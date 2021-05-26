/************************************
    Name:        parser.y 
    Version:     v1.5
    Modefied by: fusion
                 2021-5-26 17:32
************************************/

%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "AST.h"
    
    AST *temp;
    AST *head = NULL;
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
%token <str> ID Fun_ID String
%token <token> L_OR L_AD L_EQ L_LE L_GE L_NE '<' '>'
%token <token> '=' '&'
%token <token> '(' ')' '[' ']' ',' ';'
%token <token> '+' '-' '*' '/' '%'
%token <token> IF LOOP DO DONE FUNCTION DECLARE BREAK CONTINUE RETURN 
%token <token> INT VOID STRING 
%token <token> INPUT OUTPUT

%type <node> Program Def_List Fun_Def_List Fun_Def Def
%type <node> TYPE FUN_TYPE Var Var_List Fun_Var Fun_Var_List LValue Fun_Value List LList
%type <node> Exp_List Exp As_Exp If_Stmt Lop_Stmt Op_Exp Cond_Exp Input_Exp Output_Exp
%type <node> Cond_Term Cond_Factor Cond_Op
%type <node> Op_Term Op_Factor Add_op Mul_op

%left '+' '-' '*' '/' '%'
%left L_OR L_AD L_EQ L_LE L_GE L_NE '<' '>'

%start Program

%%
Program : Def_List Fun_Def_List {$$=new AST(Type::none, "program"); $$->Insert($1); $$->Insert($2); head = $$;}
        ;

Def_List : Def_List Def ';' {$$=$1; $$->Insert($2);}
         | {$$=new AST(Type::list, "Def_List");}
         ;

Fun_Def_List : Fun_Def_List Fun_Def {$$=$1; $$->Insert($2);}
             | {$$=new AST(Type::list, "Fun_List");}
             ;

Def : TYPE Var_List {$$=new AST(Type::none, "Def"); $$->Insert($1); $$->Insert($2);}
    ;

Fun_Def : FUNCTION FUN_TYPE Fun_ID '(' Fun_Var_List ')' DECLARE Def_List DO Exp_List DONE {$$=new AST(Type::func, $3); delete $3; $$->Insert($2); $$->Insert($5); $$->Insert($8); $$->Insert($10);}
        ;

TYPE : INT {$$=new AST(Type::tydf, "int"); $$->dtype=DataType::integer;}
     | STRING {$$=new AST(Type::tydf, "string"); $$->dtype=DataType::string;}
     ;

FUN_TYPE : TYPE {$$=$1;}
         | VOID {$$=new AST(Type::tydf, "void"); $$->dtype=DataType::vvoid;}
         ;

Var_List : Var_List ',' Var {$$=$1; $$->Insert($3);}
         | Var {$$=new AST(Type::list, "Var_List"); $$->Insert($1);}
         ;

Var : Var '[' Number ']' {$$->Insert(new AST($3));}
    | ID {$$=new AST(Type::var, $1); delete $1;}
    ;

Fun_Var_List : Fun_Var_List ',' Fun_Var {$$=$1; $$->Insert($3);}
             | Fun_Var {$$=new AST(Type::list, "Fun_Var_List"); $$->Insert($1);}
             | VOID {$$=new AST(Type::list, "Fun_Var_List"); temp=new AST(Type::tydf, "void"); temp->dtype=DataType::vvoid; $$->Insert(temp);}
             ;

Fun_Var : TYPE ID {$$=new AST(Type::fvar, $2); delete $2; $$->Insert($1); }
        ;

LValue : LValue '[' LValue ']' {$$->Insert($3);}
       | LValue '[' Number ']' {$$->Insert(new AST($3));}
       | ID {$$=new AST(Type::var, $1); delete $1; }
       ;

Fun_Value : Fun_ID '(' List ')' {$$=new AST(Type::exp, $1); delete $1; $$->Insert($3);}
          | Fun_ID '(' ')' {$$=new AST(Type::exp, $1); delete $1;}
          ;

List : List ',' LValue {$$=$1; $$->Insert($3);}
     | List ',' Number {$$=$1; $$->Insert(new AST($3));}
     | List ',' String {$$=$1; $$->Insert(new AST($3));}
     | LValue {$$=new AST(Type::list, "List"); $$->Insert($1);}
     | Number {$$=new AST(Type::list, "List"); $$->Insert(new AST($1));}
     | String {$$=new AST(Type::list, "List"); $$->Insert(new AST($1));}
     ;

LList : LList ',' '&' LValue {$$=$1; $$->Insert($4);}
      | '&' LValue {$$=new AST(Type::list, "List"); $$->Insert($2);}
     ;

Exp_List : Exp_List Exp {$$=$1; $$->Insert($2);}
         | Exp {$$=new AST(Type::list, "Exp_List"); $$->Insert($1);}
         ;

Exp : As_Exp ';' {$$=$1;}
    | Op_Exp ';' {$$=$1;}
    | Cond_Exp ';' {$$=$1;}
    | If_Stmt {$$=$1;}
    | Lop_Stmt {$$=$1;}
    | BREAK ';' {$$=new AST(Type::exp, "break");}
    | CONTINUE ';' {$$=new AST(Type::exp, "continue");}
    | RETURN Op_Exp ';' {$$=new AST(Type::exp, "return"); $$->Insert($2);}
    | RETURN ';' {$$=new AST(Type::exp, "return");}
    | Input_Exp ';' {$$=$1;}
    | Output_Exp ';' {$$=$1;}
    ;

Input_Exp : INPUT '(' String ',' LList ')' {$$=new AST(Type::exp, "scanf"); $$->Insert(new AST($3)); $$->Insert($5);}
          ;

Output_Exp : OUTPUT '(' String ',' List ')' {$$=new AST(Type::exp, "printf"); $$->Insert(new AST($3)); $$->Insert($5);}
           | OUTPUT '(' String ')' {$$=new AST(Type::exp, "printf"); $$->Insert(new AST($3));}
           ;

As_Exp : LValue '=' Op_Exp {$$=new AST(Type::exp, "As_Exp"); $$->Insert($1); $$->Insert($3);}
       | LValue '=' String {$$=new AST(Type::exp, "As_Exp"); $$->Insert($1); $$->Insert(new AST($3));}
       ;

Op_Exp : Op_Exp Add_op Op_Term {$$=$1; $$->Insert($2); $$->Insert($3);}
       | Op_Term {$$=new AST(Type::exp, "Op_Exp"); $$->Insert($1);}
       ;

Op_Term : Op_Term Mul_op Op_Factor {$$=$1; $$->Insert($2); $$->Insert($3);}
        | Op_Factor {$$=new AST(Type::exp, "Op_Term"); $$->Insert($1);}
        ;

Op_Factor : '(' Op_Exp ')' {$$=$2;}
          | LValue {$$=$1;}
          | Fun_Value {$$=$1;}
          | Number {$$=new AST($1);}
          ;

Add_op : '+' {$$=new AST(Type::opr, ""); $$->op=Operator::add;}
       | '-' {$$=new AST(Type::opr, ""); $$->op=Operator::min;}
       ;

Mul_op : '*' {$$=new AST(Type::opr, ""); $$->op=Operator::mul;}
       | '/' {$$=new AST(Type::opr, ""); $$->op=Operator::ddi;}
       | '%' {$$=new AST(Type::opr, ""); $$->op=Operator::mod;}
       ;

Cond_Exp : Cond_Exp L_OR Cond_Term {$$=$1; temp=new AST(Type::opr, ""); $$->op=Operator::OR; $$->Insert(temp); $$->Insert($3);}
         | Cond_Term {$$=new AST(Type::exp, "Cond_Exp"); $$->Insert($1);}
         ;

Cond_Term : Cond_Term L_AD Cond_Factor {$$=$1; temp=new AST(Type::opr, ""); $$->op=Operator::AD; $$->Insert(temp); $$->Insert($3);}
          | Cond_Factor {$$=new AST(Type::exp, "Cond_Term"); $$->Insert($1);}
          ;

Cond_Factor : '(' Cond_Exp ')' {$$=$2;}
            | Op_Exp Cond_Op Op_Exp {$$=new AST(Type::exp, "Cond_Factor"); $$->Insert($1); $$->Insert($2); $$->Insert($3);}
            ;

Cond_Op : L_EQ {$$=new AST(Type::opr, ""); $$->op=Operator::EQ;}
        | L_GE {$$=new AST(Type::opr, ""); $$->op=Operator::GE;}
        | L_LE {$$=new AST(Type::opr, ""); $$->op=Operator::LE;}
        | L_NE {$$=new AST(Type::opr, ""); $$->op=Operator::NE;}
        | '<' {$$=new AST(Type::opr, ""); $$->op=Operator::LT;}
        | '>' {$$=new AST(Type::opr, ""); $$->op=Operator::GT;}
        ;

If_Stmt : IF '(' Cond_Exp ')' DO Exp_List DONE {$$=new AST(Type::exp, "If_Stmt"); $$->Insert($3); $$->Insert($6);}
        ;

Lop_Stmt : LOOP '(' Cond_Exp ')' DO Exp_List DONE {$$=new AST(Type::exp, "Lop_Stmt"); $$->Insert($3); $$->Insert($6);}
        ;

%%

void yyerror(const char*s)
{
    printf("%s\n", s);
    printf("line %d:", yylineno);
    printf("text \"%s\" \n", yytext);
    exit(1);
}
