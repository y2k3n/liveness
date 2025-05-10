#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>
#include <memory>
#include <set>
// #include <unordered_map>

using namespace llvm;

std::set<BasicBlock *> findExitBBs(Function &func) {
  std::set<BasicBlock *> exitBBs;
  for (auto &BB : func) {
    auto &lastI = BB.back();
    if (isa<ReturnInst>(lastI)) {
      exitBBs.insert(&BB);
    }
  }
  return exitBBs;
}

// PhiDefs(B) the variables defined by φ-functions at the entry of block B
// PhiUses(B) the set of variables used in a φ-function at the entry of a
// successor of the block B
void findUSEsDEFs(
    Function &func, std::unordered_map<BasicBlock *, std::set<Value *>> &USEs,
    std::unordered_map<BasicBlock *, std::set<Value *>> &DEFs,
    std::unordered_map<BasicBlock *, std::set<Value *>> &phiUSEs,
    std::unordered_map<BasicBlock *, std::set<Value *>> &phiDEFs) {
  for (auto &BB : func) {
    auto &DEF = DEFs[&BB];
    auto &USE = USEs[&BB];
    auto &pDEF = phiDEFs[&BB];

    auto iter = BB.begin();
    for (; iter != BB.end(); ++iter) {
      if (!isa<PHINode>(*iter))
        break;
      auto phi = dyn_cast<PHINode>(&*iter);
      pDEF.insert(phi);
      int numIncoming = phi->getNumIncomingValues();
      for (int i = 0; i < numIncoming; ++i) {

        Value *inVal = phi->getIncomingValue(i);
        BasicBlock *inBB = phi->getIncomingBlock(i);
        if (!isa<Instruction>(inVal) && !isa<Argument>(inVal))
          continue;
        phiUSEs[inBB].insert(inVal);
      }
    }

    for (; iter != BB.end(); ++iter) {
      auto &inst = *iter;
      for (auto &oprand : inst.operands()) {
        Value *val = oprand.get();
        if (isa<Instruction>(val) || isa<Argument>(val)) {
          if (DEF.find(val) == DEF.end()) {
            USE.insert(val); // use without def
          }
        }
      }
      if (!inst.getType()->isVoidTy()) {
        DEF.insert(&inst);
      }
    }
  }
}

void findLiveVars(Function &func) {
  std::unordered_map<BasicBlock *, std::set<Value *>> USEs, DEFs, phiUSEs,
      phiDEFs;
  findUSEsDEFs(func, USEs, DEFs, phiUSEs, phiDEFs);
  std::set<BasicBlock *> worklist = findExitBBs(func);

  std::unordered_map<BasicBlock *, std::set<Value *>> INs, OUTs;
  while (!worklist.empty()) {
    BasicBlock *BB = *worklist.begin();
    worklist.erase(BB);

    // LiveOut(B) = ⋃_S∈succs(B) (LiveIn(S) \ PhiDefs(S)) ∪ PhiUses(B)
    // LiveIn(B) = PhiDefs(B) ∪ UpwardExposed(B) ∪ (LiveOut(B) \ Defs(B))
    std::set<Value *> oldIN = INs[BB], oldOUT = OUTs[BB];
    std::set<Value *> liveIN, liveOUT;
    liveOUT = phiUSEs[BB];
    for (BasicBlock *succ : successors(BB)) {
      std::set_difference(INs[succ].begin(), INs[succ].end(),
                          phiDEFs[succ].begin(), phiDEFs[succ].end(),
                          std::inserter(liveOUT, liveOUT.end()));
    }
    OUTs[BB] = liveOUT;

    liveIN = phiDEFs[BB];
    std::set_difference(OUTs[BB].begin(), OUTs[BB].end(), DEFs[BB].begin(),
                        DEFs[BB].end(), std::inserter(liveIN, liveIN.end()));
    liveIN.insert(USEs[BB].begin(), USEs[BB].end());
    INs[BB] = liveIN;

    if ((liveIN != oldIN) || (liveOUT != oldOUT)) {
      for (BasicBlock *pred : predecessors(BB)) {
        worklist.insert(pred);
      }
    }
  }

  for (auto &BB : func) {
    outs() << BB;
    outs() << "IN set: ----------------\n";
    for (auto in : INs[&BB]) {
      outs() << *in << "\n";
    }
    outs() << "---------------- :IN set\n";
    outs() << "OUT set: ++++++++++++++++\n";
    for (auto out : OUTs[&BB]) {
      outs() << *out << "\n";
    }
    outs() << "++++++++++++++++ :OUT set\n";
    // outs() << "DEF set: ****************\n";
    // for (auto def : DEFs[&BB]) {
    //   outs() << *def << "\n";
    // }
    // outs() << "**************** :DEF set\n";
    // outs() << "USE set: ****************\n";
    // for (auto use : USEs[&BB]) {
    //   outs() << *use << "\n";
    // }
    // outs() << "**************** :USE set\n";
    // outs() << "phiDEF: ****************\n";
    // for (auto def : phiDEFs[&BB]) {
    //   outs() << *def << "\n";
    // }
    // outs() << "**************** :phiDEF\n";
    // outs() << "phiUSE: ****************\n";
    // for (auto use : phiUSEs[&BB]) {
    //   outs() << *use << "\n";
    // }
    // outs() << "**************** :phiUSE\n";
  }
}

int main(int argc, char *argv[]) {
  InitLLVM X(argc, argv);
  if (argc < 2) {
    errs() << "Expect IR filename\n";
    exit(1);
  }
  LLVMContext context;
  SMDiagnostic smd;
  char *filename = argv[1];
  std::unique_ptr<Module> module = parseIRFile(filename, smd, context);
  if (!module) {
    errs() << "Cannot parse IR file\n";
    smd.print(filename, errs());
    exit(1);
  }

  // auto &func_list = module->getFunctionList();
  for (auto &func : *module) {
    if (func.isDeclaration())
      continue;
    outs() << "Function: " << func.getName().data() << "\n";
    findLiveVars(func);
    // auto exitBBs = findExitBBs(func);
    // for (auto &BB : func) {
    //   outs() << BB;
    // }
    outs() << "******************************** " << func.getName().data()
           << "\n";
  }
}