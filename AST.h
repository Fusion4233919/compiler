/************************************
    Name:        AST.h 
    Version:     v1.1
    Modefied by: fusion
                 2021-5-26 17:08
************************************/

#ifndef AST_H
#define AST_H
#include <vector>

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
    exp
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
    char* str;
} Value;

class AST
{
public:
    static int ID;
    int id;
    Type ntype;
    char name[32];  /* only for var */
    DataType dtype;
    Value dvalue;   /* only for const */
    Operator op;    /* only for operator */ 
    int child_num;
    std::vector<AST*> *children;

    AST(Type type, const char* name);
    AST(int value);
    AST(char *value);
    ~AST();
    void Insert(AST*);
};

extern AST *head;

#endif