#ifndef WHOLEPASS
#define WHOLEPASS
#include "TimeGlobal.h"
#include <llvm/Pass.h>
class WholePass {
protected:
	GlobalContext *Ctx;
	const char * ID;
public:
	WholePass(GlobalContext *Ctx_, const char *ID_)
		: Ctx(Ctx_), ID(ID_) { }

	//virtual ~WholePass() = 0;
	
	// run on each module before iterative pass
	virtual bool doInitialization(llvm::Module &M, llvm::ModulePass &MP)
		{ return true; }

	// for final cleanup
	virtual void done()
		{  }

	// iterative pass
	virtual bool doPass()
		{ return false; }

	virtual void run(ModuleList &modules, llvm::ModulePass& MP);

};
#endif
