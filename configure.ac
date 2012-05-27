AC_INIT([frobnicator],[1],[ext@sidvind.com])

AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([1.11 foreign color-tests subdir-objects -Wall -Werror])
AM_SILENT_RULES([yes])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

AC_GNU_SOURCE
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CXX
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

AC_DEFINE_UNQUOTED([PATH_MODELS],  ["${srcdir}/models/"],      [Path to models])
AC_DEFINE_UNQUOTED([PATH_SRC],     ["${srcdir}/src/"],         [Path to sources])
AC_DEFINE_UNQUOTED([PATH_SHADERS], ["${srcdir}/shaders/"],     [Path to shaders])
AC_DEFINE_UNQUOTED([PATH_CL],      ["${srcdir}/cl_programs/"], [Path to OpenCL programs])

AS_IF([test x"${srcdir}" != x"."], [
	test -e shaders     || ln -s "${srcdir}"/shaders shaders
	test -e models      || ln -s "${srcdir}"/models  models
	test -e cl_programs || ln -s "${srcdir}"/cl_programs cl_programs
])
AM_CONDITIONAL([HAVE_SUBDIR], [test x"${srcdir}" != x"."])

AC_OUTPUT
