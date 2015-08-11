//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// Details are in a white paper by F. Tip called:
// A survey of program slicing techniques
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "ifslice"

#include <ctype.h>
#include <map>

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
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
#include "IntrsctFwdFunctionStaticSlicer.h"

using namespace llvm;
using namespace llvm::slicing;

typedef llvm::SmallVector<const Instruction *, 10> PredList;

int IntrsctInfo::count = 0;
InitCrits IntrsctFwdFunctionStaticSlicer::initCrits;

/**
 *	retRC - whether return value or call arguments are relevant 
 **/
static bool handleIntrsct(IntrsctFwdFunctionStaticSlicer &ss,
        const CallInst *CI, bool retRC) {

    if( ! ss.isInAnySlice(CI) ){
#ifdef DEBUG_INITCRIT
        errs() << "    adding ";
        CI->print(errs());
        errs() << "\n";
#endif
        if( retRC ){
            // Add return value of the call instruction
            ss.addInitialCriterion(CI, Pointee(CI, -1));
        } else{
            // Add the arguments of the call instruction
            assert( CI->getNumArgOperands() == 1 );	// For the getTime1
            for( int i = 0; i< CI->getNumArgOperands(); i++){
                ss.addInitialCriterion(CI, Pointee(CI->getArgOperand(i), -1));
            }
        }
        return true;
    }
    return false;
}


bool llvm::slicing::findIntrsctInitialCriterion(const Function &F,
        IntrsctFwdFunctionStaticSlicer &ss) {
    bool added = false;
#ifdef DEBUG_INITCRIT
    errs() << __func__ << " ============ BEGIN\n";
#endif

    const Module* M =  F.getParent();
    Function *GetTime1 = M->getFunction("klock_getT1");
    Function *GetTime2 = M->getFunction("klock_getT2");

    for (const_inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
        const Instruction *i = &*I;
        if (const CallInst *CI = dyn_cast<CallInst>(i)) {
            Function *callie = CI->getCalledFunction();
            if(!callie){
                continue;
            }
            DEBUG( errs()<<"FuncName "<<callie->getName()<<"\n" );
            const Module* M =  F.getParent();
            if (GetTime1 && GetTime1==callie){
                added = handleIntrsct(ss, CI, false);		// Call arguments are relevant
            } else if (GetTime2 && GetTime2==callie){
                added = handleIntrsct(ss, CI, true);		// Return value is relevant
            }
            if( added ) {
                DEBUG( errs()<<"matchedInteresting \n" );
                return true;
            }
        }
    } 
#ifdef DEBUG_INITCRIT
    errs() << __func__ << " ============ END\n";
#endif
    return false;
}

void IntrsctFwdFunctionStaticSlicer::newSlice() {
    for (inst_iterator I = inst_begin(fun), E = inst_end(fun); I != E; ++I) {
        getInsInfo(&*I)->clear();
    }
}
