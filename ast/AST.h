/************************************
    Name:        AST.h 
    Version:     v3.0
    Modefied by: fusion
                 2021-6-5 14:20
************************************/

#ifndef AST_H
#define AST_H
#include <vector>
#include <string>
#include <map>

typedef enum Type
{
    none,
    func,
    cconst,
    var,
    fvar,
    list,
    opr,
    tydf,
    expr,
    lvalue
} Type;

typedef enum Operator
{
    noneop,
    OR,
    AD,
    EQ,
    LE,
    GE,
    NE,
    LT,
    GT,
    add,
    min,
    mul,
    ddi,
    mod,
} Operator;

typedef enum DataType
{
    nonedt,
    vvoid,
    integer,
    string,
} DataType;

typedef union Value
{
    int integer;
    char *str;
} Value;

class Var_attr;
class Fun_attr;
class AST;
typedef std::map<std::string, Var_attr *> Vmap;
typedef std::map<std::string, Fun_attr *> Fmap;
extern AST *head;
extern Vmap glovars;
extern Fmap funs;

class Var_attr
{
public:
    std::string name;
    int dim;
    DataType dtype;
    Fun_attr *belong;
    std::vector<int> *dimention;

    Var_attr(std::string name, DataType dtype)
    {
        this->dim = 0;
        this->name = name;
        this->dtype = dtype;
        this->belong = NULL;
        this->dimention = NULL;
    }
    ~Var_attr();
};

class Fun_attr
{
public:
    std::string name;
    int argc;
    DataType rtype;
    Vmap *locvars;
    std::vector<std::pair<DataType, std::string> > *argv;

    Fun_attr(std::string name, DataType rtype)
    {
        this->name = name;
        this->argc = 0;
        this->rtype = rtype;
        this->locvars = new Vmap;
        this->argv = NULL;
    }
    ~Fun_attr() { delete this->locvars; delete this->argv;}
};

class AST
{
public:
    static int IDAccumulate;
    int id;
    Type ntype;
    std::string name; /* only for var */
    DataType dtype;
    Value dvalue; /* only for const */
    Operator op;  /* only for operator */
    int child_num;
    std::vector<AST *> *children;

    AST(Type type, const char *name);
    AST(int value);
    AST(char *value);
    ~AST();
    void Insert(AST *);
    void BuildTable(Fun_attr *);
    bool CheckTable(Fun_attr *);
    void print(void);
};

#endif
