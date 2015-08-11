// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

#ifndef SLICING_FUNCTIONSTATICSLICER_H
#define SLICING_FUNCTIONSTATICSLICER_H

#include <map>
#include <utility> /* pair */

#include <llvm/IR/Value.h>
#include <llvm/IR/Instructions.h>
#include <llvm/ADT/SetVector.h>
#include <llvm/Support/InstIterator.h>
#include <llvm/Support/Debug.h>

#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/Analysis/AliasSetTracker.h>
#include "PostDominanceFrontier.h"
#include "Helper.h"

namespace llvm { namespace slicing {

typedef llvm::SmallSetVector<Pointee, 10> ValSet;

class InsInfo {
public:
  InsInfo(const llvm::Instruction *i, AliasAnalysis& AA, AliasSetTracker &AST);

  const Instruction *getIns() const { return ins; }

	virtual bool addRC(const Pointee &var) { 
		bool ret = false;
		uint64_t Size = var.second;                        
		MemoryLocation op = var.first;
		//if( Size == -1 ){
		if (op->getType()->isSized())             
			Size = AA->getTypeStoreSize(op->getType());         
		//}

		Value* temp = const_cast<Value*>(op);     
		const AliasSet* S = AST->getAliasSetForPointerIfExists(temp, Size,
				ins->getMetadata(LLVMContext::MD_tbaa));            
		if( S != NULL ){                          
			for (AliasSet::iterator I = S->begin(), E = S->end(); I != E; ++I){
				ret |= addRC_(Pointee(I.getPointer(), -1));  
			}
		}

		ret |= addRC_(var); 
		return ret;
	}
	bool addDEF(const Pointee &var) { 
		DEBUG_WITH_TYPE( "bfunslice", errs() << "addDEF " ); 
		if( ! checkLocal( var ) ) return false;
		DEBUG_WITH_TYPE( "bfunslice", var.first->print(errs()) ); 
		DEBUG_WITH_TYPE( "bfunslice", errs() << "\n " ); 
		return DEF.insert(var); 
	}
	bool addREF(const Pointee &var) { 
		DEBUG_WITH_TYPE( "bfunslice", errs() << "addREF " ); 
		if( ! checkLocal( var ) ) return false;
		DEBUG_WITH_TYPE( "bfunslice", var.first->print(errs()) ); 
		DEBUG_WITH_TYPE( "bfunslice", errs() << "\n " ); 
		return REF.insert(var); 
	}
	// For reducing memory footprint
  virtual void clearRC() { RC.clear(); }

  virtual void deslice() { 
		DEBUG_WITH_TYPE( "bfunslice", errs() << "Deslice " ); 
		DEBUG_WITH_TYPE( "bfunslice", ins->print(errs()) ); 
		DEBUG_WITH_TYPE( "bfunslice", errs() << "\n" ); 
		sliced = false; }

  virtual ValSet::const_iterator RC_begin() const { 
		DEBUG( ins->print( dbgs() ) );
		DEBUG( dbgs() << "\n" );
		DEBUG( dbgs() << "RC_begin " << RC.size() << "\n" );
		return RC.begin(); 
	}
  virtual ValSet::const_iterator RC_end() const { return RC.end(); }
  ValSet::const_iterator DEF_begin() const { return DEF.begin(); }
  ValSet::const_iterator DEF_end() const { return DEF.end(); }
  ValSet::const_iterator REF_begin() const { return REF.begin(); }
  ValSet::const_iterator REF_end() const { return REF.end(); }

  bool isSliced() const { return sliced; }
  virtual void dump() const ;


protected:
  const llvm::Instruction *ins;
  ValSet RC, DEF, REF;
  bool sliced;
	AliasAnalysis *AA;
	AliasSetTracker *AST;

	// The value being added should either be of the same
	// function or should be global.
	bool checkLocal( const Pointee &var ){
		MemoryLocation op = var.first;

		if( dyn_cast<Constant>( op ) )
			if( ! dyn_cast<GlobalVariable>( op ) )
				return false;
		if( dyn_cast<InlineAsm>( op ) )
			return false;

		const Function *F1 = getParent( ins );
		const Function *F2 = getParent( op );
		
		if( F1 && F2 && F1 != F2 ){
			errs() << "checkLocal failed for the variables \n";
			ins->print( errs() );
			errs() << F1->getName(); 
			errs() << "\n";
			op->print( errs() );
			errs() << F2->getName(); 
			errs() << "\n";
			assert( false );
		}
		return true;
	}

	bool addRC_( const Pointee &var ){
		if( ! checkLocal( var ) ) return false;
		DEBUG_WITH_TYPE( "bfunslice", errs() << "addRC_ " ); 
		DEBUG_WITH_TYPE( "bfunslice", var.first->print(errs()) ); 
		DEBUG_WITH_TYPE( "bfunslice", ins->print(errs()) ); 
		DEBUG_WITH_TYPE( "bfunslice", errs() << "\n " ); 
		return RC.insert( var );
	}
};

class FunctionStaticSlicer {
public:
  typedef std::map<const llvm::Instruction *, InsInfo *> InsInfoMap;

