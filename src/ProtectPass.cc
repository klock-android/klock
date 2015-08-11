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

#include "ProtectPass.h"

using namespace llvm;

bool ProtectPass::doInitialization(Module *M)
{	
	return true;
}

Protection ProtectPass::findInitProtection( Function *F ){
	CISet calls = Ctx->CallSites[F];
	Protection P;

	if( ! calls.empty() ) {

		// Copy from first call
		CISet::iterator i = calls.begin();
		P = CallEnt[*i];
		++i;
	
		// Intersection with rest calls
		for( CISet::iterator e = calls.end(); i!=e; ++i ){
			Protection &calleeProt = CallEnt[*i];
			for (Protection::iterator ii = P.begin(), ee = P.end(); ii != ee; ++ii) {
				// Intersection 
				if( calleeProt.find(*ii) == calleeProt.end() ){
					P.erase( ii );
				}
			}
		}
	}
	return P;
}

bool ProtectPass::visitCallInst(CallInst *CI)
{
	bool changed = false;
	if (CI->isInlineAsm() || Ctx->Callees.count(CI) == 0)
		return false;

	Protection BBProt = FuncProtOut[CI->getParent()];
	//DEBUG( dbgs() << "visitCallInst " );
	//DEBUG( CI->print ( dbgs() )  );
	//DEBUG( printProtection( dbgs(), BBProt ));
	//DEBUG( dbgs() << "\n " );

	// update protection of call
	CallEnt[CI] = BBProt;

	FuncSet &CEEs = Ctx->Callees[CI];
	for (FuncSet::iterator i = CEEs.begin(), e = CEEs.end(); i != e; ++i) {
		changed |=  Ctx->FuncSum[*i].applyThisSummary( &BBProt );
	}

	// Above misses the klock_PC function. It does'nt have valid summary
	// computed and saved. Its summary depends on parameters passed to it.
	// Handling it separately
	Function *CF = CI->getCalledFunction();
	if( CF != NULL ){
		StringRef Name = CF->getName();
		if (Name == "klock_PC") {
			assert( CI->getNumArgOperands() == 2 );
			if(ConstantInt *o0 = dyn_cast<ConstantInt>(CI->getArgOperand(0))){
				if(ConstantInt *o1 = dyn_cast<ConstantInt>(CI->getArgOperand(1))){
					if ( o0->getSExtValue() )
						BBProt.insert( o1->getSExtValue() );
					else
						BBProt.erase( o1->getSExtValue() );
				}
			}
		} 
	}


	FuncProtOut[CI->getParent()] = BBProt;
	return changed;
}

bool ProtectPass::updateProtectFor(Instruction *I)
{
	bool changed = false;
	
	if (CallInst *CI = dyn_cast<CallInst>(I)) {
		changed |= visitCallInst(CI);
	}

	if( FuncProtOut[ I->getParent() ].empty() ){
		Ctx->SAFE.erase( &*I );
		//DEBUG (dbgs() << "Erase from SAFE ");
	} else{
		//DEBUG( dbgs() << "Insert into SAFE ");
		Ctx->SAFE.insert( &*I );
	}
	//DEBUG( I->print(dbgs()));
	//DEBUG(dbgs() << "\n ");

	//DEBUG(dbgs() << "updateProtectFor ins " << changed );
	//DEBUG( I->print(dbgs()));
	//DEBUG(dbgs() << "\n");
	return changed;
}

bool ProtectPass::isBackEdge(const Edge &E)
{
	return std::find(BackEdges.begin(), BackEdges.end(), E)	!= BackEdges.end();
}

bool ProtectPass::updateProtectFor(BasicBlock *BB)
{
	bool changed = false;
	if( pred_begin(BB) != pred_end(BB) ){
		pred_iterator i = pred_begin(BB);
		Protection temp = FuncProtOut[*i];
		++i;

		// propagate protections from pred BBs, protections in BB are intersection of protections
		// in pred BBs (worst case analysis), constrained by each terminator.
		for (pred_iterator e = pred_end(BB); i != e; ++i) {
			BasicBlock *Pred = *i;
			if (isBackEdge(Edge(Pred, BB)))
				continue;

			Protection &PredProt = FuncProtOut[Pred];

			for (Protection::iterator ii = temp.begin(), 
					ee = temp.end(); ii != ee; ++ii) {
				// Intersection 
				if( PredProt.find(*ii) == PredProt.end() ){
					temp.erase( ii );
				}
			}

			// Refine according to the terminator
			// TODO: branch instruction wl.isHeld() should
			// be passed to the next basic block. Ignoring such cases for now.
			// visitTerminator(Pred->getTerminator(), BB, Prot);
		}
		if ( FuncProtIn.find(BB) == FuncProtIn.end() || FuncProtIn[BB] != temp )
			FuncProtIn[BB] = temp;
		else{
			// There was no change in the IN set. No need to run through BB
			return false;
		}
	}
	
	// Make a copy of protection for later comparison
	Protection BBProt (FuncProtOut[BB]);
	FuncProtOut[BB] = FuncProtIn[BB];

	// Now run through instructions
	for (BasicBlock::iterator i = BB->begin(), e = BB->end(); 
		 i != e; ++i) {
		updateProtectFor(&*i);
	}
	// Don't pass changed from updateProtectFor(instruction). If one call acquires,
	// other call releases in same basic block, then it will keep
	// oscillating forever
	changed = (BBProt != FuncProtOut[BB]);
	//DEBUG( dbgs() << "return BB " << changed << "\n");
	return changed;
}

bool ProtectPass::updateProtectFor(Function *F)
{
	bool changed = false;
	
	BackEdges.clear();

	if( F->getBasicBlockList().empty() ){	// No definition of function
		return false;
	}

	FindFunctionBackedges(*F, BackEdges);
	BasicBlock* first = F->begin();
	Protection temp = findInitProtection(F);
	
	// If it is first pass or IN has changed
	if( FuncProtIn.find(first) == FuncProtIn.end() || FuncProtIn[first] != temp ) {
		FuncProtIn[first] = temp;
	} else{
		return false;
	}

	DEBUG( dbgs() << "updateProtectFor " << F->getName() << "\n" );
	
	for (Function::iterator b = F->begin(), be = F->end(); b != be; ++b)
		FuncProtOut[&*b].clear();

	for (Function::iterator b = F->begin(), be = F->end(); b != be; ++b)
		changed |= updateProtectFor(&*b);
	
	return changed;
}

bool ProtectPass::doModulePass(Module *M)
{
	bool changed = true, ret = false;
	while (changed){
		changed = false;
		for (Module::iterator i = M->begin(), e = M->end(); i != e; ++i)
			if (!i->empty())
				changed |= updateProtectFor(&*i);
		ret |= changed;
	}
	return ret;
}

// write back
bool ProtectPass::doFinalization(Module *M) {
	//Following should have already converged and must be saved in 
	//FuncProtIn[first]
	return true;
}

void ProtectPass::done(){
	FuncProtIn.clear();
	FuncProtOut.clear();
	CallEnt.clear();
}

void ProtectPass::dumpProtect()
{
}

