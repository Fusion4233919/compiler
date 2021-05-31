/************************************
    Name:        AST.cpp 
    Version:     v1.2
    Modefied by: fusion
                 2021-5-26 22:58
************************************/

#include "AST.h"
#include <stdio.h>
#include <string.h>
#include <queue>

int AST::IDAccumulate = 0;
AST::AST(Type type, const char *name)
{
    this->id = ++IDAccumulate;
    this->ntype = type;
    strcpy(this->name, name);
    this->dtype = DataType::nonedt;
    this->op = Operator::noneop;
    this->child_num = 0;
    this->children = NULL;
}

AST::AST(int value)
{
    this->id = ++IDAccumulate;
    this->ntype = Type::cconst;
    strcpy(this->name, "\0");
    this->dtype = DataType::integer;
    this->dvalue.integer = value;
    this->op = Operator::noneop;
    this->child_num = 0;
    this->children = NULL;
}

AST::AST(char *value)
{
    this->id = ++IDAccumulate;
    this->ntype = Type::cconst;
    strcpy(this->name, "\0");
    this->dtype = DataType::string;
    this->dvalue.str = value;
    this->op = Operator::noneop;
    this->child_num = 0;
    this->children = NULL;
}

AST::~AST()
{
    if (this->ntype == Type::cconst && this->dtype == DataType::string)
        delete this->dvalue.str;
    delete this->children;
}

void AST::Insert(AST *p)
{
    if (this->child_num == 0)
        this->children = new std::vector<AST *>;
    this->child_num++;
    children->push_back(p);
}

void AST::print(void)
{
    std::queue<AST *> Q;
    Q.push(this);
    while (!Q.empty())
    {
        AST *tmp = Q.front();
        Q.pop();
        printf("%d:%s", tmp->id, tmp->name);
        if (tmp->ntype == Type::cconst)
        {
            switch (tmp->dtype)
            {
            case DataType::integer:
                printf("%d", tmp->dvalue.integer);
                break;
            case DataType::string:
                printf("%s", tmp->dvalue.str);
                break;
            default:
                break;
            }
        }
        printf("\t");
        for (int _ = 0; _ < tmp->child_num; _++)
        {
            printf("%d ", tmp->children->at(_)->id);
            Q.push(tmp->children->at(_));
        }
        puts("");
    }
}