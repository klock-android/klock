#define DEBUG_TYPE "timeg"
#include <llvm/IR/LLVMContext.h>
#include <llvm/PassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/PrettyStackTrace.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/SystemUtils.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/Signals.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/SourceMgr.h>
#include <memory>
#include <vector>
#include <time.h>

#include "ProtectPass.h"
#include "ProtectSummaryPass.h"
#include "CallGraph.h"
#include "CallbackPass.h"
#include "BSlice.h"
#include "FSlice.h"
#include "IFSlice.h"

using namespace llvm;

ModuleList Modules;

time_t begin = 0;
time_t end = 0;
static void printElapsedTime(){
	DEBUG( dbgs() << "begin " << begin << "\n" );
	if ( begin != 0 ){
		//sleep(1);
		time( &end );
		double elapsed_secs = difftime( end, begin );
		DEBUG( dbgs() << "Time " << elapsed_secs << "\n" );
		begin = end;
	} else
		time( &begin );
}

void dumpInstructionSet( InstructionSet IS ){
	/*for (InstructionSet::iterator ii = IS.begin(), 
		ee = IS.end(); ii != ee; ++ii) {
		DEBUG( (*ii)->print(dbgs()) );
		DEBUG( dbgs() << "\n");
	}*/
	DEBUG( dbgs() << IS.size() << "\n" );
}

bool TimeGlobal::doInitialization(Module &M){ return false;}
bool TimeGlobal::doFinalization(Module &M){ return false;}


bool TimeGlobal::runOnModule(Module &M) {

	Modules.push_back(std::make_pair(&M, "Input"));

	DEBUG( dbgs() << "Starting CallGraphPass \n");
	printElapsedTime();

	static AnnotationPass AnnoPass;
	AnnoPass.doInitialization(M);
	for (Module::iterator j = M.begin(), je = M.end(); j != je; ++j)
		AnnoPass.runOnFunction(*j);

	CallGraphPass CGPass(&GlobalCtx);
	CGPass.run(Modules);

	CGPass.dumpCallees();

	DEBUG( dbgs() << "Finished CallGraphPass \n");

	DEBUG( dbgs() << "Starting CallbackPass \n");
	printElapsedTime();
	CallbackPass CBPass(&GlobalCtx);
	CBPass.run(Modules);
	//for( long ik =0 ; ik < 1000000; ik ++){ errs() << "test\n"; }
	printElapsedTime();
	DEBUG( dbgs() << "Finished CallbackPass \n");

	DEBUG( dbgs() << "Starting BSlice \n");
	printElapsedTime();
	BSlice BS(&GlobalCtx);
	BS.run(Modules, *static_cast<ModulePass*>(this));
	//for( long ik =0 ; ik < 1000000; ik ++){ errs() << "test\n"; }
	printElapsedTime();
	DEBUG( dbgs() << "Finished BSlice \n");

	/*DEBUG( dbgs() << "Starting FSlice \n");
	printElapsedTime();
	FSlice FS(&GlobalCtx);
	FS.run(Modules, *static_cast<ModulePass*>(this));
	//for( long ik =0 ; ik < 1000000; ik ++){ errs() << "test\n"; }
	printElapsedTime();
	DEBUG( dbgs() << "Finished FSlice \n");*/

	DEBUG( dbgs() << "Starting IFSlice \n");
	printElapsedTime();
	IFSlice IFS(&GlobalCtx);
	IFS.run(Modules, *static_cast<ModulePass*>(this));
	//for( long ik =0 ; ik < 1000000; ik ++){ errs() << "test\n"; }
	printElapsedTime();
	DEBUG( dbgs() << "Finished IFSlice \n");

	DEBUG( dbgs() << "Printing TICS \n");
	printElapsedTime();
	dumpInstructionSet( GlobalCtx.TICS );
	printElapsedTime();
	DEBUG( dbgs() << "Printing TICS done \n");

	DEBUG( dbgs() << "Starting ProtectSummaryPass \n");
	printElapsedTime();

	ProtectSummaryPass PSPass(&GlobalCtx);
	PSPass.run(Modules, *static_cast<ModulePass*>(this));
	
	printElapsedTime();
	DEBUG( dbgs() << "Finished ProtectSummaryPass \n");
	DEBUG( dbgs() << "Starting ProtectPass \n");
	printElapsedTime();

	ProtectPass PPass(&GlobalCtx);
	PPass.run(Modules);

	printElapsedTime();
	DEBUG( dbgs() << "Finished ProtectPass \n");

	DEBUG( dbgs() << "Printing SAFE \n");
	printElapsedTime();
	dumpInstructionSet( GlobalCtx.SAFE );
	printElapsedTime();
	DEBUG( dbgs() << "Printing SAFE done \n");

	printElapsedTime();
	// Spit all unprotected statements
	for (InstructionSet::iterator ii=GlobalCtx.TICS.begin(), 
				ee = GlobalCtx.TICS.end(); ii!=ee; ii++ ){
		if( GlobalCtx.SAFE.find(*ii) == GlobalCtx.SAFE.end() ){
			DEBUG( dbgs() << "Unprotected TICS: ");
		} else{
			DEBUG( dbgs() << "Protected TICS: ");
		}

		DEBUG( dbgs() << (*ii)->getParent()->getParent()->getName() );
		DEBUG( (*ii)->print(dbgs()) );
		DEBUG( dbgs() << "\n");
	}
	printElapsedTime();

	//PSPass.dumpProtect();
	//PPass.dumpProtect();

	return false;
}

char TimeGlobal::ID = 0;
static RegisterPass<TimeGlobal> X("klock", "TimeGlobal",
		false /* Only looks at CFG */,
		false /* Analysis Pass */);

