/************************************
    Name:        AST.cpp 
    Version:     v4.1
    Modefied by: fusion
                 2021-6-6 12:47
************************************/

#include "AST.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <queue>

int AST::IDAccumulate = 0;

static bool error = 0;

static char *StrPreprocess(char *str)
{
    int len = strlen(str);
    int l = 0, r = 0;
    while (r < len)
    {
        if (str[r] != '\\')
            str[l] = str[r];
        else
        {
            r++;
            switch (str[r])
            {
            case 'n':
                str[l] = '\n';
                break;
            case 't':
                str[l] = '\t';
                break;
            case 'r':
                str[l] = '\r';
                break;
            case '0':
                str[l] = '\0';
                break;
            case '\\':
                str[l] = '\\';
                break;
            case '\'':
                str[l] = '\'';
                break;
            case '\"':
                str[l] = '\"';
                break;
            }
        }
        l++;
        r++;
    }
    str[l] = '\0';
    return str;
}

AST::AST(Type type, const char *name)
{
    this->id = ++IDAccumulate;
    this->ntype = type;
    this->name = name;
    this->dtype = DataType::nonedt;
    this->op = Operator::noneop;
    this->child_num = 0;
    this->children = NULL;
}

AST::AST(int value)
{
    this->id = ++IDAccumulate;
    this->ntype = Type::cconst;
    this->name = "\0";
    this->dtype = DataType::integer;
    this->dvalue.integer = value;
    this->op = Operator::noneop;
    this->child_num = 0;
    this->children = NULL;
}

