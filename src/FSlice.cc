#define DEBUG_TYPE "fslice"
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Argument.h>
#include <llvm/Pass.h>
#include <llvm/IR/Value.h>
#include <llvm/Analysis/AliasAnalysis.h>

#include "FSlice.h"
#include "LLVMSlicer/Slicing/FwdFunctionStaticSlicer.h"
#include "LLVMSlicer/Slicing/FwdStaticSlicerHelper.h"

using namespace llvm;
using namespace llvm::slicing;
#define DEBUG_GLOBALS

void FSlice::computeSlice() {
	typedef SmallVector<const Function *, 20> WorkSet;
	WorkSet Q(initFuns);

	while (!Q.empty()) {
		for (WorkSet::const_iterator f = Q.begin(); f != Q.end(); ++f){
			DEBUG( errs() << "computeSlice: " << (*f)->getName() << "\n" );
			slicers[*f]->calculateStaticSlice();
		}

		WorkSet tmp;
		for (WorkSet::const_iterator f = Q.begin(); f != Q.end(); ++f) {
			// Map return val. and add all functions that call f in tmp
			toCalls(*f, std::inserter(tmp, tmp.end()));
			// Map arguments and add all functions called by f in tmp
			toExits(*f, std::inserter(tmp, tmp.end()));
		}
		std::swap(tmp,Q);
	}
}

void FSlice::runFSS(Function &F) {
	//errs() << "FSlice runFSS called \n" ;

	FwdFunctionStaticSlicer *FSS = new FwdFunctionStaticSlicer(F, AA, AST);
	FSS->init();
	bool hadAssert = slicing::findfwdInitialCriterion(F, *FSS);

	/*
	 * Functions with an assert might not have a return and slicer wouldn't
	 * compute them at all in that case.
	 */
	if (hadAssert){
		initFuns.push_back(&F);
		//errs() << "Pushing initFuns " << F.getName();
	}

	slicers.insert(Slicers::value_type(&F, FSS));
}

template<typename OutIterator>
void FSlice::addGlobals(RelevantSet R,
		OutIterator out) {
	for(Slicers::iterator it=slicers.begin(); it!=slicers.end(); ++it) {
		FunctionStaticSlicer *FSS = it->second;

		// Add all global variables to the first instruction of the function
		const Instruction* i = getFunctionEntry(it->first);
		bool addFunction = FSS->addCriterion(i, R.begin(), R.end());
		if (addFunction){
			DEBUG( errs() << "Adding " << __func__ << " " << it->first->getName() << "\n" );
			*out++ = it->first;
		}
	}
}

