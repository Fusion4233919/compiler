#include "gen.h"

namespace gen {
    struct ValueWrapper {
        llvm::Value *value = nullptr;
        DataType type = DataType::vvoid;

        ValueWrapper(llvm::Value *value, DataType type) {
            this->value = value;
            this->type = type;
        }

        ValueWrapper(llvm::Value *value, DataType type, bool isRet) {
            this->value = value;
            this->type = type;
        }
    };

    struct FunctionWrapper {
        llvm::Function *func = nullptr;
        DataType retType = DataType::vvoid;

        FunctionWrapper(llvm::Function *func, DataType retType) {
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

    static ValueWrapper *GetLValue(AST *LValueNode) {
        if (LValueNode->child_num == 0) {
            return NamedValues[std::string(LValueNode->name)];
        } else {
            // TODO array
        }
    }

    static ValueWrapper *OpExprGen(AST *Expr);

    static void AssignGen(AST *Expr) {
        auto *LValueNode = Expr->children->at(0);
        auto *OpExpr = Expr->children->at(1);

        ValueWrapper *LValue = GetLValue(LValueNode);
        auto *OpResult = OpExprGen(OpExpr);
        irBuilder.CreateStore(OpResult->value, LValue->value);
    }

    static ValueWrapper *CallGen(AST *Expr);

    static ValueWrapper *OpFactorGen(AST *Factor) {
        if (Factor->ntype == Type::expr && strcmp(Factor->name, "Op_Exp") == 0) {
            return OpExprGen(Factor);
        } else if (Factor->ntype == Type::cconst && Factor->dtype == DataType::integer) {
            return new ValueWrapper(llvm::ConstantInt::get(
                    GetLLVMType(DataType::integer),
                    Factor->dvalue.integer),DataType::integer);
        } else if (Factor->ntype == Type::lvalue) {
            auto *LValue = GetLValue(Factor);
            auto *Loaded = irBuilder.CreateLoad(LValue->value);
            return new ValueWrapper(Loaded, LValue->type);
        } else if (Factor->ntype == Type::expr) {
            auto *FuncRet = CallGen(Factor);
            return new ValueWrapper(FuncRet->value, FuncRet->type);
        }
    }

    static ValueWrapper *OpTermGen(AST *Term) {
        auto ChildIter = Term->children->begin();
        llvm::Value *Acc = OpFactorGen(*ChildIter)->value;
        ++ChildIter;
        while (ChildIter != Term->children->end()) {
            switch ((*ChildIter)->op) {
                case Operator::mul:
                    ++ChildIter;
                    Acc = irBuilder.CreateMul(Acc, OpFactorGen(*ChildIter)->value);
                    ++ChildIter;
                    break;
                case Operator::ddi:
                    ++ChildIter;
                    Acc = irBuilder.CreateSDiv(Acc, OpFactorGen(*ChildIter)->value);
                    ++ChildIter;
                    break;
                case Operator::mod:
                    ++ChildIter;
                    Acc = irBuilder.CreateSRem(Acc, OpFactorGen(*ChildIter)->value);
                    ++ChildIter;
                    break;
                default:
                    break;
            }
        }
        return new ValueWrapper(Acc, DataType::integer);
    }

    static ValueWrapper *OpExprGen(AST *Expr) {
        auto ChildIter = Expr->children->begin();
        llvm::Value *Acc = OpTermGen(*ChildIter)->value;
        ++ChildIter;
        while (ChildIter != Expr->children->end()) {
            switch ((*ChildIter)->op) {
                case Operator::add:
                    ++ChildIter;
                    Acc = irBuilder.CreateAdd(Acc, OpTermGen(*ChildIter)->value);
                    ++ChildIter;
                    break;
                case Operator::min:
                    ++ChildIter;
                    Acc = irBuilder.CreateSub(Acc, OpTermGen(*ChildIter)->value);
                    ++ChildIter;
                    break;
                default:
                    break;
            }
        }
        return new ValueWrapper(Acc, DataType::integer);
    }

    static ValueWrapper *CondExprGen(AST *Expr);

    static ValueWrapper *CondFactorGen(AST *Factor) {
        if (Factor->ntype == Type::expr && strcmp(Factor->name, "Cond_Exp") == 0) {
            return CondExprGen(Factor);
        } else {
            auto *LHS = OpExprGen(Factor->children->at(0));
            auto *CondOp = Factor->children->at(1);
            auto *RHS = OpExprGen(Factor->children->at(2));

            switch (CondOp->op) {
                case Operator::EQ:
                    return new ValueWrapper(irBuilder.CreateICmpEQ(LHS->value, RHS->value), DataType::integer);
                case Operator::GE:
                    return new ValueWrapper(irBuilder.CreateICmpSGE(LHS->value, RHS->value), DataType::integer);
                case Operator::LE:
                    return new ValueWrapper(irBuilder.CreateICmpSLE(LHS->value, RHS->value), DataType::integer);
                case Operator::NE:
                    return new ValueWrapper(irBuilder.CreateICmpNE(LHS->value, RHS->value), DataType::integer);
                case Operator::LT:
                    return new ValueWrapper(irBuilder.CreateICmpSLT(LHS->value, RHS->value), DataType::integer);
                case Operator::GT:
                    return new ValueWrapper(irBuilder.CreateICmpSGT(LHS->value, RHS->value), DataType::integer);
            }
        }
    }

    static ValueWrapper *CondTermGen(AST *Term) {
        auto ChildIter = Term->children->begin();
        llvm::Value *Acc = CondFactorGen(*ChildIter)->value;
        ++ChildIter;
        while (ChildIter != Term->children->end()) {
            ++ChildIter;
            Acc = irBuilder.CreateAnd(Acc, CondFactorGen(*ChildIter)->value);
            ++ChildIter;
        }
        return new ValueWrapper(Acc, DataType::integer);
    }

    static ValueWrapper *CondExprGen(AST *Expr) {
        auto ChildIter = Expr->children->begin();
        llvm::Value *Acc = CondTermGen(*ChildIter)->value;
        ++ChildIter;
        while (ChildIter != Expr->children->end()) {
            ++ChildIter;
            Acc = irBuilder.CreateOr(Acc, CondTermGen(*ChildIter)->value);
            ++ChildIter;
        }
        return new ValueWrapper(Acc, DataType::integer);
    }

    static void ExprListGen(AST *ExprList);

    static void IfGen(AST *Expr) {
        auto *CondExpr = Expr->children->at(0);
        auto *ExprList = Expr->children->at(1);

        auto *CondResult = CondExprGen(CondExpr);
        auto *CurrentFunc = irBuilder.GetInsertBlock()->getParent();
        llvm::BasicBlock *Then = llvm::BasicBlock::Create(llvmContext, "then", CurrentFunc);
        llvm::BasicBlock *Merge = llvm::BasicBlock::Create(llvmContext, "merge", CurrentFunc);
        irBuilder.CreateCondBr(CondResult->value, Then, Merge);

        irBuilder.SetInsertPoint(Then);
        ExprListGen(ExprList);
        irBuilder.CreateBr(Merge);

        irBuilder.SetInsertPoint(Merge);
    }

    static void LopGen(AST *Expr) {
        auto *CondExpr = Expr->children->at(0);
        auto *ExprList = Expr->children->at(1);

        auto *CurrentFunc = irBuilder.GetInsertBlock()->getParent();
        llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(llvmContext, "loop", CurrentFunc);
        llvm::BasicBlock *CondBB = llvm::BasicBlock::Create(llvmContext, "cond", CurrentFunc);
        llvm::BasicBlock *Merge = llvm::BasicBlock::Create(llvmContext, "merge", CurrentFunc);
        irBuilder.CreateBr(CondBB);

        irBuilder.SetInsertPoint(LoopBB);
        ExprListGen(ExprList);
        irBuilder.CreateBr(CondBB);
        irBuilder.SetInsertPoint(CondBB);
        auto *CondResult = CondExprGen(CondExpr);
        irBuilder.CreateCondBr(CondResult->value, LoopBB, Merge);
        irBuilder.SetInsertPoint(Merge);
    }

    static void InputGen(AST *Expr) {

    }

    static void OutputGen(AST *Expr) {

    }

    static void BreakGen(AST *Expr) {

    }

    static void ContGen(AST *Expr) {

    }

    static void RetGen(AST *Expr) {

    }

    static ValueWrapper *CallGen(AST *Expr) {
        auto *CalleeFunc = NamedFuncs[std::string(Expr->name)];
        auto *ArgList = Expr->children->at(0);

        std::vector<llvm::Value *> ArgValues;
        for (auto *Arg : *(ArgList->children)) {
            if (Arg->ntype == Type::cconst && Arg->dtype == DataType::integer) {
                ArgValues.push_back(llvm::ConstantInt::get(
                        GetLLVMType(DataType::integer),
                        Arg->dvalue.integer));
            } else if (Arg->ntype == Type::lvalue) {
                auto *LValue = GetLValue(Arg);
                auto *Loaded = irBuilder.CreateLoad(LValue->value);
                ArgValues.push_back(Loaded);
            } else {
                // TODO string?
            }
        }

        return new ValueWrapper(irBuilder.CreateCall(CalleeFunc->func, ArgValues), CalleeFunc->retType);
    }

    static void ExprGen(AST *Expr) {
        if (strcmp(Expr->name, "As_Exp") == 0) {
            AssignGen(Expr);
        } else if (strcmp(Expr->name, "Op_Exp") == 0) {
            OpExprGen(Expr);
        } else if (strcmp(Expr->name, "Cond_Exp") == 0) {
            CondExprGen(Expr);
        } else if (strcmp(Expr->name, "If_Stmt") == 0) {
            IfGen(Expr);
        } else if (strcmp(Expr->name, "Lop_Stmt") == 0) {
            LopGen(Expr);
        } else if (strcmp(Expr->name, "scanf") == 0) {
            InputGen(Expr);
        } else if (strcmp(Expr->name, "printf") == 0) {
            OutputGen(Expr);
        } else if (strcmp(Expr->name, "break") == 0) {
            BreakGen(Expr);
        } else if (strcmp(Expr->name, "continue") == 0) {
            ContGen(Expr);
        } else if (strcmp(Expr->name, "return") == 0) {
            RetGen(Expr);
        } else {
            CallGen(Expr);
        }
    }

    static void ExprListGen(AST *ExprList) {
        for (auto* Expr : *(ExprList->children)) {
            ExprGen(Expr);
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
            Func = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", &llvmModule);
        } else {
            Func = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, std::string(FunDefNode->name), &llvmModule);
        }
        NamedFuncs[std::string(FunDefNode->name)] = new FunctionWrapper(Func, FunDataType);

        llvm::BasicBlock *BB = llvm::BasicBlock::Create(llvmContext, "entry", Func);
        irBuilder.SetInsertPoint(BB);

        int index = 0;
        for (auto &Arg : Func->args()) {
            auto *Var = FunVarList->children->at(index);
            auto *name = Var->name;
            auto type = Var->children->at(0)->dtype;
            CreateLocalVariable(name, type);
            Arg.setName(std::string(name));
            irBuilder.CreateStore(&Arg, NamedValues[std::string(name)]->value);
        }

        LocalVarDeclListGen(DefList);

        ExprListGen(ExpList);

        if (FunDataType == DataType::vvoid) {
            irBuilder.CreateRetVoid();
        }

        llvm::verifyFunction(*Func);
    }

    void ProgramGen(AST *node) {
        AST *DefList = node->children->at(0);
        AST *FunDefList = node->children->at(1);

        GlobalVarDeclListGen(DefList);

        for (auto* FunDef : *(FunDefList->children)) {
            FuncGen(FunDef);
        }

        std::error_code EC;
        llvm::raw_fd_ostream llout("mhl.ll", EC);
        llvmModule.print(llout, nullptr);
    }
}