AC_INIT([frobnicator-demo-engine],[1],[torandi@gmail.com])

AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign subdir-objects -Wall -Werror])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AH_BOTTOM([#include "forward.hpp"])

AC_GNU_SOURCE
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CXX
AC_PROG_RANLIB
AX_CHECK_COMPILE_FLAG([-std=c++0x], [CXXFLAGS="$CXXFLAGS -std=c++0x"])

AX_CHECK_GL
AX_CHECK_GLU
AX_GLEW
AX_GLM
AX_CHECK_CL
AM_PATH_SDL

AS_IF([test "x$no_gl" == "xyes"], [AC_MSG_ERROR([OpenGL libraries required])])
AS_IF([test "x$no_cl" == "xyes"], [AC_MSG_ERROR([OpenCL libraries required])])

AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([GL/glx.h])

dnl Setup paths
AC_ARG_VAR([DATA_PATH], [Data path prefix. Default is relative path to top srcdir])
AS_IF([test "x${DATA_PATH}" == "x"], [DATA_PATH="${srcdir}"])
AC_DEFINE_UNQUOTED([PATH_BASE],    ["${DATA_PATH}/"],             [Base path])

dnl Defaults
AC_ARG_VAR([FULLSCREEN], [If non-empty fullscreen is used by default])
AS_IF([test "x${FULLSCREEN}" == "x" -o "x${FULLSCREEN}" == "x0"], [FULLSCREEN=0], [FULLSCREEN=1])
AC_DEFINE_UNQUOTED([FULLSCREEN], [$FULLSCREEN], [Define to 1 if fullscreen is used by default])

platform=$(uname -s)
arch=$(uname -m)
AS_IF([test "x${platform}" == "xDarwin"], [arch="osx"])

dnl Use vendor libs
VENDOR_CFLAGS=""
VENDOR_LIBS=""
AC_ARG_ENABLE([vendor-libs], [AS_HELP_STRING([--enable-vendor-libs], [Use libraries from vendor folder instead of system. @<:@default=enabled@:>@])], [], [enable_vendor_libs="yes"])
AS_IF([test "x$enable_vendor_libs" == "xyes"], [
	VENDOR_CFLAGS="-I \${top_srcdir}/vendor/include"
	VENDOR_LIBS="-L \${top_srcdir}/vendor/libs/$arch -Wl,--rpath \${top_srcdir}/vendor/libs/$arch"
])
AC_SUBST(VENDOR_CFLAGS)
AC_SUBST(VENDOR_LIBS)

dnl Show config
echo
echo "     Platform/arch: $platform $arch (vendor libs: $enable_vendor_libs)"
echo "         Data path: $DATA_PATH"
echo "    Editor enabled: $enable_editor"
echo "     Input enabled: $enable_input"
echo "     Loading scene: $loadingscene"
echo " Scene autoloading: $enable_autoload"
echo "          CXXFLAGS: $CXXFLAGS"
echo "           LDFLAGS: $LDFLAGS"
echo

AC_OUTPUT
