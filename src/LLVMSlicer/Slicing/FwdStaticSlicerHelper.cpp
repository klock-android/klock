#define DEBUG_TYPE "fwsshelper"
#include "FwdStaticSlicerHelper.h"

namespace llvm { namespace slicing { 

    // C is the call instruction. F is the function being called.
    void FwdStaticSlicerHelper::getRelevantVarsAtCall(llvm::CallInst const* const C,
			       llvm::Function const* const F,
			       const ValSet::const_iterator &_b, // Relevant vars begin
			       const ValSet::const_iterator &e,  // Relevant vars end
             RelevantSet &out) {

      DEBUG( errs()  << "getRelevantVarsAtCall " );
      DEBUG( C->print(errs()) );
      DEBUG( errs() << F->getName() << "\n" );

      assert(!isInlineAssembly(C) && "Inline assembly is not supported!");
      const Function *caller = C->getParent()->getParent();

      ArgsToParams toParams;
      fillArgsToParams(C, F, toParams);

      // Iterate on all relevant vars
      for (ValSet::const_iterator b(_b); b != e; ++b) {
        DEBUG( errs() << "Check: " << b->first->getName() << "\n" );
        ArgsToParams::const_iterator it = toParams.find(*b);
        if (it != toParams.end()){
          // Insert all the relevant arguments
          out.insert(it->second);
          DEBUG( "Insert argument " );
          DEBUG( errs() << it->second.first->getName() );
          DEBUG( "\n" );
        //} else if (!isLocalToFunction(b->first, F)) {
        } else if (!isLocalToFunction(b->first, caller)) {
          // Insert global relevant variables
          out.insert(*b);
          DEBUG( "Insert global " );
          DEBUG( errs() << b->first->getName() );
          DEBUG( "\n" );
        }
      }
    }

}}

