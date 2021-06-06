/************************************
    Name:        main.cpp 
    Version:     v1.3
    Modefied by: fusion
                 2021-6-2 22:13
************************************/

#include <stdio.h>
#include <stdlib.h>
#include "AST.h"
#include "gen.h"

extern int yyparse();
extern FILE *yyin;
static FILE *file_in;

Vmap glovars;
Fmap funs;

void showTable(void)
{
    for (auto _ = glovars.begin(); _ != glovars.end(); _++)
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
        for (auto __ = _->second->locfuns->begin(); __ != _->second->locfuns->end(); __++)
        {
            printf("\t%s %s %d \n", __->first.c_str(), __->second->name.c_str(), __->second->rtype);
            for (auto ___ = __->second->locvars->begin(); ___ != __->second->locvars->end(); ___++)
            {
                printf("\t\t%s %s %d %d\n", ___->first.c_str(), ___->second->name.c_str(), ___->second->dtype, ___->second->dim);
            }
            if (__->second->parents_argv != NULL)
            {
                for (auto ___ = __->second->parents_argv->begin(); ___ != __->second->parents_argv->end(); ___++)
                {
                    printf("\t\t(p) %s %s %d %d\n", ___->first.c_str(), ___->second->name.c_str(), ___->second->dtype, ___->second->dim);
                }
            }
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
    if (head->CheckTable(NULL));
    showTable();
    gen::ProgramGen(head);
#ifdef __APPLE__
    if (argc == 2) return 0;
    const char *option = argv[2];
    if (strcmp(option, "-e") == 0) {
        system("/usr/local/llvm/12.0.0/bin/llc ./mhl.ll && /usr/bin/clang -o mhl -lm mhl.S");
    }
#endif
    return 0;
}
