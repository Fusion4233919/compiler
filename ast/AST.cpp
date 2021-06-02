/************************************
    Name:        AST.cpp 
    Version:     v2.2
    Modefied by: fusion
                 2021-6-2 20:20
************************************/

#include "AST.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
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

void AST::BuildTable(Fun_attr *current_fun)
{
    Var_attr *temp = NULL;
    switch (this->ntype)
    {
    case Type::none:
    {
        if (strcmp(this->name, "program") == 0)
        {
            this->children->at(0)->BuildTable(NULL);
            this->children->at(1)->BuildTable(NULL);
        }
        if (strcmp(this->name, "Def") == 0)
        {
            /* TYPE Var_List */
            this->children->at(1)->dtype = this->children->at(0)->dtype;
            this->dtype = this->children->at(0)->dtype;
            this->children->at(1)->BuildTable(current_fun);
        }
    }
    break;
    case Type::func:
    {
        /* FUN_TYPE Fun_Var_List  Def_List Exp_List */
        this->dtype = this->children->at(0)->dtype;
        if (funs.find(this->name) != funs.end())
        {
            printf("Multiple definition of %s\n", this->name);
            return;
        }
        funs[this->name] = new Fun_attr(this->name, this->dtype);
        this->children->at(1)->BuildTable(funs[this->name]);
        this->children->at(2)->BuildTable(funs[this->name]);
    }
    break;
    case Type::list:
    {
        if (strcmp(this->name, "Def_List") == 0)
        {
            for (int _ = 0; _ < this->child_num; _++)
                this->children->at(_)->BuildTable(current_fun);
        }
        if (strcmp(this->name, "Fun_List") == 0)
        {
            for (int _ = 0; _ < this->child_num; _++)
                this->children->at(_)->BuildTable(NULL);
        }
        if (strcmp(this->name, "Var_List") == 0)
        {
            for (int _ = 0; _ < this->child_num; _++)
            {
                this->children->at(_)->dtype = this->dtype;
                this->children->at(_)->BuildTable(current_fun);
            }
        }
        if (strcmp(this->name, "Fun_Var_List") == 0)
        {
            for (int _ = 0; _ < this->child_num; _++)
                this->children->at(_)->BuildTable(current_fun);
        }
    }
    break;
    case Type::var:
    {
        if (current_fun == NULL)
        {
            if (glovars.find(this->name) != glovars.end())
            {
                printf("Multiple definition of %s\n", this->name);
                return;
            }
        }
        else if (current_fun->locvars->find(this->name) != current_fun->locvars->end())
        {
            printf("Multiple definition of %s\n", this->name);
            return;
        }
        temp = new Var_attr(this->name, this->dtype);
        if (current_fun != NULL)
            (*(current_fun->locvars))[this->name] = temp;
        temp->belong = current_fun;
        if (this->child_num)
        {
            temp->dim = this->child_num;
            temp->dimention = new std::vector<int>;
            for (int _ = 0; _ < this->child_num; _++)
                temp->dimention->push_back((this->children->at(_)->dvalue).integer);
        }
    }
    break;
    case Type::fvar:
    {
        /* TYPE */
        if (current_fun->locvars->find(this->name) != current_fun->locvars->end())
        {
            printf("Multiple definition of %s\n", this->name);
            return;
        }
        this->dtype = this->children->at(0)->dtype;
        temp = new Var_attr(this->name, this->dtype);
        (*(current_fun->locvars))[this->name] = temp;
        temp->belong = current_fun;
        if (current_fun->argc == 0)
        {
            current_fun->argv = new std::vector<std::pair<DataType, char *>>;
        }
        current_fun->argc++;
        current_fun->argv->push_back(std::pair<DataType, char *>(this->dtype, this->name));
    }
    break;
    default:
        break;
    }
}

void AST::CheckTable(Fun_attr *current_fun)
{
    Var_attr *temp = NULL;
    switch (this->ntype)
    {
    case Type::func:
    {
        this->children->at(3)->CheckTable(funs[this->name]);
    }
    break;
    case Type::list:
    {
        if (strcmp(this->name, "Fun_List") == 0)
        {
            for (int _ = 0; _ < this->child_num; _++)
                this->children->at(_)->CheckTable(NULL);
        }
        if (strcmp(this->name, "Exp_List") == 0)
        {
            for (int _ = 0; _ < this->child_num; _++)
                this->children->at(_)->CheckTable(current_fun);
        }
        if (strcmp(this->name, "List") == 0)
        {
            for (int _ = 0; _ < this->child_num; _++)
                this->children->at(_)->CheckTable(current_fun);
        }
        if (strcmp(this->name, "LList") == 0)
        {
            for (int _ = 0; _ < this->child_num; _++)
                this->children->at(_)->CheckTable(current_fun);
        }
    }
    break;
    case Type::lvalue:
    {
        temp = NULL;
        if (current_fun->locvars->find(this->name) != current_fun->locvars->end())
        {
            temp = current_fun->locvars->find(this->name)->second;
        }
        else if (glovars.find(this->name) != glovars.end())
        {
            temp = glovars.find(this->name)->second;
        }
        else
        {
            printf("No definition of %s\n", this->name);
            return;
        }

        if (temp->dim != this->child_num)
        {
            printf("Dimention not match of %s\n", this->name);
            return;
        }
        this->dtype = temp->dtype;
        for (int _ = 0; _ < this->child_num; _++)
            if (this->children->at(_)->ntype == Type::lvalue)
                this->children->at(_)->CheckTable(current_fun);
    }
    break;
    case Type::exp:
    {
        if (this->name[0] == '_')
        {
            if (funs.find(this->name) == funs.end())
            {
                printf("No definition of %s\n", this->name);
                return;
            }
            Fun_attr *temp = funs.find(this->name)->second;
            this->dtype = temp->rtype;
            if (this->child_num == 0)
            {
                if (temp->argc != 0)
                {
                    printf("No definition of %s()\n", this->name);
                    return;
                }
            }
            else if (this->children->at(0)->child_num != temp->argc)
            {
                printf("Number not match of %s\n", this->name);
                return;
            }
            else
            {
                this->children->at(0)->CheckTable(current_fun);
                for(int _ = 0; _ < temp->argc; _++)
                    if (this->children->at(0)->children->at(_)->dtype != (temp->argv->at(_)).first)
                    {
                        printf("Type not match on %s of %s\n", (temp->argv->at(_)).second, this->name);
                        return;
                    }
            }
        }
        else if (strcmp(this->name, "return") == 0)
        {
            if (this->child_num == 0)
            {
                if (current_fun->rtype != DataType::vvoid)
                {
                    printf("Return Type not match of %s\n", current_fun->name);
                    return ;
                }
            }
            else
            {
                this->children->at(0)->CheckTable(current_fun);
                if (current_fun->rtype != this->children->at(0)->dtype)
                {
                    printf("Return type not match of %s\n", current_fun->name);
                    return ;
                }
            }
        }
        else if (strcmp(this->name, "scanf") == 0)
        {
            this->children->at(1)->CheckTable(current_fun);
        }
        else if (strcmp(this->name, "printf") == 0)
        {
            if (this->child_num > 1)
                this->children->at(1)->CheckTable(current_fun);
        }
        else if (strcmp(this->name, "As_Exp") == 0)
        {
            this->children->at(0)->CheckTable(current_fun);
            this->children->at(1)->CheckTable(current_fun);
            if (this->children->at(0)->dtype != this->children->at(1)->dtype)
            {
                printf("Assign type not match of %s\n", this->children->at(0)->name);
                return ;
            }
        }
    }
        break;
    default:
        break;
    }
}