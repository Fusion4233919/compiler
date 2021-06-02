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

llvm::Value *codeGen(const AST& node) {
    
}

}