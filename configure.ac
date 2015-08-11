#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([klock],[0.1],[klock.android@gmail.com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([foreign -Wall -Werror])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Checks for programs.
AM_PROG_CC_C_O
AC_PROG_CXX
m4_ifdef([AM_PROG_AR],[AM_PROG_AR])
AC_PROG_LN_S
AC_PROG_MKDIR_P

LT_INIT([disable-static pic-only])
AC_PROG_LIBTOOL

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/LLVMSlicer/Makefile
	lib/Makefile
])
AC_OUTPUT