  //FunctionStaticSlicer(llvm::Function &F, llvm::ModulePass *MP,
  FunctionStaticSlicer(llvm::Function &F, AliasAnalysis* aa, AliasSetTracker* ast) :
	  //fun(F), MP(MP) {
	  fun(F), AST(ast), AA(aa) {
			// DONT all init from here. Everything will be screwed. 
			// It a bad design, I admit. Call init after creating construction of class.
			//errs() << "new FunctionStaticSlicer called " << F.getName() << "\n";
      }
  ~FunctionStaticSlicer();

	virtual void init() {
		for (llvm::inst_iterator I = llvm::inst_begin(fun), E = llvm::inst_end(fun);
				I != E; ++I)
			insInfoMap.insert(InsInfoMap::value_type(&*I, newInsInfo(&*I, *AA, *AST)));
	}

  virtual ValSet::const_iterator relevant_begin(const llvm::Instruction *I) const {
    return getInsInfo(I)->RC_begin();
  }
  virtual ValSet::const_iterator relevant_end(const llvm::Instruction *I) const {
    return getInsInfo(I)->RC_end();
  }

	virtual void clearMem();

  ValSet::const_iterator REF_begin(const llvm::Instruction *I) const {
    return getInsInfo(I)->REF_begin();
  }
  ValSet::const_iterator REF_end(const llvm::Instruction *I) const {
    return getInsInfo(I)->REF_end();
  }

  ValSet::const_iterator DEF_begin(const llvm::Instruction *I) const {
    return getInsInfo(I)->DEF_begin();
  }
  ValSet::const_iterator DEF_end(const llvm::Instruction *I) const {
    return getInsInfo(I)->DEF_end();
  }

  template<typename FwdValueIterator>
  bool addCriterion(const llvm::Instruction *ins, FwdValueIterator b,
		    FwdValueIterator const e, bool desliceIfChanged = false) {
    InsInfo *ii = getInsInfo(ins);
    //errs() << "addCriterion ";
    //ins->print(errs());
    //errs() << "\n";
    bool change = false;
    for (; b != e; ++b){
      //errs() << "try to addRC ";
      //b->first->print(errs());
      //errs() << "\n";
      if (ii->addRC(*b)){
        change = true;
      }
    }
    if (change && desliceIfChanged)
      ii->deslice();
    return change;
  }

	virtual void addInitialCriterion(const llvm::Instruction *ins,
			const Pointee &cond = Pointee(0, 0),
			bool deslice = true) {
		InsInfo *ii = getInsInfo(ins);
		if (cond.first){
			ii->addRC(cond);
		}
		ii->deslice();

	}
  virtual void calculateStaticSlice();
  virtual bool slice();
  //static void removeUndefs(ModulePass *MP, Function &F);

  void addSkipAssert(const llvm::CallInst *CI) {
    skipAssert.insert(CI);
  }

  bool shouldSkipAssert(const llvm::CallInst *CI) {
    return skipAssert.count(CI);
  }

  virtual InsInfo *getInsInfo(const llvm::Instruction *i) const {
    InsInfoMap::const_iterator I = insInfoMap.find(i);
    assert(I != insInfoMap.end());
    return I->second;
  }

protected:
  llvm::Function &fun;
  //llvm::ModulePass *MP;
  InsInfoMap insInfoMap;
  llvm::SmallSetVector<const llvm::CallInst *, 10> skipAssert;
	AliasSetTracker* AST;
	AliasAnalysis* AA;

  void crawlBasicBlock(const llvm::BasicBlock *bb);
  virtual bool computeRCi(InsInfo *insInfoi, InsInfo *insInfoj);
  virtual bool computeRCi(InsInfo *insInfoi);
  virtual void computeRC();

  virtual void computeSCi(const llvm::Instruction *i, const llvm::Instruction *j);

  virtual void computeSC();

  virtual bool computeBC();
  virtual bool updateRCSC(llvm::PostDominanceFrontier::DomSetType::const_iterator start,
                  llvm::PostDominanceFrontier::DomSetType::const_iterator end);

  virtual void dump();
  virtual bool canSlice(const Instruction &i);
	// InsInfo factory. Can be overriden
	virtual InsInfo* newInsInfo(const llvm::Instruction *I, 
				AliasAnalysis& AA, AliasSetTracker& AST){
		return new InsInfo(I, AA, AST);
	}


  //static void removeUndefBranches(ModulePass *MP, Function &F);
  //static void removeUndefCalls(ModulePass *MP, Function &F);
};

bool findInitialCriterion(llvm::Function &F, FunctionStaticSlicer &ss,
                          bool startingFunction = false);
bool handleCall(Function &F, FunctionStaticSlicer &ss, const CallInst *CI) ;

}}
#endif
