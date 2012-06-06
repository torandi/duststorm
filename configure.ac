AC_INIT([frobnicator],[1],[ext@sidvind.com])

AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([1.11 foreign color-tests subdir-objects -Wall -Werror])
AM_SILENT_RULES([yes])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AH_BOTTOM([#include "forward.hpp"])

AC_GNU_SOURCE
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CXX
AC_PROG_RANLIB
AX_CHECK_COMPILE_FLAG([-std=c++0x], [CXXFLAGS="$CXXFLAGS -std=c++0x"])

AX_ASSIMP
AX_CHECK_CL
AX_CHECK_GL
AX_CHECK_GLU
AX_GLEW
AX_GLM
AM_PATH_SDL

AC_ARG_ENABLE([input], [AS_HELP_STRING([--enable-input], [Enable mouse- and keyboard debugging/development. @<:@default=disabled@:>@])])
AS_IF([test "x$enable_input" = "xyes"],
	[AC_DEFINE([ENABLE_INPUT],  [1], [Define to 1 to enable input])],
)
AM_CONDITIONAL(INPUT_ENABLED, test "x$enable_input" = "xyes")

AC_ARG_ENABLE([editor], [AS_HELP_STRING([--enable-editor], [Build editor. @<:@default=disabled@:>@])])
AM_CONDITIONAL([BUILD_EDITOR], [test "x$enable_editor" = "xyes"])

AC_DEFINE_UNQUOTED([PATH_BASE],    ["${srcdir}/"],             [Base path])
AC_DEFINE_UNQUOTED([PATH_MODELS],  ["${srcdir}/models/"],      [Path to models])
AC_DEFINE_UNQUOTED([PATH_SRC],     ["${srcdir}/src/"],         [Path to sources])
AC_DEFINE_UNQUOTED([PATH_SHADERS], ["${srcdir}/shaders/"],     [Path to shaders])
AC_DEFINE_UNQUOTED([PATH_OPENCL],  ["${srcdir}/cl_programs/"], [Path to OpenCL programs])
AC_DEFINE_UNQUOTED([PATH_ICON],    ["${srcdir}/icons/"],       [Path to editor icons])

AM_PATH_CPPUNIT(1.9.6,,[AC_MSG_NOTICE([cppunit not found, tests disabled])])
AM_CONDITIONAL([BUILD_TESTS], [test "x$no_cppunit" != "xyes"])

AC_OUTPUT
