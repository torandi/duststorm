AC_INIT([frobnicator-demo-engine],[1],[ext@sidvind.com])

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

AS_IF([test "x$no_gl" == "xyes"], [AC_MSG_ERROR([OpenGL headers required])])
AS_IF([test "x$no_cl" == "xyes"], [AC_MSG_ERROR([OpenCL headers required.

On debian/ubuntu: "apt-get install opencl-headers".
])])

AC_CHECK_HEADERS([GL/glx.h])

AC_ARG_ENABLE([input], [AS_HELP_STRING([--enable-input], [Enable mouse- and keyboard debugging/development. @<:@default=disabled@:>@])])
AS_IF([test "x$enable_input" = "xyes"],
	[AC_DEFINE([ENABLE_INPUT],  [1], [Define to 1 to enable input])],
  [enable_input=no])
AM_CONDITIONAL(INPUT_ENABLED, test "x$enable_input" = "xyes")

AC_ARG_ENABLE([editor], [AS_HELP_STRING([--enable-editor], [Build editor. @<:@default=disabled@:>@])])
AS_IF([test "x$enable_editor" = "xyes"],
   [PKG_CHECK_MODULES([gtk3], [gtk+-3.0 gtkglext-3.0])],
   [enable_editor=no])
AM_CONDITIONAL([BUILD_EDITOR], [test "x$enable_editor" = "xyes"])

dnl Setup paths
AC_ARG_VAR([DATA_PATH], [Data path prefix. Default is relative path to top srcdir])
AS_IF([test "x${DATA_PATH}" == "x"], [DATA_PATH="${srcdir}"])
AC_DEFINE_UNQUOTED([PATH_BASE],    ["${DATA_PATH}/"],             [Base path])
AC_DEFINE_UNQUOTED([PATH_MODELS],  ["${DATA_PATH}/models/"],      [Path to models])
AC_DEFINE_UNQUOTED([PATH_SRC],     ["${DATA_PATH}/src/"],         [Path to sources])
AC_DEFINE_UNQUOTED([PATH_SHADERS], ["${DATA_PATH}/shaders/"],     [Path to shaders])
AC_DEFINE_UNQUOTED([PATH_OPENCL],  ["${DATA_PATH}/cl_programs/"], [Path to OpenCL programs])
AC_DEFINE_UNQUOTED([PATH_ICON],    ["${DATA_PATH}/icons/"],       [Path to editor icons])
AC_DEFINE_UNQUOTED([PATH_MUSIC],   ["${DATA_PATH}/music/"],       [Path to music])

dnl Defaults
AC_ARG_VAR([FULLSCREEN], [If non-empty fullscreen is used by default])
AS_IF([test "x${FULLSCREEN}" == "x" -o "x${FULLSCREEN}" == "x0"], [FULLSCREEN=0], [FULLSCREEN=1])
AC_DEFINE_UNQUOTED([FULLSCREEN], [$FULLSCREEN], [Define to 1 if fullscreen is used by default])

AM_PATH_CPPUNIT(1.9.6,,[AC_MSG_NOTICE([cppunit not found, tests disabled])])
AM_CONDITIONAL([BUILD_TESTS], [test "x$no_cppunit" != "xyes"])

dnl Show config
echo
echo "         Data path: $DATA_PATH"
echo "    Editor enabled: $enable_editor"
echo "     Input enabled: $enable_input"
echo "          CXXFLAGS: $CXXFLAGS"
echo "           LDFLAGS: $LDFLAGS"
echo

AC_OUTPUT
