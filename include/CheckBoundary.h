//==============================================================================
// FILE:
//    CheckBoundary.h
//
// DESCRIPTION:
//    Declares the CheckBoundary Passes:
//      * new pass manager interface
//      * printer pass for the new pass manager
//
// License: MIT
//==============================================================================
#ifndef LLVM_PASS_CHECKBOUNDARY_H
#define LLVM_PASS_CHECKBOUNDARY_H

#include "llvm/ADT/StringMap.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/IR/Module.h>

//------------------------------------------------------------------------------
// New PM interface
//------------------------------------------------------------------------------
using ResultCheckBoundary = llvm::StringMap<llvm::StringRef>;

struct CheckBoundary : public llvm::AnalysisInfoMixin<CheckBoundary> {
  using Result = ResultCheckBoundary;
  Result res;

  Result run(llvm::Module &M, llvm::ModuleAnalysisManager &) {
    computeImportedFuncs(M);
    computeExportedFuncs(M);
    return res;
  }

  void computeImportedFuncs(llvm::Module &M);
  void computeExportedFuncs(llvm::Module &M);

  static bool isRequired() { return true; }

private:
  // A special type used by analysis passes to provide an address that
  // identifies that particular analysis pass type.
  static llvm::AnalysisKey Key;
  friend struct llvm::AnalysisInfoMixin<CheckBoundary>;
};

//------------------------------------------------------------------------------
// New PM interface for the printer pass
//------------------------------------------------------------------------------
class CheckBoundaryPrinter : public llvm::PassInfoMixin<CheckBoundaryPrinter> {
public:
  explicit CheckBoundaryPrinter(llvm::raw_ostream &OutS) : OS(OutS) {}
  llvm::PreservedAnalyses run(llvm::Module &Module,
                              llvm::ModuleAnalysisManager &MAM);

  static bool isRequired() { return true; }

private:
  llvm::raw_ostream &OS;
};
#endif
