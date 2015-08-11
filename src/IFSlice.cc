#define DEBUG_TYPE "ifslice"
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Function.h>
#include <llvm/Pass.h>
#include <llvm/IR/Value.h>

#include "IFSlice.h"
#include "LLVMSlicer/Slicing/IntrsctFwdFunctionStaticSlicer.h"

using namespace llvm;
using namespace llvm::slicing;


void IFSlice::runFSS(Function &F) {
	//errs() << "FSlice runFSS called \n" ;

	IntrsctFwdFunctionStaticSlicer *FSS = new IntrsctFwdFunctionStaticSlicer(F, AA, AST);
	FSS->init();
	slicers.insert(Slicers::value_type(&F, FSS));
}

void IFSlice::computeSlice() {
	typedef SmallVector<const Function *, 20> WorkSet;
	bool hadAssert = false;
	for( Slicers::iterator ii = slicers.begin(), ee = slicers.end();
			ii != ee; ii++ ){
		IntrsctFwdFunctionStaticSlicer* FSS = 
				static_cast<IntrsctFwdFunctionStaticSlicer*>(ii->second);
		const Function* F = ii->first;
		// Find just one slicing criteria
		hadAssert = slicing::findIntrsctInitialCriterion(*F, *FSS);

		// If found a interesting call in this function, then slice on that
		if (hadAssert) {
			WorkSet Q;
			Q.push_back(&*F);

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
			// Done slicing this instance, cleaning up
			newSlice();
			ii = slicers.begin();
		}
	}

}

// Clean up and prepare for newSlice
void IFSlice::newSlice() {
	for( Slicers::iterator ii = slicers.begin(), ee = slicers.end();
				ii != ee; ii++ ){
		IntrsctFwdFunctionStaticSlicer* FSS = static_cast<IntrsctFwdFunctionStaticSlicer*>(ii->second);
		FSS->newSlice();
	}
}

void IFSlice::done() {
	TimePairs arithPairs;
	for( Slicers::iterator ii = slicers.begin(), ee = slicers.end();
			ii != ee; ii++ ){
		IntrsctFwdFunctionStaticSlicer* FSS = 
				static_cast<IntrsctFwdFunctionStaticSlicer*>(ii->second);
		FSS->getAllPairs( arithPairs );
	}	

	errs() << "Printing Time Pairs \n" ;
	for (TimePairs::iterator ii = arithPairs.begin(), ee = arithPairs.end(); ii != ee; ii++){
		int c1 = ii->first;
		int c2 = ii->second;
		const Instruction* I1 = IntrsctFwdFunctionStaticSlicer::initCrits[c1];
		const Instruction* I2 = IntrsctFwdFunctionStaticSlicer::initCrits[c2];
		errs() << "Pair " << c1 << "\t" << c2 << "\n";
		errs() << I1->getParent()->getParent()->getName();
		I1->print(errs());
		errs() << "\n";
		errs() << I2->getParent()->getParent()->getName();
		I2->print(errs());
		errs() << "\n";

		Ctx->TICS.insert(const_cast<Instruction*>(I1));
		Ctx->TICS.insert(const_cast<Instruction*>(I2));
	}
	errs() << "Done Printing Time Pairs \n" ;

	SliceCount sliceCount;
	for( Slicers::iterator ii = slicers.begin(), ee = slicers.end();
			ii != ee; ii++ ){
		IntrsctFwdFunctionStaticSlicer* FSS = 
				static_cast<IntrsctFwdFunctionStaticSlicer*>(ii->second);
		FSS->addSliceCount( sliceCount );
	}

	errs() << "Printing sliceCount \n" ;
	for (SliceCount::iterator ii = sliceCount.begin(), ee = sliceCount.end(); ii != ee; ii++){
		int id = ii->first;
		int count = ii->second;
		const Instruction* I = IntrsctFwdFunctionStaticSlicer::initCrits[id];
		errs() << "Count " << count << "\n";
		errs() << I->getParent()->getParent()->getName();
		I->print(errs());
		errs() << "\n";
	}

	// Free up memory
	//for (Module::iterator f = M->begin(), ef = M->end(); f != ef; ++f){
	//FunctionStaticSlicer* FSS = slicers[&*f];
	//delete FSS;
	//}
	slicers.clear();
	//initFuns.clear();
}
