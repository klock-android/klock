#ifndef FSLICE_H
#define FSLICE_H

#include <map>
#include <set>
#include <vector>
#include <iterator>
#include <algorithm>

#include "BSlice.h"

#include <llvm/ADT/STLExtras.h> /* tie */
#include <llvm/Analysis/PostDominators.h>
#include "LLVMSlicer/Slicing/FwdStaticSlicerHelper.h"

class FSlice: public BSlice {
	public:
		FSlice(GlobalContext *Ctx_) : BSlice(Ctx_) { }
		virtual void computeSlice();

	protected:
		virtual void runFSS(Function &F) ;

		virtual void initHelper() {
			helper = new FwdStaticSlicerHelper();
		}

		template<typename OutIterator>
					void addGlobals(RelevantSet R, OutIterator out);

		template<typename OutIterator>
			void toCalls(llvm::Function const* const f, OutIterator out);

		template<typename OutIterator>
			void toExits(llvm::Function const* const f, OutIterator out);
};
#endif
