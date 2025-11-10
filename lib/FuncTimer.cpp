#include "FuncTimer.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

using namespace llvm;

#define DEBUG_TYPE "func-timer"

bool FuncTimer::runOnModule(Module &M) {
  bool InsertedAtLeastOnePrintf = false;

  LLVMContext &C = M.getContext();

  FunctionCallee LogFunc = M.getOrInsertFunction(
      "log_time",
      Type::getVoidTy(C),
      Type::getInt8Ty(C),
      Type::getInt1Ty(C)
  );

  for (auto &F : M) {
    if (F.isDeclaration())
      continue;
    if (F.getName() == "log_time")
      continue;
    if (F.getName().startswith("__")) 
      continue;
    if (F.getName().contains("init")) 
      continue;
    if (F.getName().contains("printk")) 
      continue;

    IRBuilder<> Builder(&*F.getEntryBlock().getFirstInsertionPt());
    Value *FuncName = Builder.CreateGlobalString(F.getName());
    Value *IsExit = Builder.getInt1(false);

    // The following is visible only if you pass -debug on the command line
    // *and* you have an assert build.
    LLVM_DEBUG(dbgs() << " Injecting call to printf inside " << F.getName()
                      << "\n");

    Builder.CreateCall(LogFunc, {FuncName, IsExit});

    for (auto &BB : F) {
      Instruction *Term = BB.getTerminator();
      if (isa<ReturnInst>(Term)) {
        IRBuilder<> RetBuilder(Term);
        Value *ExitVal = RetBuilder.getInt1(true);
        RetBuilder.CreateCall(LogFunc, {FuncName, ExitVal});
      }
    }

    InsertedAtLeastOnePrintf = true;
  }

  return InsertedAtLeastOnePrintf;
}

PreservedAnalyses FuncTimer::run(llvm::Module &M,
                                       llvm::ModuleAnalysisManager &) {
  bool Changed = false;
  if (!hasRun) {
    Changed = runOnModule(M);
    hasRun = 1;
  }

  return (Changed ? llvm::PreservedAnalyses::none()
                  : llvm::PreservedAnalyses::all());
}

llvm::PassPluginLibraryInfo getFuncTimerPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "inject-func-call", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            // PB.registerPipelineParsingCallback(
            //     [](StringRef Name, ModulePassManager &MPM,
            //        ArrayRef<PassBuilder::PipelineElement>) {
            //       if (Name == "inject-func-call") {
            //         MPM.addPass(FuncTimer());
            //         return true;
            //       }
            //       return false;
            //     });
            // PB.registerPipelineStartEPCallback( // 优化流程一开始就执行
            //     [](ModulePassManager &MPM, llvm::OptimizationLevel Level) {
            //         MPM.addPass(FuncTimer());
            //     });
            PB.registerOptimizerLastEPCallback( // 优化流水线的最后阶段
                [](ModulePassManager &MPM, llvm::OptimizationLevel Level) {
                    MPM.addPass(FuncTimer());
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getFuncTimerPluginInfo();
}
