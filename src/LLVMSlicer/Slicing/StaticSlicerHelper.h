#ifndef SLICING_STATICSLICERHELPER_H
#define SLICING_STATICSLICERHELPER_H

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

#include "Helper.h"
#include "FunctionStaticSlicer.h"
#include "../Languages/LLVM.h"

using namespace llvm;


namespace llvm { namespace slicing { 
  typedef std::map<const Pointee, const Pointee> ParamsToArgs;
  typedef std::set<Pointee> RelevantSet;

  class StaticSlicerHelper : public Helper {
    public:
      virtual void getRelevantVarsAtCall(llvm::CallInst const* const C,
          llvm::Function const* const F,
          const ValSet::const_iterator &b,
          const ValSet::const_iterator &e,
          RelevantSet &out);

      virtual void getRelevantVarsAtExit(llvm::CallInst const* const C,
          llvm::ReturnInst const* const R,
          ValSet::const_iterator &b,
          const ValSet::const_iterator &e,
          RelevantSet &out);

			virtual void getRelevantPtrParams(
				const Function *const F, // Function to check arguments from 
				AliasAnalysis* AA,			// AliasAnalysis for F
        const ValSet::const_iterator &b,	// Relevant variables at function exit
        const ValSet::const_iterator &e,
				RelevantSet &out);

  };

}}

#endif
