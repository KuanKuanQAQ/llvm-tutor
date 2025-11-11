#ifndef LLVM_TUTOR_INJECT_TRAMPOLINE_H
#define LLVM_TUTOR_INJECT_TRAMPOLINE_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

struct InjectTrampoline : public llvm::PassInfoMixin<InjectTrampoline> {
  bool hasRun = false; 
  llvm::PreservedAnalyses run(llvm::Module &M,
                              llvm::ModuleAnalysisManager &);
  bool runOnModule(llvm::Module &M);
};

#endif
