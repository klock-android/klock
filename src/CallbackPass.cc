#define DEBUG_TYPE "callback"
#include <llvm/Pass.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/InstIterator.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Constants.h>
#include <llvm/ADT/OwningPtr.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/DebugInfo.h>
#include <llvm/Analysis/CallGraph.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/ScalarEvolution.h>
#include <llvm/Analysis/ScalarEvolutionExpressions.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include "llvm/Support/CommandLine.h"

#include "CallbackPass.h"

using namespace llvm;

bool CallbackPass::doInitialization(Module *M)
{	
	return true;
}

bool CallbackPass::isRegistration(Function* F){
	if( F->getName() == "klock_CB")
		return true;
	return false;
}

bool CallbackPass::addFnInTics( Function *F ){
	if( alreadyInTics.find( F) == alreadyInTics.end() ){
		toPutInTics.push( F );
		return true;
	}
	return false;
}

bool CallbackPass::visitCallInst(CallInst *CI)
{
	bool changed = false;
	if (CI->isInlineAsm() || Ctx->Callees.count(CI) == 0)
		return false;

	FuncSet &fs = Ctx->Callees[CI];
	for (FuncSet::iterator ii = fs.begin(), ee = fs.end(); ii != ee; ++ii) {
		if( isRegistration( *ii ) ){
			DEBUG( dbgs() << "Found registration function \n" );
			// First argument is function pointer
			if( Function* F = dyn_cast<Function>(CI->getArgOperand(0)) ){
				DEBUG( dbgs() << "Detected callback " << F->getName() << " \n"); 
				addFnInTics( F );
			}
			Ctx->TICS.insert( CI );
		}
	}
	return changed;
}

bool CallbackPass::visitInst(Instruction *I) {
	bool changed = false;
	if (CallInst *CI = dyn_cast<CallInst>(I)) {
		changed |= visitCallInst(CI);
	}
	return changed;
}

bool CallbackPass::visitBB(BasicBlock *BB) {
	bool changed = false;
	for (BasicBlock::iterator i = BB->begin(), e = BB->end(); 
		 i != e; ++i) {
		changed |= visitInst(&*i);
	}
	return changed;
}

bool CallbackPass::visitFunc(Function *F) {
	bool changed = false;
	for (Function::iterator b = F->begin(), be = F->end(); b != be; ++b)
		changed |= visitBB(&*b);
	return changed;
}

bool CallbackPass::doModulePass(Module *M)
{
	bool changed = true, ret = false;
	while (changed){
		changed = false;
		for (Module::iterator i = M->begin(), e = M->end(); i != e; ++i)
			if (!i->empty())
				changed |= visitFunc(&*i);
		ret |= changed;
	}
	return ret;
}

// write back
bool CallbackPass::doFinalization(Module *M) {
	while ( ! toPutInTics.empty() ){
		Function* F = toPutInTics.front();
		DEBUG( dbgs() << "Popped function: " << F->getName() << "\n");
		if( alreadyInTics.find( F ) != alreadyInTics.end() ) continue;
		for (Function::iterator b = F->begin(), be = F->end(); b != be; ++b){
			BasicBlock *BB = &*b;
			for (BasicBlock::iterator i = BB->begin(), e = BB->end(); 
					i != e; ++i) {
				// Add the instruction to TICS
				Ctx->TICS.insert( &*i );

				// Add the functions it calls to in the queue
				if (CallInst *CI = dyn_cast<CallInst>(&*i)) {
					FuncSet &fs = Ctx->Callees[CI];
					for (FuncSet::iterator ii = fs.begin(), ee = fs.end(); ii != ee; ++ii) {
						addFnInTics( *ii );
					}
				}
			}
		}
		alreadyInTics.insert( F );
		toPutInTics.pop();
	}

	// Free up memory
	alreadyInTics.clear();

	return true;
}

