#define DEBUG_TYPE "protect"
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
#include <llvm/IR/DerivedTypes.h>

#include "IterativeModulePass.h"
#include "ProtectSummaryPass.h"

using namespace llvm;

bool ProtectSummaryPass::doInitialization(Module &M, ModulePass &MP)
{	
	for (Module::iterator i = M.begin(), e = M.end(); i != e; ++i){
		addInWorkQ(&*i);
	}
	return true;
}

bool ProtectSummaryPass::visitCallInst(CallInst *CI)
{
	bool changed = false;
	if (CI->isInlineAsm() || Ctx->Callees.count(CI) == 0)
		return false;

	bool isProtChng = false;
	ProtectSummary thisBBSum = BBSum[CI->getParent()];

	Function *CF = CI->getCalledFunction();
	if( CF != NULL ){
		StringRef Name = CF->getName();
		if (Name == "klock_PC") {
			isProtChng = true;
			assert( CI->getNumArgOperands() == 2 );
			if(ConstantInt *o0 = dyn_cast<ConstantInt>(CI->getArgOperand(0))){
				if(ConstantInt *o1 = dyn_cast<ConstantInt>(CI->getArgOperand(1))){
					changed |= thisBBSum.changeProtection( o0->getSExtValue(), o1->getSExtValue() );
					DEBUG( dbgs() << "After changeProtection " );
					DEBUG( thisBBSum.print(dbgs()));
				}
			}
		} 
	}
	if (! isProtChng ) {
		FuncSet &CEEs = Ctx->Callees[CI];
		for (FuncSet::iterator i = CEEs.begin(), e = CEEs.end(); i != e; ++i) {
			ProtectSummary &calleeSum = Ctx->FuncSum[*i];
			changed |= thisBBSum.applyPSummary( calleeSum );
		}
	}
	BBSum[CI->getParent()] = thisBBSum;
	return changed;
}

bool ProtectSummaryPass::updateProtectFor(Instruction *I)
{
	//DEBUG( dbgs() << "updateProtectFor inst " );
	//DEBUG( I->print( dbgs() ) );
	//DEBUG( dbgs() << "\n" );
	bool changed = false;
	
	if (CallInst *CI = dyn_cast<CallInst>(I)) {
		changed |= visitCallInst(CI);
	} 	
	
	return changed;
}

bool ProtectSummaryPass::isBackEdge(const Edge &E)
{
	return std::find(BackEdges.begin(), BackEdges.end(), E)	!= BackEdges.end();
}

bool ProtectSummaryPass::updateProtectFor(BasicBlock *BB)
{
	bool changed = false;
	ProtectSummary &thisBBSum = BBSum[BB];
	std::vector<ProtectSummary> predSums;

	// propagate protections from pred BBs, protections in BB are union of protections
	// in pred BBs, constrained by each terminator.
	for (pred_iterator i = pred_begin(BB), e = pred_end(BB);
			i != e; ++i) {
		BasicBlock *Pred = *i;
		if (isBackEdge(Edge(Pred, BB)))
			continue;
		
		predSums.push_back(BBSum[Pred]);
		
		// Refine according to the terminator
		// TODO: branch instruction wl.isHeld() should
		// be passed to the next basic block. Ignoring such cases for now.
		// visitTerminator(Pred->getTerminator(), BB, Prot);
	}
	
	// Union with its predecessor
	thisBBSum.unionMany( predSums );

	// Now run through instructions
	for (BasicBlock::iterator i = BB->begin(), e = BB->end(); 
		 i != e; ++i) {
		changed |= updateProtectFor(&*i);
	}
	
	return changed;
}

bool ProtectSummaryPass::updateProtectFor(Function *F)
{
	//raw_ostream &OS = dbgs();
	//DEBUG( dbgs() << "updateProtectFor " << F->getName() << "\n");
	bool changed = false;
	
	// XXX: Not sure why to clear this map
	//FuncProt.clear();
	BackEdges.clear();

	if( F->getBasicBlockList().empty() ){	// No definition of function
		return false;
	}

	FindFunctionBackedges(*F, BackEdges);
	
	ProtectSummary thisFuncSum;
	std::vector<ProtectSummary> returnBB;
	for (Function::iterator b = F->begin(), be = F->end(); b != be; ++b) {
		changed |= updateProtectFor(&*b);
		if ( dyn_cast<ReturnInst>(b->getTerminator())) {
			returnBB.push_back(BBSum[b]);
		}
	}
	thisFuncSum.unionMany( returnBB );
	changed =  ! (thisFuncSum.isEqual( Ctx->FuncSum[F] ));
	if( changed ) {
		Ctx->FuncSum[F] = thisFuncSum;

		// Add all the places where F is called from into the queue
		CISet &CIS = Ctx->CallSites[F];
		for (CISet::iterator i = CIS.begin(), e = CIS.end(); i != e; ++i) {
			addInWorkQ( (*i)->getParent()->getParent() );
		}
	}
	return changed;
}

void ProtectSummaryPass::addInWorkQ( Function *F ){
	DEBUG( dbgs() << "addInWorkQ " << F->getName() << "\n");
	// FuncsQ is SetVector. Automatically takes care of the duplication
	FuncsQ.insert(F);
}

bool ProtectSummaryPass::doPass() {
	while (!FuncsQ.empty()) {
		// Obtain and remove the last element
		Function *F = FuncsQ.back();
		FuncsQ.pop_back();

		updateProtectFor( F );
	}
	return true;
}

void ProtectSummaryPass::done(){
	BBSum.clear();
}

void ProtectSummaryPass::dumpProtect()
{
	//raw_ostream &OS = dbgs();
	for (FuncSummary::iterator i = Ctx->FuncSum.begin(), e = Ctx->FuncSum.end(); i != e; ++i) {
		DEBUG( dbgs() << i->first->getName() << "\n");
		DEBUG( i->second.print(dbgs()));
	}
}
