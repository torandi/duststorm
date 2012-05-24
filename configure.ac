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
AX_CHECK_GLU
AX_CHECK_CL
AX_GLEW
AX_GLM
AM_PATH_SDL

AS_IF([test x"${srcdir}" != x"."], [
	test -e shaders     || ln -s "${srcdir}"/shaders shaders
	test -e models      || ln -s "${srcdir}"/models  models
	test -e cl_programs || ln -s "${srcdir}"/cl_programs cl_programs
])
AM_CONDITIONAL([HAVE_SUBDIR], [test x"${srcdir}" != x"."])

AC_OUTPUT
