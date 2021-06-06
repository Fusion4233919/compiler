#include "gen.h"
#include <iostream>

extern Fmap funs;

namespace gen {
    struct ValueWrapper {
        llvm::Value *value = nullptr;
        DataType type = DataType::vvoid;
        std::vector<int> shape;
        llvm::ArrayType* arrayType;
        std::string belong = "";

        ValueWrapper(llvm::Value *value, DataType type, std::string belong = "") {
            this->value = value;
            this->type = type;
            this->belong = belong;
        }

        ValueWrapper(llvm::Value *value, DataType type, std::vector<AST *> lenArray, llvm::ArrayType *arrayType) {
            this->value = value;
            this->type = type;
            this->arrayType = arrayType;
            for (AST *node : lenArray) {
                shape.push_back(node->dvalue.integer);
            }
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
    static std::map<std::string, ValueWrapper*> NamedGlobalValues;
    static std::map<std::string, ValueWrapper*> NamedLocalValues;
    static std::map<std::string, FunctionWrapper*> NamedFuncs;

    static llvm::Type* GetLLVMType(DataType Type) {
        if (Type == DataType::integer) {
            return llvm::IntegerType::getInt32Ty(llvmContext);
        } else {
            // TODO: String?
        }
    }

    static ValueWrapper *GetLValue(AST *LValueNode);

    static ValueWrapper *GetNamedValue(std::string Name, std::string FuncName = "") {
        auto FindLocal = NamedLocalValues.find(Name);
        if (FindLocal != NamedLocalValues.end()) {
            return (*FindLocal).second;
        }
        auto FindGlobal = NamedGlobalValues.find(Name);
        if (((*FindGlobal).second->belong == FuncName ||
            (*FindGlobal).second->belong == "") &&
            FindGlobal != NamedGlobalValues.end()) {
            return (*FindGlobal).second;
        }
        std::cerr << "No such variable " + Name + " in " + FuncName << std::endl;
        exit(2);
    }

    static ValueWrapper *GetArrayElePtr(std::string Name, std::vector<AST *> *EleIndexList) {
        auto *ArrayRef = GetNamedValue(Name);
        auto *IndexValue = dynamic_cast<llvm::Value *>(llvm::ConstantInt::get(GetLLVMType(integer), 0, true));
        auto *AccValue = dynamic_cast<llvm::Value *>(llvm::ConstantInt::get(GetLLVMType(integer), 1, true));
        auto ArrayShapeItr = ArrayRef->shape.rbegin();
        std::vector<llvm::Value *> IdxVec;
        IdxVec.push_back(llvm::ConstantInt::get(GetLLVMType(integer), 0, true));
        for (auto EleIndexItr = EleIndexList->rbegin(); EleIndexItr != EleIndexList->rend(); ++EleIndexItr) {
            llvm::Value *SelectorValue = nullptr;
            if ((*EleIndexItr)->ntype == Type::lvalue) {
                auto *Loaded = irBuilder.CreateLoad(GetLLVMType(integer), GetLValue(*EleIndexItr)->value, "ldtmp");
                SelectorValue = Loaded;
            } else {
                SelectorValue = llvm::ConstantInt::get(GetLLVMType(integer), (*EleIndexItr)->dvalue.integer, true);
            }
            llvm::Value *DimValue = llvm::ConstantInt::get(GetLLVMType(integer), *ArrayShapeItr, true);
            IndexValue = irBuilder.CreateAdd(IndexValue, irBuilder.CreateMul(AccValue, SelectorValue, "multmp"), "addtmp");
            AccValue = irBuilder.CreateMul(AccValue, DimValue);
            ++ArrayShapeItr;
        }
        IdxVec.push_back(IndexValue);
        auto *Value = irBuilder.CreateGEP(ArrayRef->value, IdxVec, "arrtmp");
        return new ValueWrapper(Value, integer);
    }

    static void SingleGlobalDeclGen(std::string Name, DataType Type) {
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
        NamedGlobalValues[std::string(Name)] = wvalue;
    }

    static void ArrayGlobalDeclGen(std::string Name, DataType Type, std::vector<AST *> *LenArray) {
        auto *LLVMType = GetLLVMType(Type);
        int sum = 1;
        for (AST* node : *(LenArray)) {
            sum *= node->dvalue.integer;
        }
        auto *ArrayType = llvm::ArrayType::get(LLVMType, sum);
        auto *Value = new llvm::GlobalVariable(llvmModule, ArrayType, false, llvm::GlobalValue::CommonLinkage, 0, Name);
        auto *ConstInit = llvm::ConstantAggregateZero::get(ArrayType);
        Value->setInitializer(ConstInit);
        NamedGlobalValues[Name] = new ValueWrapper(Value, Type, *LenArray, ArrayType);
    }

    static void CreateLocalVariable(std::string Name, DataType Type) {
        llvm::Type *Ty = GetLLVMType(Type);
        llvm::Value *Value = irBuilder.CreateAlloca(Ty, nullptr, std::string(Name));
        auto *wvalue = new ValueWrapper(Value, Type);
        NamedLocalValues[std::string(Name)] = wvalue;
    }

    static void GlobalVarDeclGen(AST *Def) {
        auto Type = Def->children->at(0)->dtype;
        auto* VarList = Def->children->at(1);

        for (auto* Var : *(VarList->children)) {
            if (Var->child_num == 0) {
                SingleGlobalDeclGen(Var->name, Type);
            } else {
                ArrayGlobalDeclGen(Var->name, Type, Var->children);
            }
        }
    }

    static void GlobalVarDeclListGen(AST *DefList) {
        for (auto* Def : *(DefList->children)) {
            GlobalVarDeclGen(Def);
        }
    }

    static void SingleLocalDeclGen(std::string Name, DataType Type) {
        CreateLocalVariable(Name, Type);
        auto *wvalue = new ValueWrapper(GetNamedValue(Name)->value, Type);
        NamedLocalValues[std::string(Name)] = wvalue;
    }

    static void ArrayLocalDeclGen(std::string Name, DataType Type, std::vector<AST *> *LenArray) {
        auto *LLVMType = GetLLVMType(Type);
        int sum = 1;
        for (AST* node : *(LenArray)) {
            sum *= node->dvalue.integer;
        }
        auto *ArrayType = llvm::ArrayType::get(LLVMType, sum);
        auto *LenValue = llvm::ConstantInt::get(GetLLVMType(integer), sum, true);
        auto *Value = irBuilder.CreateAlloca(ArrayType, LenValue, Name);
        NamedLocalValues[Name] = new ValueWrapper(Value, Type, *LenArray, ArrayType);
    }

    static void LocalVarDeclGen(AST* Def) {
        auto Type = Def->children->at(0)->dtype;
        auto *VarList = Def->children->at(1);

        for (auto* Var : *(VarList->children)) {
            if (Var->child_num == 0) {
                SingleLocalDeclGen(Var->name, Type);
            } else {
                ArrayLocalDeclGen(Var->name, Type, Var->children);
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
            auto CurrentFuncName = irBuilder.GetInsertBlock()->getParent()->getName().str();
            if (CurrentFuncName == "main") {
                CurrentFuncName = "_" + CurrentFuncName;
            }
            return GetNamedValue(LValueNode->name, CurrentFuncName);
        } else {
            return GetArrayElePtr(LValueNode->name, LValueNode->children);
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
        if (Factor->ntype == Type::expr && Factor->name == "Op_Exp") {
            return OpExprGen(Factor);
        } else if (Factor->ntype == Type::cconst && Factor->dtype == DataType::integer) {
            return new ValueWrapper(llvm::ConstantInt::get(
                    GetLLVMType(DataType::integer),
                    Factor->dvalue.integer),DataType::integer);
        } else if (Factor->ntype == Type::lvalue) {
            auto *LValue = GetLValue(Factor);
            auto *Loaded = irBuilder.CreateLoad(LValue->value, "tmp");
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
                    Acc = irBuilder.CreateMul(Acc, OpFactorGen(*ChildIter)->value, "multmp");
                    ++ChildIter;
                    break;
                case Operator::ddi:
                    ++ChildIter;
                    Acc = irBuilder.CreateSDiv(Acc, OpFactorGen(*ChildIter)->value, "divtmp");
                    ++ChildIter;
                    break;
                case Operator::mod:
                    ++ChildIter;
                    Acc = irBuilder.CreateSRem(Acc, OpFactorGen(*ChildIter)->value, "remtmp");
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
                    Acc = irBuilder.CreateAdd(Acc, OpTermGen(*ChildIter)->value, "addtmp");
                    ++ChildIter;
                    break;
                case Operator::min:
                    ++ChildIter;
                    Acc = irBuilder.CreateSub(Acc, OpTermGen(*ChildIter)->value, "subtmp");
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
        if (Factor->ntype == Type::expr && Factor->name == "Cond_Exp") {
            return CondExprGen(Factor);
        } else {
            auto *LHS = OpExprGen(Factor->children->at(0));
            auto *CondOp = Factor->children->at(1);
            auto *RHS = OpExprGen(Factor->children->at(2));

            switch (CondOp->op) {
                case Operator::EQ:
                    return new ValueWrapper(irBuilder.CreateICmpEQ(LHS->value, RHS->value, "cmptmp"), DataType::integer);
                case Operator::GE:
                    return new ValueWrapper(irBuilder.CreateICmpSGE(LHS->value, RHS->value, "cmptmp"), DataType::integer);
                case Operator::LE:
                    return new ValueWrapper(irBuilder.CreateICmpSLE(LHS->value, RHS->value, "cmptmp"), DataType::integer);
                case Operator::NE:
                    return new ValueWrapper(irBuilder.CreateICmpNE(LHS->value, RHS->value, "cmptmp"), DataType::integer);
                case Operator::LT:
                    return new ValueWrapper(irBuilder.CreateICmpSLT(LHS->value, RHS->value, "cmptmp"), DataType::integer);
                case Operator::GT:
                    return new ValueWrapper(irBuilder.CreateICmpSGT(LHS->value, RHS->value, "cmptmp"), DataType::integer);
            }
        }
    }

    static ValueWrapper *CondTermGen(AST *Term) {
        auto ChildIter = Term->children->begin();
        llvm::Value *Acc = CondFactorGen(*ChildIter)->value;
        ++ChildIter;
        while (ChildIter != Term->children->end()) {
            ++ChildIter;
            Acc = irBuilder.CreateAnd(Acc, CondFactorGen(*ChildIter)->value, "andtmp");
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
            Acc = irBuilder.CreateOr(Acc, CondTermGen(*ChildIter)->value, "ortmp");
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

    static llvm::BasicBlock *CurrentMerge = nullptr;
    static llvm::BasicBlock *CurrentCond = nullptr;

    static void LopGen(AST *Expr) {
        auto *CondExpr = Expr->children->at(0);
        auto *ExprList = Expr->children->at(1);

        auto *CurrentFunc = irBuilder.GetInsertBlock()->getParent();
        llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(llvmContext, "loop", CurrentFunc);
        llvm::BasicBlock *CondBB = llvm::BasicBlock::Create(llvmContext, "cond", CurrentFunc);
        llvm::BasicBlock *Merge = llvm::BasicBlock::Create(llvmContext, "merge", CurrentFunc);
        irBuilder.CreateBr(CondBB);
        auto *PrevMerge = CurrentMerge;
        auto *PrevCond = CurrentCond;
        CurrentMerge = Merge;
        CurrentCond = CondBB;

        irBuilder.SetInsertPoint(LoopBB);
        ExprListGen(ExprList);
        irBuilder.CreateBr(CondBB);

        irBuilder.SetInsertPoint(CondBB);
        auto *CondResult = CondExprGen(CondExpr);
        irBuilder.CreateCondBr(CondResult->value, LoopBB, Merge);

        irBuilder.SetInsertPoint(Merge);
        CurrentMerge = PrevMerge;
        CurrentCond = PrevCond;
    }

    static void InputGen(AST *Expr) {
        auto *ScanfFunc = NamedFuncs["scanf"]->func;

        std::vector<llvm::Value *> ArgValues;
        auto FormatStr = std::string(Expr->children->at(0)->dvalue.str);
        auto *FormatStrInst = irBuilder.CreateGlobalStringPtr(FormatStr, "scanf_format_str");
        ArgValues.push_back(FormatStrInst);

        if (Expr->child_num > 1) {
            auto *ArgList = Expr->children->at(1);

            for (auto *Arg : *(ArgList->children)) {
                if (Arg->ntype == Type::cconst && Arg->dtype == DataType::integer) {
                    ArgValues.push_back(llvm::ConstantInt::get(
                            GetLLVMType(DataType::integer),
                            Arg->dvalue.integer));
                } else if (Arg->ntype == Type::lvalue) {
                    auto *LValue = GetLValue(Arg);
                    ArgValues.push_back(LValue->value);
                } else {
                    // TODO string?
                }
            }
        }

        irBuilder.CreateCall(ScanfFunc, ArgValues, "c_scanf");

    }

    static void OutputGen(AST *Expr) {
        auto *PrintfFunc = NamedFuncs["printf"]->func;

        std::vector<llvm::Value *> ArgValues;
        auto FormatStr = std::string(Expr->children->at(0)->dvalue.str);
        auto *FormatStrInst = irBuilder.CreateGlobalStringPtr(FormatStr, "printf_format_str");
        ArgValues.push_back(FormatStrInst);

        if (Expr->child_num > 1) {
            auto *ArgList = Expr->children->at(1);

            for (auto *Arg : *(ArgList->children)) {
                if (Arg->ntype == Type::cconst && Arg->dtype == DataType::integer) {
                    ArgValues.push_back(llvm::ConstantInt::get(
                            GetLLVMType(DataType::integer),
                            Arg->dvalue.integer));
                } else if (Arg->ntype == Type::lvalue) {
                    auto *LValue = GetLValue(Arg);
                    auto *Loaded = irBuilder.CreateLoad(LValue->value, "tmp");
                    ArgValues.push_back(Loaded);
                } else {
                    // TODO string?
                }
            }
        }

        irBuilder.CreateCall(PrintfFunc, ArgValues, "c_printf");
    }

    static void BreakGen(AST *Expr) {
        irBuilder.CreateBr(CurrentMerge);
    }

    static void ContGen(AST *Expr) {
        irBuilder.CreateBr(CurrentCond);
    }

    static void RetGen(AST *Expr) {
        if (Expr->child_num == 0) {
            irBuilder.CreateRetVoid();
        } else {
            irBuilder.CreateRet(OpExprGen(Expr->children->at(0))->value);
        }
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
                auto *Loaded = irBuilder.CreateLoad(LValue->value, "tmp");
                ArgValues.push_back(Loaded);
            } else {
                // TODO string?
            }
        }

        if (CalleeFunc->retType == vvoid) {
            return new ValueWrapper(irBuilder.CreateCall(CalleeFunc->func, ArgValues), CalleeFunc->retType);
        }
        return new ValueWrapper(irBuilder.CreateCall(CalleeFunc->func, ArgValues, "calltmp"), CalleeFunc->retType);
    }

    static void FuncGen(AST *FunDefNode);

    static std::vector<std::string> CaptureVars(std::string PrtFuncName, std::string ClsName) {
        // handle by AST.cpp
        auto CapturedPtr = funs[PrtFuncName]->locfuns->at(ClsName)->parents_argv;
        std::vector<std::string> Captured;
        for (const auto& pair : *CapturedPtr) {
            Captured.emplace_back(pair.first);
        }
        return Captured;
    }

    static void ClosureGen(AST *Func) {
        auto PrtFuncName = irBuilder.GetInsertBlock()->getParent()->getName().str();
        if (PrtFuncName == "main") {
            PrtFuncName = "_" + PrtFuncName;
        }
        auto Captured = CaptureVars(PrtFuncName, Func->name);
        for (std::string Name : Captured) {
            auto *ConstInit = llvm::dyn_cast<llvm::Constant>(llvm::ConstantInt::get(GetLLVMType(integer), 0, true));
            auto *PromotedVar = new llvm::GlobalVariable(llvmModule, GetLLVMType(integer), false, llvm::GlobalValue::CommonLinkage, ConstInit, Name + "_prm");
            auto *wvalue = new ValueWrapper(PromotedVar, integer, Func->name);
            NamedGlobalValues[Name] = wvalue;
            auto *ToPromote = GetNamedValue(Name)->value;
            auto *Loaded = irBuilder.CreateLoad(ToPromote, "loaded");
            irBuilder.CreateStore(Loaded, PromotedVar);
        }
        auto *CurrentBB = irBuilder.GetInsertBlock();
        std::map<std::string, ValueWrapper*> CurrentLocalValues(NamedLocalValues);
        NamedLocalValues.clear();
        FuncGen(Func);
        NamedLocalValues = CurrentLocalValues;
        irBuilder.SetInsertPoint(CurrentBB);
    }

    static void ExprGen(AST *Expr) {
        if (Expr->name == "As_Exp") {
            AssignGen(Expr);
        } else if (Expr->name == "Op_Exp") {
            OpExprGen(Expr);
        } else if (Expr->name == "Cond_Exp") {
            CondExprGen(Expr);
        } else if (Expr->name == "If_Stmt") {
            IfGen(Expr);
        } else if (Expr->name == "Lop_Stmt") {
            LopGen(Expr);
        } else if (Expr->name == "scanf") {
            InputGen(Expr);
        } else if (Expr->name == "printf") {
            OutputGen(Expr);
        } else if (Expr->name == "break") {
            BreakGen(Expr);
        } else if (Expr->name == "continue") {
            ContGen(Expr);
        } else if (Expr->name == "return") {
            RetGen(Expr);
        } else if (Expr->ntype == Type::func) {
            ClosureGen(Expr);
        } else if (Expr->name == "Fas_Exp") {

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
            && FunVarList->children->at(0)->name == "void")) {
            // not void parameter
            for (auto* FunVar : *(FunVarList->children)) {
                auto Type = FunVar->children->at(0)->dtype;
                if (Type == DataType::integer) {
                    TypeVector.push_back(llvm::Type::getInt32Ty(llvmContext));
                } else if (Type == DataType::string) {
                }
            }
        }

        llvm::FunctionType *FT = llvm::FunctionType::get(FunType, TypeVector, false);
        llvm::Function *Func;
        if (FunDefNode->name == "_main") {
            Func = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", &llvmModule);
        } else {
            Func = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, std::string(FunDefNode->name), &llvmModule);
        }
        NamedFuncs[std::string(FunDefNode->name)] = new FunctionWrapper(Func, FunDataType);

        llvm::BasicBlock *BB = llvm::BasicBlock::Create(llvmContext, "entry", Func);
        irBuilder.SetInsertPoint(BB);

        auto VarIter = FunVarList->children->begin();
        for (auto &Arg : Func->args()) {
            auto name = (*VarIter)->name;
            auto type = (*VarIter)->children->at(0)->dtype;
            CreateLocalVariable(name, type);
            Arg.setName(name);
            irBuilder.CreateStore(&Arg, GetNamedValue(name)->value);
            ++VarIter;
        }

        LocalVarDeclListGen(DefList);

        ExprListGen(ExpList);

        if (FunDataType == DataType::vvoid) {
            irBuilder.CreateRetVoid();
        }

        llvm::verifyFunction(*Func);
    }

    static void InputAndOutputGen() {
        auto *FTScanf = llvm::FunctionType::get(llvm::IntegerType::getInt32Ty(llvmContext),
                                           { llvm::Type::getInt8PtrTy(llvmContext) },
                                           true);
        auto *ScanfFunc = llvm::Function::Create(FTScanf, llvm::Function::ExternalLinkage, "scanf", &llvmModule);
        ScanfFunc->setCallingConv(llvm::CallingConv::C);
        NamedFuncs["scanf"] = new FunctionWrapper(ScanfFunc, DataType::integer);

        auto *FTPrintf = llvm::FunctionType::get(llvm::IntegerType::getInt32Ty(llvmContext),
                                           { llvm::Type::getInt8PtrTy(llvmContext) },
                                           true);
        auto *PrintfFunc = llvm::Function::Create(FTPrintf, llvm::Function::ExternalLinkage, "printf", &llvmModule);
        PrintfFunc->setCallingConv(llvm::CallingConv::C);
        NamedFuncs["printf"] = new FunctionWrapper(PrintfFunc, DataType::integer);
    }

    void ProgramGen(AST *node) {
        AST *DefList = node->children->at(0);
        AST *FunDefList = node->children->at(1);

        InputAndOutputGen();

        GlobalVarDeclListGen(DefList);

        for (auto* FunDef : *(FunDefList->children)) {
            FuncGen(FunDef);
            NamedLocalValues.clear();
        }

        std::error_code EC;
        llvm::raw_fd_ostream llout("mhl.ll", EC);
        llvmModule.print(llout, nullptr);
    }
}