template<typename OutIterator>
void FSlice::toCalls(llvm::Function const* const f,
		OutIterator out) {
	DEBUG( errs() << __func__ << " " << f->getName() << "\n" );

	// For global variables
	RelevantSet R;

	// For arguments passed as pointers
	RelevantSet args;

	// Is the return value relevant
	bool retRelavant = false;

	typedef std::vector<const llvm::ReturnInst *> ExitsVec;
	ExitsVec E;
	getFunctionExits(f, std::back_inserter(E));

	for (ExitsVec::const_iterator e = E.begin(); e != E.end(); ++e) {
		const ValSet::const_iterator relBgn =
			slicers[f]->relevant_begin(*e);
		const ValSet::const_iterator relEnd =
			slicers[f]->relevant_end(*e);

		// Check if any relevant variable is a call argument that was passed as a pointer
		helper->getRelevantPtrParams( f, AA, relBgn, relEnd, args );

		Value *ret = (*e)->getReturnValue();
		for ( ValSet::iterator ii = relBgn; ii != relEnd; ++ii){
			// Check if return value is relevant
			if(ret && ii->first == ret){
				retRelavant = true;
				DEBUG( errs() << "toCalls retRelavant \n");
			} 
			// Add the global values
			else if (!isLocalToFunction(ii->first, f)) {
#ifdef DEBUG_GLOBALS
				DEBUG( errs() << "global " << f->getName() << " " << ii->first->getName() << "\n");
#endif
				R.insert(*ii);
			}
			// Check if any relevant variable is a call argument that was passed as a pointer
			/*for( Function::const_arg_iterator  jj = f->arg_begin(), ejj = f->arg_end(); 
					jj != ejj; jj ++ ){
				DEBUG( errs() << "Check function argument " );
				DEBUG( jj->print( errs() ));
				DEBUG( ii->first->print( errs() ));
				DEBUG( errs() << "\n " );

				if( AA->alias( ii->first, &*jj ) != AliasAnalysis::NoAlias ){
					DEBUG( errs() << "Found relevant function argument " );
					DEBUG( jj->print( errs() ));
					DEBUG( errs() << "\n " );
					args.insert( Pointee( &*jj, -1) );
				}
			}*/
		}
	}

	// Since call graph is not accurate. Make a conservative assumption
	// that changes to global variables anywhere will be visible in
	// every other function. Add all functions in out that are seeing
	// this global var change for first time
	addGlobals(R, out);

	
	CISet cs = Ctx->CallSites[const_cast<Function*>(f)];
	for ( CISet::iterator c = cs.begin(), e = cs.end(); c != e; ++c) {
		const llvm::CallInst *CI = const_cast<CallInst*>(*c);
		// function that call f
		const llvm::Function *g = CI->getParent()->getParent();
		DEBUG( errs() << "Analyze toCalls " << g->getName() << "\n" );
		FunctionStaticSlicer *FSS = slicers[g];

		bool addFunction = false;

		if(retRelavant ) {
			// Add return value if it is relevant
			addFunction = addFunction || FSS->addCriterion(CI, FSS->DEF_begin(CI), FSS->DEF_end(CI), true);
		}

		if( ! args.empty() ){
			// Add all the pointer arguments
			ParamsToArgs toArgs;
			helper->fillParamsToArgs( CI, f, toArgs );
			RelevantSet rcParams;
			for ( RelevantSet::iterator aa = args.begin(), aae = args.end(); aa != aae; aa++ ){
				for ( ParamsToArgs::iterator apii = toArgs.begin(), apee = toArgs.end();
						apii != apee; apii++){
					DEBUG( errs() << "Comparing " );
					DEBUG( apii->first.first->print( errs() ) );
					DEBUG( aa->first->print( errs() ) );
					DEBUG( errs() << "\n" );
					if(helper->checkValid(apii->first, *aa))
					if( ! AA->isNoAlias(apii->first.first, aa->first) ){
						DEBUG( errs() << "relevant param " );
						DEBUG( apii->second.first->print( errs() ) );
						DEBUG( errs() << "\n" );
						rcParams.insert( apii->second );
						break;
					}
				}
			}
			addFunction = addFunction || FSS->addCriterion(CI, rcParams.begin(), rcParams.end(), true);
		}

		if (addFunction){
			DEBUG( errs() << "Adding toCalls " << g->getName() << "\n" );
			*out++ = g;
		}
	}
}

template<typename OutIterator>
void FSlice::toExits(llvm::Function const* const f,
		OutIterator out) {
	DEBUG( errs() << "toExits " << f->getName() << "\n" );
	typedef std::vector<const llvm::CallInst *> CallsVec;
	CallsVec C;
	// All places that are called by f
	getFunctionCalls(f, std::back_inserter(C));
	for (CallsVec::const_iterator c = C.begin(); c != C.end(); ++c) {

		ValSet::const_iterator relBgn = slicers[f]->relevant_begin(*c);
		const ValSet::const_iterator relEnd = slicers[f]->relevant_end(*c);

		FuncSet fs = Ctx->Callees[const_cast<CallInst*>(*c)];
		for (FuncSet::iterator g=fs.begin(), e = fs.end(); g != e; ++g) {
			const Function *callie = *g;
			if( callie->size() != 0 ) {
				DEBUG( errs() << "Analyze toExits " << callie->getName() << "\n" );
				FunctionStaticSlicer *FSS = slicers[callie];
				RelevantSet R;  // R will contain the relevant vars at func call
				helper->getRelevantVarsAtCall(*c, callie, relBgn, relEnd, R);

				// Add relavant var to the first instruction of the function
				const Instruction* i = getFunctionEntry(callie);

				if (FSS->addCriterion(i, R.begin(),R .end())) {
					*out++ = callie;
					DEBUG( errs() << "Adding toExits " << callie->getName() << "\n" );
				}
			}
		}
	}
}
