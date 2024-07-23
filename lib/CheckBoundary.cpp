//=============================================================================
// FILE:
//    CheckBoundary.cpp
//
// DESCRIPTION:
//    ...
//
// USAGE:
//    clang -fpass-plugin=./lib/libCheckBoundary.so <input-c-file>
//    clang -fpass-plugin=./lib/libCheckBoundary.so ~/llvm-tutor/inputs/input_for_hello.c
// License: MIT
//=============================================================================
#include "CheckBoundary.h"
#include <llvm/IR/InstIterator.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <unordered_set>

using namespace llvm;

// Pretty-prints the result of this analysis
static void printCheckBoundaryResult(raw_ostream &,
                                     const ResultCheckBoundary &OC);

//-----------------------------------------------------------------------------
// ExportCheck implementation
//-----------------------------------------------------------------------------
AnalysisKey CheckBoundary::Key;

void CheckBoundary::computeImportedFuncs(Module &M) {
  // Result res;
  for (auto &F : M) {
    std::string func_name = F.getName().str();
    if (F.isDeclaration()) {
      res[func_name] = "imported";
    } else {
      res[func_name] = "other";
    }
  }
  return;
}

void CheckBoundary::computeExportedFuncs(Module &M) {
  for (GlobalVariable &GV : M.globals()) {
    if (dyn_cast<StructType>(GV.getValueType())) {
      if (!GV.hasInitializer())
        continue;
      TinyPtrVector<Constant *> workqueue;
      workqueue.push_back(GV.getInitializer());
      res[GV.getName()] = "processing struct";
      while (!workqueue.empty()) {
        Constant *C = workqueue.back();
        workqueue.pop_back();

        if (ConstantStruct *CS = dyn_cast<ConstantStruct>(C)) {
          for (unsigned i = 0; i < CS->getNumOperands(); ++i) {
            Constant *Op = CS->getOperand(i);

            if (dyn_cast<StructType>(Op->getType())) {
              if (Constant *InnerC = dyn_cast<Constant>(Op)) {
                workqueue.push_back(InnerC);
              }
            } else if (Function *F = dyn_cast<Function>(Op)) {
              res[F->getName()] = "exported";
            }
          }
        }
      }
      res[GV.getName()] = "struct";
    }
  }
  return;
}

PreservedAnalyses CheckBoundaryPrinter::run(Module &M,
                                            ModuleAnalysisManager &MAM) {
  auto &CheckResult = MAM.getResult<CheckBoundary>(M);

  OS << "Printing analysis 'CheckBoundary Pass' for module '" << M.getName()
     << "':\n";

  printCheckBoundaryResult(OS, CheckResult);
  return PreservedAnalyses::all();
}

PassPluginLibraryInfo getCheckBoundaryPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "CheckBoundary", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &PM, OptimizationLevel Level) {
                  PM.addPass(CheckBoundaryPrinter(errs()));
                });
            PB.registerAnalysisRegistrationCallback(
                [](ModuleAnalysisManager &MAM) {
                  MAM.registerPass([&] { return CheckBoundary(); });
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return getCheckBoundaryPluginInfo();
}

//------------------------------------------------------------------------------
// Helper functions - implementation
//------------------------------------------------------------------------------
static void printCheckBoundaryResult(raw_ostream &OutS,
                                     const ResultCheckBoundary &Res) {
  OutS << "================================================="
       << "\n";
  OutS << "LLVM-PASS: CheckBoundary results\n";
  OutS << "=================================================\n";
  const char *str1 = "SYMBOL";
  const char *str2 = "TYPE";
  OutS << format("%-20s %-10s\n", str1, str2);
  OutS << "-------------------------------------------------"
       << "\n";
  for (auto &Inst : Res) {
    OutS << format("%-20s %-10s\n", Inst.first().str().c_str(),
                   Inst.second.str().c_str());
  }
  OutS << "-------------------------------------------------"
       << "\n\n";
}
