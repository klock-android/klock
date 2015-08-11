#ifndef BSLICE_H
#define BSLICE_H
#include "WholePass.h"
#include "TimeGlobal.h"
#include "LLVMSlicer/Slicing/FunctionStaticSlicer.h"
#include "LLVMSlicer/Slicing/StaticSlicerHelper.h"

using namespace llvm;
using namespace llvm::slicing;

typedef std::map<Function const*, FunctionStaticSlicer *> Slicers;

class BSlice : public WholePass {

private:
	const unsigned MaxIterations;	
	template<typename OutIterator>
		void emitToCalls(Function const* const f, OutIterator out);

	template<typename OutIterator>
		void emitToExits(Function const* const f, OutIterator out);

protected:
	StaticSlicerHelper* helper;
	virtual void runFSS(Function &F) ;
	
	virtual void initHelper() {
		helper = new StaticSlicerHelper();
	}
	Slicers slicers;
	virtual void computeSlice();
	virtual bool sliceModule();
	typedef SmallVector<const Function *, 20> InitFuns;
	InitFuns initFuns;

	AliasAnalysis* AA;
	AliasSetTracker* AST;

public:
	BSlice(GlobalContext *Ctx_)
		: WholePass(Ctx_, "Protect"), MaxIterations(5) { }
	virtual bool doInitialization(Module &, ModulePass &);
	virtual bool doPass();
	virtual void done();

	virtual void getAnalysisUsage( AnalysisUsage &AU ) const{
		AU.addRequired<AliasAnalysis>();
		AU.addPreserved<AliasAnalysis>();
	}

	// debug
	void dumpProtect();
};
#endif
