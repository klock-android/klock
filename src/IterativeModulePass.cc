#include "llvm/IR/LLVMContext.h"
#include "llvm/PassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/SystemUtils.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/SourceMgr.h"
#include <memory>
#include <vector>

#include "IterativeModulePass.h"

using namespace llvm;

#define Diag if (false) llvm::errs()

void IterativeModulePass::run(ModuleList &modules) {

	ModuleList::iterator i, e;
	Diag << "[" << ID << "] Initializing " << modules.size() << " modules ";
	for (i = modules.begin(), e = modules.end(); i != e; ++i) {
		doInitialization(i->first);
		Diag << ".";
	}
	Diag << "\n";

	unsigned iter = 0, changed = 1;
	while (changed) {
		++iter;
		changed = 0;
		for (i = modules.begin(), e = modules.end(); i != e; ++i) {
			Diag << "[" << ID << " / " << iter << "] ";
			Diag << "'" << i->first->getModuleIdentifier() << "'";

			bool ret = doModulePass(i->first);
			if (ret) {
				++changed;
				Diag << " [CHANGED]\n";
			} else
				Diag << "\n";
		}
		Diag << "[" << ID << "] Updated in " << changed << " modules.\n";
	}

	Diag << "\n[" << ID << "] Postprocessing ...\n";
	for (i = modules.begin(), e = modules.end(); i != e; ++i) {
		if (doFinalization(i->first) ){
			/*if( !NoWriteback) {
				Diag << "[" << ID << "] Writeback " << i->second << "\n";
				doWriteback(i->first, i->second);
			}*/
		}
	}
			
	done();
	Diag << "[" << ID << "] Done!\n";
}
