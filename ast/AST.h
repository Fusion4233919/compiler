/************************************
    Name:        AST.h 
    Version:     v1.3
    Modefied by: fusion
                 2021-5-28 10:58
************************************/

#ifndef AST_H
#define AST_H
#include <vector>
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

class Var_attr
{
public:
    char *name;
    int ref;
    int dim;
    DataType dtype;
    std::vector<int> *dimention;

    Var_attr(char *name, DataType dtype)
    {
        this->ref = 0;
        this->dim = 0;
        this->name = name;
        this->dtype = dtype;
        this->dimention = NULL;
    }
    ~Var_attr();
};

typedef std::map<char *, Var_attr *> Vmap;

class Fun_attr
{
public:
    char *name;
    DataType rtype;
    Vmap *locvars;

    Fun_attr(char *name, DataType rtype)
    {
        this->name = name;
        this->rtype = rtype;
        this->locvars = new Vmap;
    }
    ~Fun_attr() { delete this->locvars; }
};

class AST
{
public:
    static int IDAccumulate;
    int id;
    Type ntype;
    char name[32]; /* only for var */
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
    void BuildTable(Vmap *);
    void print(void);
};

typedef std::map<char *, Fun_attr *> Fmap;
extern AST *head;
extern Vmap glovars;
extern Fmap funs;

#endif