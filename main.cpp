/************************************
    Name:        main.cpp 
    Version:     v1.3
    Modefied by: fusion
                 2021-6-2 22:13
************************************/

#include <stdio.h>
#include <stdlib.h>
#include "AST.h"

extern int yyparse();
extern FILE *yyin;
static FILE *file_in;

Vmap glovars;
Fmap funs;

void showTable(void)
{
    for(auto _ = glovars.begin(); _ != glovars.end(); _++)
    {
        printf("%s %s %d %d\n", _->first.c_str(), _->second->name.c_str(), _->second->dtype, _->second->dim);
    }
    puts("");
    for (auto _ = funs.begin(); _ != funs.end(); _++)
    {
        printf("%s %s %d \n", _->first.c_str(), _->second->name.c_str(), _->second->rtype);
        for (auto __ = _->second->locvars->begin(); __ != _->second->locvars->end(); __++)
        {
            printf("\t%s %s %d %d\n", __->first.c_str(), __->second->name.c_str(), __->second->dtype, __->second->dim);
        }
    }
}

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
    //head->print();
    head->BuildTable(NULL);
    showTable();
    head->CheckTable(NULL);
    return 0;
}
