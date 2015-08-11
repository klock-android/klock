// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

#ifndef SLICING_INTRSCTFWDFUNCTIONSTATICSLICER_H
#define SLICING_INTRSCTFWDFUNCTIONSTATICSLICER_H

#define DEBUG_TYPE "ifslice"

#include <map>
#include <utility> /* pair */

#include "llvm/IR/Value.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Support/InstIterator.h"

#include "PostDominanceFrontier.h"
#include "FwdFunctionStaticSlicer.h"

using namespace llvm;
// This stores id pair of two slices that were involved in an arithmetic
typedef std::set< std::pair<int, int> > TimePairs;

// This stores < slice_id, number of statements in slice > for each slice_id
typedef std::map< int, int > SliceCount;

namespace llvm { namespace slicing {

class IntrsctInfo: public ExtInsInfo {
	//typedef llvm::SmallSetVector<int, 8> SliceSet;
	typedef std::set<int> SliceSet;
  public:
		IntrsctInfo(const llvm::Instruction *i, AliasAnalysis& AA, 
			AliasSetTracker &AST): ExtInsInfo(i, AA, AST){
				inSlice.clear();
				DEBUG( errs() << "IntrsctInfo cons " << inSlice.size() << "\n");
		}
    /*IntrsctInfo(const llvm::Instruction *i, const llvm::ptr::PointsToSets &PS,
                     const llvm::mods::Modifies &MOD): ExtInsInfo(i, PS, MOD){}*/
    void clear(){
      // Remove all the relevant variables
      RC.clear();
    }
    virtual void deslice() { 
      // Add the slice count number into the set
			DEBUG (errs() << "deslice " << count << " " );
			DEBUG (ins->print( errs() ));
			DEBUG (errs() << "\n");
      inSlice.insert(count);
			for( SliceSet::iterator b = inSlice.begin(), e = inSlice.end();
					b != e; b++){
				DEBUG ( errs() << "In slice " << *b << "\n");
			}
    }
		// Is the instruction in CURRENT slice
		virtual bool isSliced() const { 
			if( inSlice.empty() ) return true;
			// This is because inSlice cannot be have slice id > count
			//if( inSlice.back() == count ) return false;
			//return true;
			if( inSlice.find(count) == inSlice.end() ) return true;
			return false;
		}

		bool isInAnySlice() const {
			bool ret = !( inSlice.empty() );
			int size = inSlice.size();
			DEBUG( errs() << "isInAnySlice " << ret << "\t" << size << "\n" );
			return ret;
		}

		void getAllPairs(TimePairs &tp){
			for( SliceSet::iterator b = inSlice.begin(), e = inSlice.end();
					b != e; b++){
				for( SliceSet::iterator bb = inSlice.begin(), ee = inSlice.end();
						bb != ee; bb++){
					int x = *b;
					int y = *bb;
					if( x < y ){
						tp.insert( std::make_pair(x,y) );
						DEBUG( errs() << "Adding to TimePairs " << x << "< " << y << "\n" );
					} 
				}
			}
		}

		void addSliceCount( SliceCount &sliceCount ){
			for( SliceSet::iterator b = inSlice.begin(), e = inSlice.end();
					b != e; b++){
				sliceCount[*b]++;	
			}
		}
		
		static int newCount(){
			return ++count;
		}
   
  private:
    // A statement can be in several different slices
    SliceSet inSlice;
		// The count shows the current slice and should be common
		// across all the instruction info's
    static int count;
		static std::map<int, Instruction* > initCrits;
};

typedef std::map<int, const Instruction*> InitCrits;

class IntrsctFwdFunctionStaticSlicer : public FwdFunctionStaticSlicer {
public:
  IntrsctFwdFunctionStaticSlicer(llvm::Function &F, AliasAnalysis* aa, AliasSetTracker* ast) :
	  FwdFunctionStaticSlicer(F, aa, ast) {

  }

	static InitCrits initCrits;

	virtual void addInitialCriterion(const llvm::Instruction *ins,
			   const Pointee &cond = Pointee(0, 0),
				 bool deslice=true) {
		errs() << "addInitialCriterion called\n";
		initCrits.insert(std::make_pair(IntrsctInfo::newCount(), ins));
		FwdFunctionStaticSlicer::addInitialCriterion(ins, cond, deslice);
	}
  
  ~IntrsctFwdFunctionStaticSlicer();
	bool isInAnySlice(const llvm::Instruction* I){
		IntrsctInfo* II = getInsInfo( I );
		return II->isInAnySlice();
	}
	void newSlice() ;
	
	void getAllPairs( TimePairs &arithPairs ){
		for (inst_iterator I = inst_begin(fun), E = inst_end(fun); I != E; I++){
			getInsInfo( &*I )->getAllPairs( arithPairs );
		}
	}

	void addSliceCount( SliceCount &sliceCount ){
		for (inst_iterator I = inst_begin(fun), E = inst_end(fun); I != E; I++){
			getInsInfo( &*I )->addSliceCount( sliceCount );
		}
	}

protected:
	virtual InsInfo* newInsInfo(const llvm::Instruction *I, 
				AliasAnalysis& AA, AliasSetTracker& AST){
		DEBUG( errs() << "newInsInfo called \n" );
		return new IntrsctInfo(I, AA, AST);
	}

	virtual IntrsctInfo* getInsInfo(const llvm::Instruction *i) const {
		return static_cast<IntrsctInfo*>(FwdFunctionStaticSlicer::getInsInfo(i));
  }

  //virtual void dump();

};

bool findIntrsctInitialCriterion(const llvm::Function &F, 
			IntrsctFwdFunctionStaticSlicer &ss);


}}

#endif
