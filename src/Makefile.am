SUBDIRS = LLVMSlicer

AM_CXXFLAGS = `llvm-config --cxxflags` -Werror -Wall -g

lib_LTLIBRARIES    =  libklock.la
EXTRA_DIST         = llvm/DataLayout.h llvm/DebugInfo.h llvm/IRBuilder.h

all-local: libklock.la
	@cd $(top_builddir)/lib && $(LN_S) -f ../src/.libs/libklock.so

libklock_la_SOURCES = BSlice.cc BSlice.h 
libklock_la_SOURCES += FSlice.cc FSlice.h 
libklock_la_SOURCES += IFSlice.cc IFSlice.h 
libklock_la_SOURCES += Annotation.cc Annotation.h
libklock_la_SOURCES += CallGraph.cc CallGraph.h
libklock_la_SOURCES += CallbackPass.cc CallbackPass.h
libklock_la_SOURCES += ProtectSummaryPass.cc ProtectSummaryPass.h ProtectSummary.h
libklock_la_SOURCES += ProtectPass.cc ProtectPass.h
libklock_la_SOURCES += TimeGlobal.h TimeGlobal.cc
libklock_la_SOURCES += IterativeModulePass.cc IterativeModulePass.h 
libklock_la_SOURCES +=  WholePass.cc WholePass.h
libklock_la_LIBADD  = LLVMSlicer/libslicer.la
libklock_la_LDFLAGS = -module
libklock_la_CFLAGS = $(AM_CXXFLAGS)
