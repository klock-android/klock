#include "IterativeModulePass.h"
#include "TimeGlobal.h"
#include "ProtectSummary.h"

typedef std::map<llvm::BasicBlock *, Protection> FuncProtection;
typedef std::map<llvm::CallInst *, Protection> CallEntries;

class ProtectPass : public IterativeModulePass {

private:
	const unsigned MaxIterations;	

	// This will hold the protection mechanisms before the BasicBlock
	FuncProtection FuncProtIn;

	// This will hold the protection mechanisms after the BasicBlock
	FuncProtection FuncProtOut;

	// This will hold the possible protection entry for call site
	// it holds the worst case protection scenario. It is supposed 
	// to answer whether this call can be unprotected 
	CallEntries CallEnt;

	void printProtection( raw_ostream &ro, Protection P ){
		for( Protection::iterator it = P.begin(), e = P.end(); it!=e; ++it){
			ro << *it;
		}
	}

	Protection findInitProtection( Function *F );
	void initCallSites();

	bool updateProtectFor(llvm::Function *);
	bool updateProtectFor(llvm::BasicBlock *);
	bool updateProtectFor(llvm::Instruction *);

	typedef std::pair<const llvm::BasicBlock *, const llvm::BasicBlock *> Edge;
	typedef llvm::SmallVector<Edge, 16> EdgeList;
	EdgeList BackEdges;
	
	bool isBackEdge(const Edge &);
	
	bool visitCallInst(llvm::CallInst *);

	// TODO: branch instruction wl.isHeld() should
	// be passed to the next basic block. Ignoring such cases for now.
	//void visitBranchInst(llvm::BranchInst *, 
	//					 llvm::BasicBlock *, ValueRangeMap &);
	//void visitTerminator(llvm::TerminatorInst *,
	//					 llvm::BasicBlock *, ValueRangeMap &);
	//void visitSwitchInst(llvm::SwitchInst *, 
	//					 llvm::BasicBlock *, ValueRangeMap &);

public:
	ProtectPass(GlobalContext *Ctx_)
		: IterativeModulePass(Ctx_, "Protect"), MaxIterations(5) { }
	
	virtual bool doInitialization(llvm::Module *);
	virtual bool doModulePass(llvm::Module *M);
	virtual bool doFinalization(llvm::Module *);
	virtual void done();
	// debug
	void dumpProtect();
};
