AM_CXXFLAGS = `llvm-config --cxxflags` -g

noinst_LTLIBRARIES = libslicer.la
libslicer_la_CPPFLAGS = -I$(top_builddir)/lib
libslicer_la_SOURCES  = Languages/LLVM.cpp Languages/LLVM.h
libslicer_la_SOURCES += Slicing/PostDominanceFrontier.cpp Slicing/PostDominanceFrontier.h
libslicer_la_SOURCES +=	Slicing/Helper.cpp Slicing/Helper.h
libslicer_la_SOURCES +=	Slicing/StaticSlicerHelper.cpp Slicing/StaticSlicerHelper.h
libslicer_la_SOURCES += Slicing/FwdStaticSlicerHelper.cpp Slicing/FwdStaticSlicerHelper.h
libslicer_la_SOURCES += Slicing/FunctionStaticSlicer.cpp Slicing/FunctionStaticSlicer.h
libslicer_la_SOURCES += Slicing/FwdFunctionStaticSlicer.cpp Slicing/FwdFunctionStaticSlicer.h
libslicer_la_SOURCES += Slicing/IntrsctFwdFunctionStaticSlicer.h Slicing/IntrsctFwdFunctionStaticSlicer.cpp

libslicer_la_LDFLAGS  = -L$(top_builddir)/lib
