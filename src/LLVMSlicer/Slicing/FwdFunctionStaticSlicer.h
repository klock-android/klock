// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

#ifndef SLICING_FWDFUNCTIONSTATICSLICER_H
#define SLICING_FWDFUNCTIONSTATICSLICER_H

#include <map>
#include <utility> /* pair */

#include "llvm/IR/Value.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Support/InstIterator.h"

#include "PostDominanceFrontier.h"
#include "FunctionStaticSlicer.h"

namespace llvm { namespace slicing {

class ExtInsInfo: public InsInfo {
public:
	ExtInsInfo(const llvm::Instruction *i, AliasAnalysis& AA, 
			AliasSetTracker &AST): InsInfo(i, AA, AST){}

  virtual bool isSliced() const { return sliced; }
/*
  //Commenting while assuming that fwd slicing and bwd 
  //slicing are not being done simultaneously

  virtual bool addRC(const Pointee &var) { return fRC.insert(var); }
  virtual ValSet::const_iterator RC_begin() const { return fRC.begin(); }
  virtual ValSet::const_iterator RC_end() const { return fRC.end(); }

private:
	ValSet fRC;*/
};
class FwdFunctionStaticSlicer : public FunctionStaticSlicer {
public:
  FwdFunctionStaticSlicer(llvm::Function &F, AliasAnalysis* aa, AliasSetTracker* ast) :
	  FunctionStaticSlicer(F, aa, ast) {

  }
  ~FwdFunctionStaticSlicer();

  ValSet::const_iterator DEF_begin(const llvm::Instruction *I) const {
    return getInsInfo(I)->DEF_begin();
  }
  ValSet::const_iterator DEF_end(const llvm::Instruction *I) const {
    return getInsInfo(I)->DEF_end();
  }
	

  virtual bool slice();
  virtual void calculateStaticSlice();


protected:
	virtual InsInfo* newInsInfo(const llvm::Instruction *I, 
				AliasAnalysis& AA, AliasSetTracker& AST){
		return new ExtInsInfo(I, AA, AST);
	}

	virtual ExtInsInfo* getInsInfo(const llvm::Instruction *i) const {
		return static_cast<ExtInsInfo*>(FunctionStaticSlicer::getInsInfo(i));
  }

  virtual bool computeRCi(InsInfo *insInfoi, InsInfo *insInfoj);
  virtual bool computeRCi(InsInfo *insInfoi);
  virtual void computeRC();

  virtual void computeSCi(const llvm::Instruction *i, const llvm::Instruction *j);
  virtual void computeSC();

  virtual void dump();

};

bool findfwdInitialCriterion(llvm::Function &F, FwdFunctionStaticSlicer &ss,
                          bool startingFunction = false);


}}

#endif
