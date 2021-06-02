/************************************
    Name:        AST.cpp 
    Version:     v1.2
    Modefied by: fusion
                 2021-5-26 22:58
************************************/

#include "AST.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <queue>

int AST::IDAccumulate = 0;
Vmap var_table;
Fmap fun_table;

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

void AST::BuildTable(Vmap *current_var_table)
{
    switch (this->ntype)
    {
    case Type::none:
        if (strcmp(this->name, "program") == 0)
        {
            this->children->at(0)->BuildTable(&var_table);
            this->children->at(1)->BuildTable(&var_table);
        }
        if (strcmp(this->name, "Def") == 0)
        {
            /* TYPE Var_List */
            this->children->at(1)->dtype = this->children->at(0)->dtype;
            this->dtype = this->children->at(0)->dtype;
            this->children->at(1)->BuildTable(current_var_table);
        }
        break;
    case Type::func:
        /* FUN_TYPE Fun_Var_List  Def_List Exp_List */
        this->dtype = this->children->at(0)->dtype;
        fun_table[this->name] = new Fun_attr(this->name, this->dtype);
        this->children->at(1)->BuildTable(fun_table[this->name]->locvars);
        this->children->at(2)->BuildTable(fun_table[this->name]->locvars);
        this->children->at(3)->BuildTable(fun_table[this->name]->locvars);
        break;
    case Type::list:
        /* TODO: List */
        if (strcmp(this->name, "Def_List") == 0)
        {
            for (int _ = 0; _ < this->child_num; _++)
                this->children->at(_)->BuildTable(current_var_table);
        }
        if (strcmp(this->name, "Fun_List") == 0)
        {
            for (int _ = 0; _ < this->child_num; _++)
                this->children->at(_)->BuildTable(&var_table);
        }
        if (strcmp(this->name, "Var_List") == 0)
        {
            for (int _ = 0; _ < this->child_num; _++)
            {
                this->children->at(_)->dtype = this->dtype;
                this->children->at(_)->BuildTable(current_var_table);
            }
        }
        if (strcmp(this->name, "Fun_Var_List") == 0)
        {
            for (int _ = 0; _ < this->child_num; _++)
                this->children->at(_)->BuildTable(current_var_table);
        }
        if (strcmp(this->name, "Exp_List") == 0)
        {
            for (int _ = 0; _ < this->child_num; _++)
                this->children->at(_)->BuildTable(current_var_table);
        }
        break;
    case Type::var:
        (*current_var_table)[this->name] = new Var_attr(this->name,this->dtype);
        if (this->child_num)
        {
            (*current_var_table)[this->name]->dim = this->child_num;
            (*current_var_table)[this->name]->dimention = new std::vector<int>;
            for(int _ = 0; _ < this->child_num; _++)
                (*current_var_table)[this->name]->dimention->push_back((this->children->at(_)->dvalue).integer);
        }
        break;
    case Type::fvar:
        /* TYPE */
        this->dtype = this->children->at(0)->dtype;
        (*current_var_table)[this->name] = new Var_attr(this->name,this->dtype);
        break;
    default:
        break;
    }
}
