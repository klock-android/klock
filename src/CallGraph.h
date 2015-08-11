#include "IterativeModulePass.h"
#include "TimeGlobal.h"
#include "Annotation.h"

class CallGraphPass : public IterativeModulePass {
private:
	bool runOnFunction(llvm::Function *);
	void processInitializers(llvm::Module *, llvm::Constant *, llvm::GlobalValue *);
	bool mergeFuncSet(FuncSet &S, const std::string &Id);
	bool mergeFuncSet(FuncSet &Dst, const FuncSet &Src);
	bool findFunctions(llvm::Value *, FuncSet &);
	bool findFunctions(llvm::Value *, FuncSet &, 
	                   llvm::SmallPtrSet<llvm::Value *, 4>);
	void initCallSites();

public:
	CallGraphPass(GlobalContext *Ctx_)
		: IterativeModulePass(Ctx_, "CallGraph") { }
	virtual bool doInitialization(llvm::Module *);
	virtual bool doFinalization(llvm::Module *);
	virtual bool doModulePass(llvm::Module *);

	// debug
	void dumpFuncPtrs();
	void dumpCallees();
};
