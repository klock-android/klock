#define DEBUG_TYPE "bslice"
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
#include <llvm/Pass.h>
#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/Analysis/AliasSetTracker.h>

#include "TimeGlobal.h"
#include "BSlice.h"

#include "LLVMSlicer/Slicing/FunctionStaticSlicer.h"
#include "LLVMSlicer/Slicing/StaticSlicerHelper.h"
#include "LLVMSlicer/Languages/LLVM.h"


using namespace llvm;
using namespace llvm::slicing;

bool BSlice::doInitialization(Module &M, ModulePass &MP) {	
	AA = &(MP.getAnalysis<AliasAnalysis>());
	assert ( AA  );
	AST = new AliasSetTracker( *AA );
	assert ( AST );

	initHelper();
	for (Module::iterator f = M.begin(); f != M.end(); ++f)
		if (!f->isDeclaration() && !memoryManStuff(&*f))
			runFSS(*f);

	return true;
}

void BSlice::runFSS(Function &F) {

	FunctionStaticSlicer *FSS = new FunctionStaticSlicer(F, AA, AST);
	FSS->init();
	bool hadAssert = slicing::findInitialCriterion(F, *FSS);

	/*
	 * Functions with an assert might not have a return and slicer wouldn't
	 * compute them at all in that case.
	 */
	if ( hadAssert)
		initFuns.push_back(&F);

	slicers.insert(Slicers::value_type(&F, FSS));
}

// All functions that call f
template<typename OutIterator>
void BSlice::emitToCalls(Function const* const f,
		OutIterator out) {
	DEBUG( dbgs() << "emitToCalls " << f->getName() << "\n");
	const ValSet::const_iterator relBgn =
		slicers[f]->relevant_begin(getFunctionEntry(f));
	const ValSet::const_iterator relEnd =
		slicers[f]->relevant_end(getFunctionEntry(f));
	CISet cs = Ctx->CallSites[const_cast<Function*>(f)];
	for ( CISet::iterator c = cs.begin(), 
			e = cs.end() ; c != e; ++c) {
		const CallInst *CI = const_cast<CallInst*>(*c);
		// function that call f
		const Function *g = CI->getParent()->getParent();
		DEBUG( dbgs() << "emittingCa " << g->getName() << "\n");
		FunctionStaticSlicer *FSS = slicers[g];
		RelevantSet R;  // R contains the relevant vars at func call
		helper->getRelevantVarsAtCall(CI, f, relBgn, relEnd, R);

		// Debug- print R
		DEBUG( dbgs() << "Print relevant set" );
		for( RelevantSet::iterator ri = R.begin(), re = R.end(); ri != re; ri++){
			DEBUG( (*ri).first->print( dbgs() ) );
			DEBUG( dbgs() << "\n" );
		}

		if (FSS->addCriterion(CI, R.begin(), R.end(),
					!FSS->shouldSkipAssert(CI))) {
			FSS->addCriterion(CI, FSS->REF_begin(CI), FSS->REF_end(CI));
			*out++ = g;
		}
	}
}

