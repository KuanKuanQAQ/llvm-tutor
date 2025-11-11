#ifndef LLVM_TUTOR_FuncTimer_H
#define LLVM_TUTOR_FuncTimer_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

//------------------------------------------------------------------------------
// New PM interface
//------------------------------------------------------------------------------
struct FuncTimer : public llvm::PassInfoMixin<FuncTimer> {
  bool hasRun = false; 
  llvm::PreservedAnalyses run(llvm::Module &M,
                              llvm::ModuleAnalysisManager &);
  bool runOnModule(llvm::Module &M);
};

//------------------------------------------------------------------------------
// Legacy PM interface
//------------------------------------------------------------------------------
struct LegacyFuncTimer : public llvm::ModulePass {
  static char ID;
  LegacyFuncTimer() : ModulePass(ID) {}
  bool runOnModule(llvm::Module &M) override;

  FuncTimer Impl;
};

#endif
