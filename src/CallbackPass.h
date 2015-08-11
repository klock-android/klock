#include "IterativeModulePass.h"
#include "TimeGlobal.h"
#include <set>
#include <queue>

typedef std::set<llvm::Function* > BigFuncSet;
typedef std::queue<llvm::Function* > FuncQ;

class CallbackPass: public IterativeModulePass {
	public: 
	bool doInitialization(Module *M);
	bool doModulePass(Module *M);
	bool doFinalization(Module *M) ;
	bool isRegistration(Function* F);
	CallbackPass(GlobalContext *Ctx_)
		: IterativeModulePass(Ctx_, "Callback") { }

	private:
	// This is like a workqueue. This contains the functions
	// that should completely be in TICS. This should include
	// callback functions and the functions called by the 
	// callback functions
	FuncQ toPutInTics;

	// toPutInTics is drained into alreadyInTics. So that we don't put
	// them back in toPutInTics
	BigFuncSet alreadyInTics;

	bool visitCallInst(CallInst *CI);
	bool visitInst(Instruction *I) ;
	bool visitBB(BasicBlock *BB) ;
	bool visitFunc(Function *F) ;
	bool addFnInTics( Function *F );
};
