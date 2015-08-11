#ifndef SLICING_FWDSTATICSLICERHELPER_H
#define SLICING_FWDSTATICSLICERHELPER_H

#include "StaticSlicerHelper.h"

using namespace llvm;

namespace llvm { namespace slicing {
  typedef std::map<const Pointee, const Pointee> ArgsToParams;

  class FwdStaticSlicerHelper: public StaticSlicerHelper {
    public:
      virtual void getRelevantVarsAtCall(llvm::CallInst const* const C,
          llvm::Function const* const F,
          const ValSet::const_iterator &b,
          const ValSet::const_iterator &e,
          RelevantSet &out);
  };
}}

#endif
