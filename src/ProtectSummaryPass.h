#include "WholePass.h"
#include "ProtectSummary.h"
#include "TimeGlobal.h"
#include <llvm/ADT/SetVector.h>

typedef std::map<llvm::BasicBlock *, ProtectSummary> BBSummary;
typedef llvm::SetVector<llvm::Function *> WorkQ;

class ProtectSummaryPass : public WholePass {

private:
	const unsigned MaxIterations;	

	// Just for processing. Will be discarded once FuncSum is created
	BBSummary BBSum;

	// Keeps the functions that needs to be evaluated. Initialized by all functions
	WorkQ FuncsQ;
	
	bool updateProtectFor(llvm::Function *);
	bool updateProtectFor(llvm::BasicBlock *);
	bool updateProtectFor(llvm::Instruction *);

	virtual void addInWorkQ( Function *F );

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
	ProtectSummaryPass(GlobalContext *Ctx_)
		: WholePass(Ctx_, "ProtectSummary"), MaxIterations(5) { }
	
	virtual bool doInitialization(llvm::Module &, llvm::ModulePass &);
	virtual bool doPass();
	virtual void done();

	// debug
	void dumpProtect();
};
