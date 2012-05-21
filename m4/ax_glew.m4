AC_DEFUN([AX_GLEW], [
	saved_CPPFLAGS="$CPPFLAGS"
	saved_LDFLAGS="$LDFLAGS"
	
	case "x$1" in
			xyes | "x")
					ax_glew_path=
					ax_glew_want="yes"
					;;
			xno)
					ax_glew_want="no"
					;;
			*)
					ax_glew_path="$1"
					ax_glew_want="yes"
					CPPFLAGS="$CPPFLAGS -I$ax_glew_path/include"
					LDFLAGS="$LDFLAGS -I$ax_glew_path/lib"
					;;
	esac
	
	AS_IF([test "x${ax_glew_want}" == "xyes"], [
					AC_CHECK_HEADER([GL/glew.h],[
									AC_CHECK_LIB([GLEW], [glewInit], [
													ax_glew_cflags=
													ax_glew_libs=-lGLEW
													
													AS_IF([test "x$ax_glew_path" != "x"], [
																	ax_glew_cflags="-I$ax_glew_path/include"
																	ax_glew_libs="-L$ax_glew_path/lib $ax_glew_libs"
													])
													
													AC_SUBST(GLEW_CFLAGS, [$ax_glew_cflags])
													AC_SUBST(GLEW_LIBS, [$ax_glew_libs])
													AC_DEFINE([HAVE_GLEW], 1, [Define to 1 if you have assimp])
													], [
													AC_MSG_ERROR([Make sure GLEW is installed.])
													])
									], [
													AC_MSG_ERROR([Make sure GLEW is installed.])
									])
					]) dnl if ${ax_glew_want}
	
	CPPFLAGS="$saved_CPPFLAGS"
	LDFLAGS="$saved_LDFLAGS"
])
