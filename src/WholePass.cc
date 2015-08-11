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

#include "WholePass.h"

using namespace llvm;

#define Diag if (false) llvm::errs()

void WholePass::run(ModuleList &modules, ModulePass &MP) {

	ModuleList::iterator i, e;
	Diag << "[" << ID << "] Initializing " << modules.size() << " modules ";
	for (i = modules.begin(), e = modules.end(); i != e; ++i) {
		doInitialization(*(i->first), MP);
		Diag << ".";
	}
	Diag << "\n";

	doPass();
		
	done();
	Diag << "[" << ID << "] Done!\n";
}
