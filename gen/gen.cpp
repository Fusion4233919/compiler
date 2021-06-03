#include "gen.h"
#include "AST.h"

namespace gen {

    struct ValueWrapper {
        llvm::Value *value = nullptr;
        DataType type = DataType::vvoid;

        ValueWrapper(llvm::Value *value, DataType type) {
            this->value = value;
            this->type = type;
        }
    };

    struct FunctionWrapper {
        llvm::Function *func = nullptr;
        llvm::Type *retType = nullptr;

        FunctionWrapper(llvm::Function *func, llvm::Type* retType) {
            this->func = func;
            this->retType = retType;
        }
    };
    
    llvm::LLVMContext llvmContext;
    llvm::Module llvmModule("compiler_module", llvmContext);
    llvm::IRBuilder<> irBuilder(llvmContext);
    static std::map<std::string, ValueWrapper*> NamedValues;
    static std::map<std::string, FunctionWrapper*> NamedFuncs;

    static llvm::Type* GetLLVMType(DataType Type) {
        if (Type == DataType::integer) {
            return llvm::IntegerType::getInt32Ty(llvmContext);
        } else {
            // TODO: String?
        }
    }

    static void SingleGlobalDeclGen(char* Name, DataType Type) {
        llvm::Constant *InitConst;
        llvm::Value *Value;
        if (Type == DataType::integer) {
            llvm::Type *llvmType = llvm::Type::getInt32Ty(llvmContext);
            InitConst = llvm::dyn_cast<llvm::Constant>(llvm::ConstantInt::get(llvmType, 0, true));
            Value = new llvm::GlobalVariable(llvmModule, llvmType, false, llvm::GlobalValue::CommonLinkage, InitConst, std::string(Name));
        } else if (Type == DataType::string) {
            // TODO: string
        }
        auto *wvalue = new ValueWrapper(Value, Type);
        NamedValues[std::string(Name)] = wvalue;
    }

    static void ArrayGlobalDeclGen(char* Name, DataType Type, int Len) {
        // TODO: array
    }

    static void CreateLocalVariable(char *Name, DataType Type) {
        llvm::Type *Ty = GetLLVMType(Type);
        // TODO: array
        llvm::Value *Value = irBuilder.CreateAlloca(Ty, nullptr, std::string(Name));
        auto *wvalue = new ValueWrapper(Value, Type);
        NamedValues[std::string(Name)] = wvalue;
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

    static void SingleLocalDeclGen(char* Name, DataType Type) {
        CreateLocalVariable(Name, Type);
        auto *wvalue = new ValueWrapper(NamedValues[std::string(Name)]->value, Type);
        NamedValues[std::string(Name)] = wvalue;
    }

    static void ArrayLocalDeclGen(char* Name, DataType Type, int Len) {
        // TODO: array
    }

    static void LocalVarDeclGen(AST* Def) {
        auto Type = Def->children->at(0)->dtype;
        auto *VarList = Def->children->at(1);

        for (auto* Var : *(VarList->children)) {
            if (Var->child_num == 0) {
                SingleLocalDeclGen(Var->name, Type);
            } else {
                ArrayLocalDeclGen(Var->name, Type, Var->children->at(0)->dvalue.integer);
            }
        }
    }

    static void LocalVarDeclListGen(AST *DefList) {
        for (auto* Def : *(DefList->children)) {
            LocalVarDeclGen(Def);
        }
    }

    static void FuncGen(AST *FunDefNode) {
        auto FunDataType = FunDefNode->children->at(0)->dtype;
        auto *FunType = FunDataType == DataType::vvoid ?
                        llvm::Type::getVoidTy(llvmContext) : GetLLVMType(FunDefNode->children->at(0)->dtype);
        auto *FunVarList = FunDefNode->children->at(1);
        auto *DefList = FunDefNode->children->at(2);
        auto *ExpList = FunDefNode->children->at(3);

        std::vector<llvm::Type *> TypeVector;
        if (FunVarList->child_num != 1 || !(FunVarList->children->at(0)->ntype == Type::tydf
            && strcmp(FunVarList->children->at(0)->name, "void") == 0)) {
            // not void parameter
            for (auto* FunVar : *(FunVarList->children)) {
                auto Type = FunVar->children->at(0)->dtype;
                if (Type == DataType::integer) {
                    TypeVector.push_back(llvm::Type::getInt32Ty(llvmContext));
                } else if (Type == DataType::string) {
                    // TODO: string
                }
            }
        }

        llvm::FunctionType *FT = llvm::FunctionType::get(FunType, TypeVector, false);
        llvm::Function *Func;
        if (strcmp(FunDefNode->name, "_main") == 0) {
            Func = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", llvmModule);
        } else {
            Func = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, std::string(FunDefNode->name), llvmModule);
        }
        NamedFuncs[FunDefNode->name] = new FunctionWrapper(Func, FunType);

        llvm::BasicBlock *BB = llvm::BasicBlock::Create(llvmContext, "entry", Func);
        irBuilder.SetInsertPoint(BB);

        int index = 0;
        for (auto &Arg : Func->args()) {
            auto *Var = FunVarList->children->at(index);
            auto *name = Var->name;
            auto type = Var->dtype;
            CreateLocalVariable(name, type);
            Arg.setName(std::string(name));
            irBuilder.CreateStore(&Arg, NamedValues[std::string(name)]->value);
        }

        LocalVarDeclListGen(DefList);

        auto* RetValue = ExpListGen(ExpList);

        llvm::verifyFunction(*Func);

        if (FunDataType == DataType::vvoid) {
            irBuilder.CreateRetVoid();
        } else {
            irBuilder.CreateRet(RetValue);
        }
    }

    static void ProgramGen(AST *node) {
        AST *DefList = node->children->at(0);
        AST *FunDefList = node->children->at(1);

        GlobalVarDeclListGen(DefList);

        for (auto* FunDef : *(FunDefList->children)) {
            if (strcmp(FunDef->name, "_main")) {
                FuncGen(FunDef);
            }
            FuncGen(FunDef);
        }
    }
}