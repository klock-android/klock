#define DEBUG_TYPE "sshelper"
#include "StaticSlicerHelper.h"
namespace llvm { namespace slicing { 

    // C is the call instruction. F is the function being called.
    void StaticSlicerHelper::getRelevantVarsAtCall(llvm::CallInst const* const C,
			       llvm::Function const* const F,
			       const ValSet::const_iterator &_b, // Relevant vars begin
			       const ValSet::const_iterator &e,  // Relevant vars end
             RelevantSet &out) {

      DEBUG( errs() << "getRelevantVarsAtCall ");
      DEBUG( C->print(errs()));
      DEBUG( errs() << F->getName() << "\n");

      assert(!isInlineAssembly(C) && "Inline assembly is not supported!");
      const Function *caller = C->getParent()->getParent();

      ParamsToArgs toArgs;
      fillParamsToArgs(C, F, toArgs);

      // Iterate on all relevant vars
      for (ValSet::const_iterator b(_b); b != e; ++b) {
        DEBUG( errs() << "Check: " << b->first->getName() << "\n");
        ParamsToArgs::const_iterator it = toArgs.find(*b);
        if (it != toArgs.end()){
          // Insert all the relevant arguments
          out.insert(it->second);
          DEBUG( errs() << "Argument" << it->second.first->getName());
        } else if (!isLocalToFunction(b->first, F)) {
        //} else if (!isLocalToFunction(b->first, caller)) {
          // Insert global relevant variables
          out.insert(*b);
          DEBUG(errs() << "Global var" << b->first->getName());
        }
      }
    }

		// Just take care of the return value here !!! Don't add other values
    void StaticSlicerHelper::getRelevantVarsAtExit(const llvm::CallInst *const C,
        const llvm::ReturnInst *const R,
        ValSet::const_iterator &b,
        const ValSet::const_iterator &e,
        RelevantSet &out) {
      assert(!isInlineAssembly(C) && "Inline assembly is not supported!");

      /*if (callToVoidFunction(C)) {
        std::copy(b, e, std::inserter(out, out.begin()));
        return;
      }*/

      for ( ; b != e; ++b)
        if (b->first == C) {
          Value *ret = R->getReturnValue();
          if (!ret) {
            /*			    C->dump();
                        C->getCalledValue()->dump();
                        R->dump();*/
            //			    abort();
            return;
          }
          out.insert(Pointee(ret, -1));
        } //else
          //out.insert(*b);
    }

		// Parameters that are passed as pointer should be relevant. 
		void StaticSlicerHelper::getRelevantPtrParams(
				const Function *const F, // Function to check arguments from 
				AliasAnalysis* AA,
        const ValSet::const_iterator &b,	// Relevant variables at function exit
        const ValSet::const_iterator &e,
				RelevantSet &out) {

			for ( ValSet::const_iterator ii = b; ii != e; ii ++ ){
				for( Function::const_arg_iterator  jj = F->arg_begin(), ejj = F->arg_end(); 
						jj != ejj; jj ++ ){
					if( jj->getType()->isPointerTy() ){
						DEBUG( errs() << "Check function argument " );
						DEBUG( jj->print( errs() ));
						DEBUG( ii->first->print( errs() ));
						DEBUG( errs() << "\n " );

						if( AA->alias( ii->first, &*jj ) != AliasAnalysis::NoAlias ){
							DEBUG( errs() << "Found relevant function argument " );
							DEBUG( jj->print( errs() ));
							DEBUG( errs() << "\n " );
							out.insert( Pointee( &*jj, -1) );
						}
					}
				}
			}
		}
     

}}


