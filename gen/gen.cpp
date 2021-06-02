#include "gen.h"
#include "AST.h"

namespace gen {
    
    llvm::LLVMContext llvmContext;
    llvm::Module llvmModule("compiler_module", llvmContext);
    llvm::IRBuilder<> irBuilder(llvmContext);
    static std::unique_ptr<llvm::LLVMContext> TheContext;
    static std::unique_ptr<llvm::Module> TheModule;
    static std::unique_ptr<llvm::IRBuilder<>> Builder;
    static std::map<std::string, llvm::AllocaInst *> NamedValues;
    static std::map<std::string, llvm::AllocaInst *> GlobalNamedValues;

    static llvm::AllocaInst *CreateEntryBlockAlloca(llvm::BasicBlock *EntryBlock, AST* TypeNode, AST* Var) {
        llvm::IRBuilder<> TempBuilder(EntryBlock, EntryBlock->begin());
        llvm::ArrayType *ArrayType = nullptr;
        if (Var->child_num > 0) {
            // array
            int Len = Var->children->at(0)->dvalue.integer;
            llvm::Type *I = llvm::IntegerType::getInt32Ty(*TheContext);
            ArrayType = llvm::ArrayType::get(I, Len);
        }
        // TODO
        return TempBuilder.CreateAlloca(llvm::Type::getInt32Ty(*TheContext), ArrayType, std::string(Var->name));
    }

    static llvm::Value *programCodeGen(const AST *node) {
        AST *DefList = node->children->at(0);
        AST *FunDefList = node->children->at(1);

        llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry");
        Builder->SetInsertPoint(BB);

        GlobalNamedValues.clear();
        for (AST *Def : *(DefList->children)) {
            AST *TypeNode = Def->children->at(0);
            AST *VarList = Def->children->at(1);

            for (AST *Var : *(VarList->children)) {
                if (TypeNode->dtype == DataType::integer) {
                    llvm::AllocaInst *Alloca = CreateEntryBlockAlloca(BB, TypeNode, Var);
                    GlobalNamedValues[std::string(Var->name)] = Alloca;
                }
            }
        }

        
    }

    static llvm::Value *varDefCodeGen(const AST *node) {
        return nullptr;
    }

    static llvm::Value *funcCodeGen(const AST *node) {
        return nullptr;
    }

    static llvm::Value *litCodeGen(const AST *node) {
        return nullptr;
    }

    static llvm::Value *declCodeGen(const AST *node) {
        return nullptr;
    }

    static llvm::Value *opCodeGen(const AST *node) {
        return nullptr;
    }

    static llvm::Value *funcAttrCodeGen(const AST *node) {
        return nullptr;
    }

    static llvm::Value *listCodeGen(const AST *node) {
        return nullptr;
    }


    static llvm::Value *expCodeGen(const AST *node) {
        return nullptr;
    }

    static llvm::Value *lvalueCodeGen(const AST *node) {
        return nullptr;
    }

    static llvm::Value *codeGen(const AST *node) {
        switch (node->ntype) {
            case none:
                if (strcmp(node->name, "program") == 0) {
                    return programCodeGen(node);
                } else if (strcmp(node->name, "Def")) {
                    return varDefCodeGen(node);
                }
                break;
            case func:
                return funcCodeGen(node);
            case cconst:
                return litCodeGen(node);
            case var:
                return declCodeGen(node);
            case fvar:
                return funcAttrCodeGen(node);
            case list:
                return listCodeGen(node);
            case opr:
                return opCodeGen(node);
            case Type::exp:
                return expCodeGen(node);
            case lvalue:
                return lvalueCodeGen(node);
        }
        return nullptr;
    }

}