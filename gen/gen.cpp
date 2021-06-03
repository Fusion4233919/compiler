#include "gen.h"
#include "AST.h"

namespace gen {

    class ValueWrapper {

    };
    
    llvm::LLVMContext llvmContext;
    llvm::Module llvmModule("compiler_module", llvmContext);
    llvm::IRBuilder<> irBuilder(llvmContext);
    static std::unique_ptr<llvm::LLVMContext> TheContext;
    static std::unique_ptr<llvm::Module> TheModule;
    static std::unique_ptr<llvm::IRBuilder<>> Builder;
    static std::map<std::string, ValueWrapper*> NamedValues;

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

    static void SingleGlobalDeclGen(char* Name, DataType Type) {
        llvm::Constant *InitConst;
        llvm::Value *Value;
        if (Type == DataType::integer) {
            llvm::Type *llvmType = llvm::Type::getInt32Ty(*TheContext);
            InitConst = llvm::dyn_cast<llvm::Constant>(llvm::ConstantInt::get(llvmType, 0, true));
            Value = new llvm::GlobalVariable(llvmModule, llvmType, false, llvm::GlobalValue::CommonLinkage, InitConst, std::string(Name));
        } else if (Type == DataType::string) {

        }
        ValueWrapper *wvalue = new ValueWrapper(Value, Type);
        NamedValues[std::string(Name)] = wvalue;
    }

    static void ArrayGlobalDeclGen(char* Name, DataType Type, int Len) {

    }

    static void GlobalVarDeclGen(AST *Def) {
        auto Type = Def->children->at(0)->dtype;
        auto* VarList = Def->children->at(1);

        for (auto* Var : *(VarList->children)) {
            if (Var->child_num == 0) {
                SingleGlobalDeclGen(Var->name, Type);
            } else {
                ArrayGlobalDeclGen(Var->name, Type, Var->children->at(0)->dvalue.integer);
            }
        }
    }

    static void GlobalVarDeclListGen(AST *DefList) {
        for (auto* Def : *(DefList->children)) {
            GlobalVarDeclGen(Def);
        }
    }

    static void ProgramGen(AST *node) {
        AST *DefList = node->children->at(0);
        AST *FunDefList = node->children->at(1);

        GlobalVarDeclListGen(DefList);

        for (auto* FunDef : *(FunDefList->children)) {
            if (strcmp(FunDef->name, "_main")) {
                MainFuncAPICalls();
                FuncGen(FunDef);
                return;
            }
            FuncGen(FunDef);
        }
    }
}