/************************************
    Name:        AST.cpp 
    Version:     v1.1
    Modefied by: fusion
                 2021-5-26 17:08
************************************/

#include "AST.h"
#include <string.h>

int AST::ID = 0;
AST::AST(Type type, const char* name)
{
    this->id = ++ID;
    this->ntype = type;
    strcpy(this->name, name);
    this->dtype = DataType::nonedt;
    this->op = Operator::noneop;
    this->child_num = 0;
    this->children = NULL;
}

AST::AST(int value)
{
    this->id = ++ID;
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
    this->id = ++ID;
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
    if(this->ntype == Type::cconst && this->dtype == DataType::string)
        delete this->dvalue.str;
    delete this->children;
}

void AST::Insert(AST *p)
{
    if(this->child_num == 0)
        this->children = new std::vector<AST*>;
    this->child_num ++;
    children->push_back(p);
}