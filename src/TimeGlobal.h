#pragma once

#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/DebugInfo.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instructions.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/Support/ConstantRange.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Pass.h>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "ProtectSummary.h"

typedef std::vector< std::pair<llvm::Module *, llvm::StringRef> > ModuleList;
typedef llvm::SmallPtrSet<llvm::Function *, 8> FuncSet;
typedef std::map<llvm::StringRef, llvm::Function *> FuncMap;
typedef std::map<std::string, FuncSet> FuncPtrMap;
typedef llvm::DenseMap<llvm::CallInst *, FuncSet> CalleeMap;
typedef std::set<llvm::StringRef> DescSet;

typedef std::map<llvm::Function *, ProtectSummary> FuncSummary;
typedef std::map<llvm::Function *, Protection> FuncEntries;
typedef std::set<llvm::Instruction *> InstructionSet;
typedef std::set<llvm::CallInst *> CISet;
typedef llvm::DenseMap<llvm::Function *, CISet> CallSiteMap;

class GlobalContext {
	public:
	// Map global function name to function defination
	FuncMap Funcs;

	// Map function pointers (IDs) to possible assignments
	FuncPtrMap FuncPtrs;
	
	// Map a callsite to all potential callees
	CalleeMap Callees;

	// This will be used to query all calls for function
	CallSiteMap CallSites;

	// This holds all the instructions that are part of TICS
	InstructionSet TICS;

	// This will hold the protection summary of function, for example,
	// this functions acquires wake_lock in all its paths or releases
	// wake_lock in one of the paths (conservative)
	FuncSummary FuncSum;

	// This will hold the possible protection entry for function, for example,
	// if function is called from 20 call sites, it holds the worst case
	// protection scenario. It is supposed to answer whether the function 
	// can be called from an unprotected call site? Should be calced by 
	// merging CallEntries
	//FuncEntries FuncEnt;

	// This holds all the instructions that are protected
	InstructionSet SAFE;

	virtual ~GlobalContext(){
		Funcs.clear();
		FuncPtrs.clear();
		Callees.clear();
		FuncSum.clear();
		SAFE.clear();
		TICS.clear();
		CallSites.clear();
	}

};

class TimeGlobal : public ModulePass {
	private:
	GlobalContext GlobalCtx;

	public:
		static char ID;
		TimeGlobal() : ModulePass(ID) { }
		virtual bool doInitialization(Module &);
		virtual bool runOnModule(Module &M);
		virtual bool doFinalization(Module &);

		virtual void getAnalysisUsage( AnalysisUsage &AU ) const{
			AU.addRequired<AliasAnalysis>();
			AU.addPreserved<AliasAnalysis>();
		}
};