AST::AST(char *value)
{
    value = StrPreprocess(value);
    this->id = ++IDAccumulate;
    this->ntype = Type::cconst;
    this->name = "\0";
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
        printf("%d:%s", tmp->id, tmp->name.c_str());
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
        this->children->at(0)->BuildTable(NULL);
        this->children->at(1)->BuildTable(NULL);
        break;
    }
    case Type::defi:
    {
        /* TYPE Var_List */
        this->children->at(1)->dtype = this->children->at(0)->dtype;
        this->dtype = this->children->at(0)->dtype;
        if (this->dtype == DataType::function && NULL == current_fun)
        {
            printf("Global fn vars are not allow\n");
            return;
        }
        this->children->at(1)->BuildTable(current_fun);
        break;
    }
    case Type::func:
    {
        /* FUN_TYPE Fun_Var_List  Def_List Exp_List */
        this->dtype = this->children->at(0)->dtype;
        if (current_fun != NULL)
        {
            if (current_fun->locfuns->find(this->name) != current_fun->locfuns->end())
            {
                printf("Multiple definition of %s\n", this->name.c_str());
                return;
            }
            (*(current_fun->locfuns))[this->name] = new Fun_attr(this->name, this->dtype);
            (*(current_fun->locfuns))[this->name]->belong = current_fun;
            this->children->at(1)->BuildTable(current_fun->locfuns->at(this->name));
            this->children->at(2)->BuildTable(current_fun->locfuns->at(this->name));
            this->children->at(3)->BuildTable(current_fun->locfuns->at(this->name));
        }
        else
        {
            if (funs.find(this->name) != funs.end())
            {
                printf("Multiple definition of %s\n", this->name.c_str());
                return;
            }
            funs[this->name] = new Fun_attr(this->name, this->dtype);
            this->children->at(1)->BuildTable(funs[this->name]);
            this->children->at(2)->BuildTable(funs[this->name]);
            this->children->at(3)->BuildTable(funs[this->name]);
        }
        break;
    }
    case Type::list:
    {
        if (this->name == "Def_List")
        {
            for (int _ = 0; _ < this->child_num; _++)
            {
                this->children->at(_)->BuildTable(current_fun);
            }
        }
        if (this->name == "Fun_List")
        {
            for (int _ = 0; _ < this->child_num; _++)
            {
                this->children->at(_)->BuildTable(NULL);
            }
        }
        if (this->name == "Var_List")
        {
            for (int _ = 0; _ < this->child_num; _++)
            {
                this->children->at(_)->dtype = this->dtype;
                this->children->at(_)->BuildTable(current_fun);
            }
        }
        if (this->name == "Fun_Var_List")
        {
            for (int _ = 0; _ < this->child_num; _++)
            {
                this->children->at(_)->BuildTable(current_fun);
            }
        }
        if (this->name == "Exp_List")
        {
            for (int _ = 0; _ < this->child_num; _++)
            {
                if (this->children->at(_)->ntype == Type::func)
                    this->children->at(_)->BuildTable(current_fun);
            }
        }
        if (this->name == "Fun_Name_List")
        {
            for (int _ = 0; _ < this->child_num; _++)
            {
                this->children->at(_)->dtype = this->dtype;
                this->children->at(_)->BuildTable(current_fun);
            }
        }
        break;
    }
    case Type::var:
    {
        if (current_fun == NULL)
        {
            if (glovars.find(this->name) != glovars.end())
            {
                printf("Multiple definition of %s\n", this->name.c_str());
                return;
            }
        }
        else if (current_fun->locvars->find(this->name) != current_fun->locvars->end())
        {
            printf("Multiple definition of %s\n", this->name.c_str());
            return;
        }

        temp = new Var_attr(this->name, this->dtype);
        if (current_fun != NULL)
        {
            (*(current_fun->locvars))[this->name] = temp;
        }
        else
        {
            glovars[this->name] = temp;
        }
        temp->belong = current_fun;

        if (this->child_num)
        {
            temp->dim = this->child_num;
            temp->dimention = new std::vector<int>;
            for (int _ = 0; _ < this->child_num; _++)
            {
                temp->dimention->push_back((this->children->at(_)->dvalue).integer);
            }
        }
        break;
    }
    case Type::fvar:
    {
        /* TYPE */
        if (current_fun->locvars->find(this->name) != current_fun->locvars->end())
        {
            printf("Multiple definition of %s\n", this->name.c_str());
            return;
        }
        this->dtype = this->children->at(0)->dtype;
        temp = new Var_attr(this->name, this->dtype);
        (*(current_fun->locvars))[this->name] = temp;
        temp->belong = current_fun;
        if (current_fun->argc == 0)
        {
            current_fun->argv = new std::vector<std::pair<DataType, std::string>>;
        }
        current_fun->argc++;
        current_fun->argv->push_back(std::pair<DataType, std::string>(this->dtype, this->name));
        break;
    }
    default:
        break;
    }
}

