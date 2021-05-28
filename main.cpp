/************************************
    Name:        main.cpp 
    Version:     v1.1
    Modefied by: fusion
                 2021-5-28 10:59
************************************/

#include <stdio.h>
#include <stdlib.h>
#include "AST.h"

extern int yyparse();
extern FILE *yyin;
static FILE *file_in;

Vmap glovars;
Fmap funs;

int main(int argc, const char *argv[])
{
    const char *file_in_name = argv[1];
    if ((file_in = fopen(file_in_name, "r")) == NULL)
    {
        printf("error on open file %s!", file_in_name);
        getchar();
        return 1;
    }
    yyin = file_in;
    yyparse();
    fclose(file_in);
    head->print();
    return 0;
}
