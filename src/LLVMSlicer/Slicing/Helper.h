#ifndef SLICING_HELPER_H
#define SLICING_HELPER_H

#include <map>
#include <set>
#include <vector>
#include <iterator>
#include <algorithm>

#include "llvm/ADT/STLExtras.h" /* tie */
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

#include "../Languages/LLVM.h"
#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/Support/Debug.h>

using namespace llvm;

namespace llvm { namespace slicing { 
	typedef const llvm::Value *MemoryLocation;
	typedef std::pair<MemoryLocation, int> Pointee;
	typedef std::map<const Pointee, const Pointee> ArgsToParams;
	typedef std::map<const Pointee, const Pointee> ParamsToArgs;
	typedef std::set<Pointee> RelevantSet;

	class Helper {
		public:
			void fillParamsToArgs(llvm::CallInst const* const C,
					llvm::Function const* const F,
					ParamsToArgs& toArgs);

			void fillArgsToParams(CallInst const* const C,
					Function const* const F,
					ArgsToParams& toParams);

			//static bool sameValues(const Pointee &val1, const Pointee &val2, AliasAnalysis* AA);
			static bool checkValid( const Pointee &var1, const Pointee &var2 );
	};
	extern const Function* getParent(const Value *V);
}}

#endif