bool AST::CheckTable(Fun_attr *current_fun)
{
    Var_attr *temp = NULL;
    switch (this->ntype)
    {
    case Type::none:
    {
        if (this->name == "program")
        {
            this->children->at(1)->CheckTable(NULL);
        }
        break;
    }
    case Type::func:
    {
        if (this->child_num == 0)
        {
            if (current_fun->locfuns->find(this->name) == current_fun->locfuns->end())
            {
                printf("No definition of %s\n", this->name.c_str());
                return error = true;
            }
            return error;
        }

        if (current_fun != NULL)
            this->children->at(3)->CheckTable(current_fun->locfuns->at(this->name));
        else
            this->children->at(3)->CheckTable(funs[this->name]);
        break;
    }
    case Type::list:
    {
        if (this->name == "Fun_List")
        {
            for (int _ = 0; _ < this->child_num; _++)
            {
                this->children->at(_)->CheckTable(NULL);
            }
        }
        if (this->name == "Exp_List")
        {
            for (int _ = 0; _ < this->child_num; _++)
            {
                this->children->at(_)->CheckTable(current_fun);
            }
        }
        if (this->name == "List")
        {
            for (int _ = 0; _ < this->child_num; _++)
            {
                this->children->at(_)->CheckTable(current_fun);
            }
        }
        if (this->name == "LList")
        {
            for (int _ = 0; _ < this->child_num; _++)
            {
                this->children->at(_)->CheckTable(current_fun);
            }
        }
        break;
    }
    case Type::lvalue:
    {
        temp = NULL;
        if (current_fun != NULL && current_fun->locvars->find(this->name) != current_fun->locvars->end())
        {
            temp = current_fun->locvars->find(this->name)->second;
        }
        else if (current_fun->belong != NULL && current_fun->belong->locvars->find(this->name) != current_fun->belong->locvars->end())
        {
            if (current_fun->parents_argv == NULL)
                current_fun->parents_argv = new std::vector<std::pair<std::string, Var_attr *>>;
            temp = current_fun->belong->locvars->find(this->name)->second;
            current_fun->parents_argv->push_back(std::pair<std::string, Var_attr *>(this->name, temp));
        }
        else if (glovars.find(this->name) != glovars.end())
        {
            temp = glovars.find(this->name)->second;
        }
        else
        {
            printf("No definition of %s\n", this->name.c_str());
            return error = true;
        }

        if (temp->dim != this->child_num)
        {
            printf("Dimention not match of %s\n", this->name.c_str());
            return error = true;
        }
        this->dtype = temp->dtype;
        for (int _ = 0; _ < this->child_num; _++)
        {
            if (this->children->at(_)->ntype == Type::lvalue)
            {
                this->children->at(_)->CheckTable(current_fun);
            }
        }
        break;
    }
    case Type::expr:
    {
        if (this->name[0] == '_')
        {
            Fun_attr *temp = NULL;
            if (current_fun->locfuns->find(this->name) != current_fun->locfuns->end())
            {
                temp = current_fun->locfuns->find(this->name)->second;
            }
            else if (funs.find(this->name) != funs.end())
            {
                temp = funs.find(this->name)->second;
            }
            else
            {
                printf("No definition of function %s\n", this->name.c_str());
                return error = true;
            }

            this->dtype = temp->rtype;
            if (this->child_num == 0)
            {
                if (temp->argc != 0)
                {
                    printf("No definition of %s()\n", this->name.c_str());
                    return error = true;
                }
            }
            else if (this->children->at(0)->child_num != temp->argc)
            {
                printf("Number not match of %s\n", this->name.c_str());
                return error = true;
            }
            else
            {
                this->children->at(0)->CheckTable(current_fun);
                for (int _ = 0; _ < temp->argc; _++)
                {
                    if (this->children->at(0)->children->at(_)->dtype != (temp->argv->at(_)).first)
                    {
                        printf("Type not match on %s of %s\n", (temp->argv->at(_)).second.c_str(), this->name.c_str());
                        return error = true;
                    }
                }
            }
        }
        else if (this->name == "return")
        {
            if (this->child_num == 0)
            {
                if (current_fun->rtype != DataType::vvoid)
                {
                    printf("Return Type not match of %s\n", current_fun->name.c_str());
                    return error = true;
                }
            }
            else if (this->children->at(0)->ntype == Type::expr)
            {
                this->children->at(0)->CheckTable(current_fun);
                if (current_fun->rtype != this->children->at(0)->dtype)
                {
                    printf("Return type not match of %s\n", current_fun->name.c_str());
                    return error = true;
                }
            }
            else if (this->children->at(0)->ntype == Type::var)
            {
                if (current_fun->locfuns->find(this->children->at(0)->name) == current_fun->locfuns->end())
                {
                    printf("Return function %s not exist\n", this->children->at(0)->name.c_str());
                    return error = true;
                }
            }
        }
        else if (this->name == "scanf")
        {
            this->children->at(1)->CheckTable(current_fun);
        }
        else if (this->name == "printf")
        {
            if (this->child_num > 1)
            {
                this->children->at(1)->CheckTable(current_fun);
            }
        }
        else if (this->name == "Fas_Exp")
        {
            this->children->at(1)->CheckTable(current_fun);
            if (this->children->at(1)->ntype == Type::var)
            {
                if (current_fun->locfuns->find(this->children->at(1)->name) == current_fun->locfuns->end())
                {
                    printf("No implement of %s\n", this->children->at(1)->name.c_str());
                    return error = true;
                }
                (*(current_fun->locfuns))[this->children->at(0)->name] = (*(current_fun->locfuns))[this->children->at(1)->name];
            }
        }
        else if (this->name == "As_Exp")
        {
            this->children->at(0)->CheckTable(current_fun);
            this->children->at(1)->CheckTable(current_fun);
            if (this->children->at(0)->dtype != this->children->at(1)->dtype)
            {
                printf("Assign type not match of %s\n", this->children->at(0)->name.c_str());
                return error = true;
            }
        }
        else if (this->name == "Op_Exp")
        {
            this->dtype = DataType::nonedt;
            for (int _ = 0; _ < this->child_num; _ += 2)
            {
                this->children->at(_)->CheckTable(current_fun);
                if (this->children->at(_)->dtype != DataType::integer)
                {
                    puts("Operand Type Error !");
                    return error = true;
                }
            }
            this->dtype = DataType::integer;
        }
        else if (this->name == "Op_Term")
        {
            this->dtype = DataType::nonedt;
            for (int _ = 0; _ < this->child_num; _ += 2)
            {
                this->children->at(_)->CheckTable(current_fun);
                if (this->children->at(_)->dtype != DataType::integer)
                {
                    if (!(this->child_num == 1 && this->children->at(0)->name[0] == '_'))
                    {
                        puts("Operand Type Error !");
                        return error = true;
                    }
                }
            }
            this->dtype = DataType::integer;
        }
        else if (this->name == "Cond_Exp")
        {
            this->dtype = DataType::nonedt;
            for (int _ = 0; _ < this->child_num; _ += 2)
            {
                this->children->at(_)->CheckTable(current_fun);
                if (this->children->at(_)->dtype != DataType::integer)
                {
                    puts("Operand Type Error !");
                    return error = true;
                }
            }
            this->dtype = DataType::integer;
        }
        else if (this->name == "Cond_Term")
        {
            this->dtype = DataType::nonedt;
            for (int _ = 0; _ < this->child_num; _ += 2)
            {
                this->children->at(_)->CheckTable(current_fun);
                if (this->children->at(_)->dtype != DataType::integer)
                {
                    puts("Operand Type Error !");
                    return error = true;
                }
            }
            this->dtype = DataType::integer;
        }
        else if (this->name == "Cond_Factor")
        {
            this->dtype = DataType::nonedt;
            this->children->at(0)->CheckTable(current_fun);
            if (this->children->at(0)->dtype != DataType::integer)
            {
                puts("Operand Type Error !");
                return error = true;
            }
            this->children->at(2)->CheckTable(current_fun);
            if (this->children->at(2)->dtype != DataType::integer)
            {
                puts("Operand Type Error !");
                return error = true;
            }
            this->dtype = DataType::integer;
        }
        else if (this->name == "If_Stmt")
        {
            this->dtype = DataType::nonedt;
            this->children->at(0)->CheckTable(current_fun);
            if (this->children->at(0)->dtype != DataType::integer)
            {
                puts("Operand Type Error !");
                return error = true;
            }
            this->children->at(1)->CheckTable(current_fun);
            this->dtype = DataType::integer;
        }
        else if (this->name == "Lop_Stmt")
        {
            this->dtype = DataType::nonedt;
            this->children->at(0)->CheckTable(current_fun);
            if (this->children->at(0)->dtype != DataType::integer)
            {
                puts("Operand Type Error !");
                return error = true;
            }
            this->children->at(1)->CheckTable(current_fun);
            this->dtype = DataType::integer;
        }
        break;
    }
    default:
        break;
    }
    return error;
}
