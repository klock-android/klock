//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// Details are in a white paper by F. Tip called:
// A survey of program slicing techniques
//===----------------------------------------------------------------------===//

// DEBUG FLAGS
//#define DEBUG_DUMP
//#define DEBUG_INITCRIT
//#define DEBUG_RC
//#define DEBUG_SLICE
//#define DEBUG_SLICING

#include <ctype.h>
#include <map>

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include <llvm/IR/Argument.h>
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/IR/TypeBuilder.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "PostDominanceFrontier.h"
#include "FwdFunctionStaticSlicer.h"
#include "Helper.h"

using namespace llvm;
using namespace llvm::slicing;

typedef llvm::SmallVector<const Instruction *, 10> PredList;

// Find all predecessors of Instruction i
static PredList getPredList(const Instruction *i) {
  PredList predList;
  const BasicBlock *bb = i->getParent();
  if (i != &bb->front()) {
    // If i is not first instruction of basic block, then 
    // just push back previous instruction of basic block into predList
    BasicBlock::const_iterator I(i);
    I--;
    predList.push_back(&*I);
  } else {
		// If i is first instruction of basic block, then 
		// push back last instruction of all pred. basic blocks
    for (const_pred_iterator I = pred_begin(bb), E = pred_end(bb); I != E; I++){
      predList.push_back(&(*I)->back());
    }
  }
  return predList;
}

/*
 * j is a predecessor of i
 * RC(i)=RC(i) \cup
 *   {v| v \in RC(j), v \notin DEF(i)} \cup
 *   {v| v \in DEF(i), REF(i) \cap RC(j) \neq \emptyset}
 *
 *   {v| v \in RC(j), v \notin DEF(i)} - v was relevant, and is not redefined
 *   {v| v \in DEF(i), REF(i) \cap RC(j) \neq \emptyset} - v is defined using a relevant variable
 */
bool FwdFunctionStaticSlicer::computeRCi(InsInfo *insInfoi, InsInfo *insInfoj) {
#ifdef DEBUG_RC
  errs() << "\n" << __func__;
  errs() << "j -----> \n";
  insInfoj->dump();
  errs() << "i ----->\n";
  insInfoi->dump();
#endif

  bool changed = false;

  /* {v| v \in RC(j), v \notin DEF(i)} */
#ifdef DEBUG_RC
  errs() << "\t\tChecking RC value ";
#endif
  for (ValSet::const_iterator I = insInfoj->RC_begin(),
       E = insInfoj->RC_end(); I != E; I++) {
    const Pointee &RCj = *I;
    bool in_DEF = false;
#ifdef DEBUG_RC
    errs() << "\n\t\t";
  RCj.first->print(errs());
#endif
    for (ValSet::const_iterator II = insInfoi->DEF_begin(),
         EE = insInfoi->DEF_end(); II != EE; II++) {
    	if(Helper::checkValid(*II, RCj))
    		if (AA->isMustAlias(II->first, RCj.first)) {
    			in_DEF = true;
    			break;
    		}
    }
    if (!in_DEF)
      if (insInfoi->addRC(RCj))
        changed = true;
  }
  /* REF(i) \cap RC(j) \neq \emptyset */
  bool isect_nonempty = false;
#ifdef DEBUG_RC
  errs() << "\n\t\tChecking REFvalue ";
#endif
  for (ValSet::const_iterator I = insInfoi->REF_begin(),
       E = insInfoi->REF_end(); I != E && !isect_nonempty; I++) {
    const Pointee &REFi = *I;
#ifdef DEBUG_RC
    errs() << "\n\t\t";
    REFi.first->dump();
#endif
    for (ValSet::const_iterator II = insInfoj->RC_begin(),
         EE = insInfoj->RC_end(); II != EE; II++) {
    	if (Helper::checkValid(REFi, *II)) {
#ifdef DEBUG_RC
    		errs() << "\t\t\t";
    		II->first->dump();
#endif
    		if (AA->isMustAlias(REFi.first, II->first)) {
    			isect_nonempty = true;
    			break;
    		}
    	}
    }
  }

  /* {v| v \in DEF(i), ...} */
  if (isect_nonempty)
    for (ValSet::const_iterator I = insInfoi->DEF_begin(),
         E = insInfoi->DEF_end(); I != E; I++)
      if (insInfoi->addRC(*I))
        changed = true;
#ifdef DEBUG_RC
  errs() << "\ns" << __func__ << "2 END";
  if (changed)
    errs() << " ----------CHANGED";
#endif
  return changed;
}

/*
 * Call computeRCi with (i, all predecessors of i)
 */
bool FwdFunctionStaticSlicer::computeRCi(InsInfo *insInfoi) {
  const Instruction *i = insInfoi->getIns();
  bool changed = false;
#ifdef DEBUG_RC
  errs() << "  " << __func__ << ": " << i->getOpcodeName();
  if (i->hasName())
    errs() << " (" << i->getName() << ")";
  errs() << '\n';
  errs() << "    DUMP: ";
  i->print(errs());
  errs() << '\n';
#endif
  PredList predList = getPredList(i);
  if( !predList.empty() ){
    for (PredList::const_iterator I = predList.begin(), E = predList.end();
        I != E; I++)
      changed |= computeRCi(insInfoi, getInsInfo(*I));
  }

  return changed;
}

