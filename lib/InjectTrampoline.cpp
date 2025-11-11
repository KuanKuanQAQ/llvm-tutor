#include "InjectTrampoline.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

using namespace llvm;

#define DEBUG_TYPE "inject-trampoline"

bool InjectTrampoline::runOnModule(Module &M) {
  bool Changed = false;
  LLVMContext &C = M.getContext();

  // 声明 log 函数: void x(i8* funcName, i1 isExit)
  FunctionCallee LogFunc = M.getOrInsertFunction(
      "x",
      Type::getVoidTy(C),
      Type::getInt8PtrTy(C),
      Type::getInt1Ty(C)
  );

  std::vector<Function*> ToProcess;
  for (Function &F : M) {
    if (F.isDeclaration())
      continue;
    if (F.getName().startswith("__"))
      continue;
    if (F.getName().contains("init"))
      continue;
    if (F.isVarArg())
      continue;
    if (F.getName() == "x")
      continue;

    StringRef Name = F.getName();
    
    // 安全过滤
    // if (Name.startswith("early_") || Name.startswith("start_kernel"))
    //   continue;
    // if (Name.contains("idle") || Name.contains("schedule"))
    //   continue;
    // if (Name.contains("irq") || Name.contains("trap") || Name.contains("exception"))
    //   continue;


    ToProcess.push_back(&F);
  }

  for (Function *Orig : ToProcess) {
    std::string OrigName = Orig->getName().str();

    std::string TempName = OrigName + ".tramp_tmp";
    while (M.getFunction(TempName))
      TempName += "_x"; // 重名的话就命名为 .tramp_tmp_x_x_x...
    Function *TempTramp = Function::Create(
        Orig->getFunctionType(),
        Orig->getLinkage(),
        TempName,
        &M
    );
    TempTramp->setCallingConv(Orig->getCallingConv());
    TempTramp->copyAttributesFrom(Orig);

    std::vector<User*> Users;
    for (User *U : Orig->users())
      Users.push_back(U);
    for (User *U : Users) {
      if (Instruction *I = dyn_cast<Instruction>(U)) {
        // if (I->getFunction() == Orig) continue; // 跳过 Orig 自身函数体内的 use
      }
      U->replaceUsesOfWith(Orig, TempTramp);
    }

    std::string RealName = OrigName + "_real";
    while (M.getFunction(RealName))
      RealName += "_x";
    Orig->setName(RealName);

    TempTramp->setName(OrigName);
    // errs() << "[InjectTrampoline] Wrapped function: "
    //     << OrigName << " -> " << RealName << "\n";

    BasicBlock *Entry = BasicBlock::Create(C, "entry", TempTramp);
    IRBuilder<> B(Entry);

    Value *FuncName = B.CreateGlobalStringPtr(OrigName);
    Value *EnterFlag = B.getInt1(false);
    B.CreateCall(LogFunc, {FuncName, EnterFlag});

    std::vector<Value*> Args;
    for (auto &A : TempTramp->args())
      Args.push_back(&A);

    CallInst *CallReal = B.CreateCall(Orig, Args); // Orig 现在是 foo_real
    CallReal->setCallingConv(TempTramp->getCallingConv());

    Value *ExitFlag = B.getInt1(true);
    B.CreateCall(LogFunc, {FuncName, ExitFlag});

    if (TempTramp->getReturnType()->isVoidTy())
      B.CreateRetVoid();
    else
      B.CreateRet(CallReal);

    Orig->setLinkage(GlobalValue::InternalLinkage);

    Changed = true;
  }

  return Changed;
}


PreservedAnalyses InjectTrampoline::run(llvm::Module &M, llvm::ModuleAnalysisManager &) {
  bool Changed = false;
  if (!hasRun) {
    Changed = runOnModule(M);
    hasRun = 1;
  }

  return (Changed ? llvm::PreservedAnalyses::none()
                  : llvm::PreservedAnalyses::all());
}

llvm::PassPluginLibraryInfo getFuncInjectTrampolineInfo() {
  return {LLVM_PLUGIN_API_VERSION, "inject-trampoline", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            // PB.registerPipelineStartEPCallback( // 优化流程一开始就执行
            //     [](ModulePassManager &MPM, llvm::OptimizationLevel Level) {
            //         MPM.addPass(InjectTrampoline());
            //     });
            PB.registerOptimizerLastEPCallback( // 优化流水线的最后阶段
                [](ModulePassManager &MPM, llvm::OptimizationLevel Level) {
                    MPM.addPass(InjectTrampoline());
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getFuncInjectTrampolineInfo();
}
