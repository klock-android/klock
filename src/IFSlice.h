#ifndef IFSLICE_H
#define IFSLICE_H

#include <map>
#include <set>
#include <vector>
#include <iterator>
#include <algorithm>

#include "FSlice.h"

#include <llvm/ADT/STLExtras.h> /* tie */
#include <llvm/Analysis/PostDominators.h>

class IFSlice: public FSlice {
	public:
		IFSlice(GlobalContext *Ctx_) : FSlice(Ctx_) { }
		virtual void computeSlice();
		virtual void done();

	protected:
		virtual void runFSS(Function &F) ;
		void newSlice() ;

};
#endif