void FwdFunctionStaticSlicer::computeRC() {
  bool changed;
#ifdef DEBUG_RC
  int it = 1;
#endif
  do {
    changed = false;
#ifdef DEBUG_RC
    errs() << "\n" << __func__ << ": ============== Iteration " << it++ << '\n';
#endif
		//Iterate forward in Function
    for (Function::iterator I = fun.begin(), E = fun.end(); I != E; I++) {
			// Info about previous instruction
      InsInfo *past = NULL;
			//Iterate forward in BasicBlock
      for (BasicBlock::iterator II = I->begin(), EE = I->end(); II != EE; ++II) {
        InsInfo *insInfo = getInsInfo(&*II);
        if (!past)
          changed |= computeRCi(insInfo);
        else
          changed |= computeRCi(insInfo, past);
        past = insInfo;
      }
    }
  } while (changed);
}

/*
 * j is predecessor of i
 * SC(i)={i| REF(i) \cap RC(j) \neq \emptyset}
 * Statement i should be in slice if it uses a relevant variable
 */
void FwdFunctionStaticSlicer::computeSCi(const Instruction *i, const Instruction *j) {
  InsInfo *insInfoi = getInsInfo(i), *insInfoj = getInsInfo(j);
#ifdef DEBUG_SLICE
  errs() << "\n\ti ==>\t";
  i->print(errs());
  errs() << "\n\tj ==>\t";
  j->print(errs());
#endif

  bool isect_nonempty = false;
  for (ValSet::const_iterator I = insInfoi->REF_begin(),
       E = insInfoi->REF_end(); I != E && !isect_nonempty; I++) {
    const Pointee &REFi = *I;
    for (ValSet::const_iterator II = insInfoj->RC_begin(),
         EE = insInfoj->RC_end(); II != EE; II++) {

    	if(Helper::checkValid(REFi, *II)) {
#ifdef DEBUG_SLICE
        	errs() << "\n\t\t";
        	REFi.first->print(errs());
        	errs() << "\t\t";
    		II->first->print(errs());
#endif
    		if (AA->isMustAlias(REFi.first, II->first)) {
    			isect_nonempty = true;
    			break;
    		}
    	}
    }
  }

  if (isect_nonempty) {
    insInfoi->deslice();
#ifdef DEBUG_SLICING
    errs() << "\n\tXXXXXXXXXXXXXY ";
    i->print(errs());
    errs() << '\n';
#endif
  }
}

/*
 * Calls computeSCi for (i, all pred of i)
 */
void FwdFunctionStaticSlicer::computeSC() {
  for (inst_iterator I = inst_begin(fun), E = inst_end(fun); I != E; I++) {
    const Instruction *i = &*I;
    PredList predList = getPredList(i);
    if( !predList.empty() ){
      for (PredList::const_iterator II = predList.begin(), EE = predList.end();
          II != EE; II++){
        computeSCi(i, *II);
      }
    }
  }
}

void FwdFunctionStaticSlicer::dump() {
#ifdef DEBUG_DUMP
  for (inst_iterator I = inst_begin(fun), E = inst_end(fun); I != E; I++) {
    const Instruction &i = *I;
    const ExtInsInfo *ii = getInsInfo(&i);
    ii->dump();
  }
#endif
}

/**
 * this method calculates the static slice for the CFG
 */
void FwdFunctionStaticSlicer::calculateStaticSlice() {
#ifdef DEBUG_SLICE
  errs() << __func__ << " ============ BEG\n";
#endif
#ifdef DEBUG_SLICE
	errs() << __func__ << " ======= compute RC\n";
#endif
	computeRC();
#ifdef DEBUG_SLICE
	errs() << __func__ << " ======= compute SC";
#endif
	computeSC();

  dump();

#ifdef DEBUG_SLICE
  errs() << __func__ << " ============ END\n";
#endif
}



bool FwdFunctionStaticSlicer::slice() {
#ifdef DEBUG_SLICE
  errs() << __func__ << " ============ BEG\n";
#endif
  bool removed = false;
  for (inst_iterator I = inst_begin(fun), E = inst_end(fun); I != E;) {
    Instruction &i = *I;
    InsInfoMap::iterator ii_iter = insInfoMap.find(&i);
    assert(ii_iter != insInfoMap.end());
    const InsInfo *ii = ii_iter->second;
    ++I;
    if (ii->isSliced() && canSlice(i)) {
#ifdef DEBUG_SLICE
      errs() << "  removing:";
      i.print(errs());
      errs() << " from " << i.getParent()->getName() << '\n';
#endif
      i.replaceAllUsesWith(UndefValue::get(i.getType()));
      i.eraseFromParent();
      insInfoMap.erase(ii_iter);
      delete ii;

      removed = true;
    }
  }
  return removed;
}

static bool handleFwd(Function &F, FwdFunctionStaticSlicer &ss,
		const CallInst *CI) {

#ifdef DEBUG_INITCRIT
        errs() << "    adding ";
        CI->print(errs());
        errs() << "\n";
#endif
  ss.addInitialCriterion(CI,
      Pointee(F.getParent()->getGlobalVariable("__ai_init_functions", true), -1));
  // Add return value of the call instruction
  ss.addInitialCriterion(CI, Pointee(CI, -1));
  return true;
}

bool llvm::slicing::findfwdInitialCriterion(Function &F,
                                         FwdFunctionStaticSlicer &ss,
                                         bool starting) {
  bool added = false;
#ifdef DEBUG_INITCRIT
  errs() << __func__ << " ============ BEGIN\n";
#endif
  const Function *F__assert_fail = F.getParent()->getFunction("klock_getT2");
  if (!F__assert_fail) /* no cookies in this module */
    return false;

  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    const Instruction *i = &*I;
		// Add the assert function
		if (const CallInst *CI = dyn_cast<CallInst>(i)) {
			Function *callie = CI->getCalledFunction();
			if (callie == F__assert_fail) {
				added = handleFwd(F, ss, CI);
			}
		} 
  }
#ifdef DEBUG_INITCRIT
  errs() << __func__ << " ============ END\n";
#endif
  return added;
}