// All functions that are called by f
template<typename OutIterator>
void BSlice::emitToExits(Function const* const f,
		OutIterator out) {
	DEBUG( dbgs() << "emitToExits " << f->getName() << "\n");
	typedef std::vector<const CallInst *> CallsVec;
	CallsVec C;
	// All places that are called by f
	getFunctionCalls(f, std::back_inserter(C));
	for (CallsVec::const_iterator c = C.begin(); c != C.end(); ++c) {
		ValSet::const_iterator relBgn =
			slicers[f]->relevant_begin(getSuccInBlock(*c));
		const ValSet::const_iterator relEnd =
			slicers[f]->relevant_end(getSuccInBlock(*c));

		// Get relevant pointer arguments
		std::set<int> args;
		for ( ValSet::const_iterator relii = relBgn; relii != relEnd; relii ++ ){
			for ( std::size_t argii = 0; argii < (*c)->getNumArgOperands(); argii++ ){
				const Value* A = (*c)->getArgOperand(argii);
				if( A->getType()->isPointerTy() ){		// Only add pointer types
					if( AA->alias( A, (*relii).first ) != AliasAnalysis::NoAlias ){
						DEBUG( errs() << "Found relevant function argument " );
						DEBUG( A->print( errs() ));
						DEBUG( errs() << "\n " );
						args.insert( argii );
					}
				}
			}
		}

		// Get the relevant global values
			RelevantSet R_global;
			for( ValSet::const_iterator ii = relBgn; ii != relEnd; ii++){
				if (!isLocalToFunction(ii->first, f)) {
					R_global.insert(*ii);
				}
			}

		FuncSet fs =  Ctx->Callees[const_cast<CallInst*>(*c)];
		for (FuncSet::iterator g =fs.begin(),
				ee = fs.end() ; g != ee; ++g) {
			bool addFunction = false;
			// Make the return values relevant
			typedef std::vector<const ReturnInst *> ExitsVec;
			const Function *callie = const_cast<const Function*>(*g);
			DEBUG( dbgs() << "emittingEx " << callie->getName() << "\n");
			ExitsVec E;
			getFunctionExits(callie, std::back_inserter(E));
			for (ExitsVec::const_iterator e = E.begin(); e != E.end(); ++e) {
				RelevantSet R;
				helper->getRelevantVarsAtExit(*c, *e, relBgn, relEnd, R);
				addFunction = addFunction | slicers[callie]->addCriterion(*e, R.begin(),R .end());
			}

			
			// Add the relevant global values
			for (ExitsVec::const_iterator e = E.begin(); e != E.end(); ++e) {
				addFunction = addFunction | slicers[callie]->addCriterion(*e, R_global.begin(),R_global.end());
			}

			// Make the pointer arguments relevant
			if( !args.empty() ){
				RelevantSet params;
				int k = 0;
				for( Function::const_arg_iterator argit = callie->arg_begin(); 
						argit != callie->arg_end(); argit ++, k++ ){
					if( args.find(k) != args.end() ){
						DEBUG( errs() << "Add param " );
						DEBUG( errs() << argit->getName() );
						DEBUG( errs() << "\n" );
						params.insert( Pointee( &*argit, -1) );
					}
				}

				// Add to function exits-- **backward slicing**
				for (ExitsVec::const_iterator e = E.begin(); e != E.end(); ++e) {
					addFunction = addFunction | slicers[callie]->addCriterion( *e, 
							params.begin(), params.end());
				}
			}

			if( addFunction )
					*out++ = callie;
		}
	}
}

bool BSlice::sliceModule() {
	bool modified = false;
	return modified;
}

void BSlice::computeSlice() {
	typedef SmallVector<const Function *, 20> WorkSet;
	WorkSet Q(initFuns);

	while (!Q.empty()) {
		for (WorkSet::const_iterator f = Q.begin(); f != Q.end(); ++f)
			slicers[*f]->calculateStaticSlice();

		WorkSet tmp;
		for (WorkSet::const_iterator f = Q.begin(); f != Q.end(); ++f) {
			// Map arguments and add all functions that call f in tmp
			emitToCalls(*f, std::inserter(tmp, tmp.end()));
			// Map return val. and add all functions called by f in tmp
			emitToExits(*f, std::inserter(tmp, tmp.end()));
			// Clear f's memory a bit
			//slicers[*f]->clearMem();
		}
		std::swap(tmp,Q);
	}
}

bool BSlice::doPass()
{
	computeSlice();
	return sliceModule();
}

// write back
void BSlice::done() {
	for (Slicers::iterator it = slicers.begin() , eit = slicers.end();
			it != eit; ++it ){
		FunctionStaticSlicer* FSS = it->second;
		const Function* f = it->first;
		if( FSS != NULL ){
			for (const_inst_iterator i = inst_begin(*f), e = inst_end(*f); i != e; ++i){
				if( ! FSS->getInsInfo( &*i )->isSliced() ){
					Ctx->TICS.insert( const_cast<Instruction*>(&*i) );
				}
			}
		} else {
			DEBUG( errs() << "FSS null for " << f->getName() << "\n");
		}
	}

	// Free up memory
	//for (Module::iterator f = M->begin(), ef = M->end(); f != ef; ++f){
	//FunctionStaticSlicer* FSS = slicers[&*f];
	//delete FSS;
	//}
	slicers.clear();
	initFuns.clear();
}
