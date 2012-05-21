AC_DEFUN([AX_GLM], [
	saved_CPPFLAGS="$CPPFLAGS"
	saved_LDFLAGS="$LDFLAGS"
	
	case "x$1" in
			xyes | "x")
					ax_glm_path=
					ax_glm_want="yes"
					;;
			xno)
					ax_glm_want="no"
					;;
			*)
					ax_glm_path="$1"
					ax_glm_want="yes"
					CPPFLAGS="$CPPFLAGS -I$ax_glm_path/include"
					;;
	esac
	
	AS_IF([test "x${ax_glm_want}" == "xyes"], [
					AC_LANG_PUSH([C++])
					AC_CHECK_HEADER([glm/glm.hpp],[
									ax_glm_cflags=
													
									AS_IF([test "x$ax_glm_path" != "x"], [
													ax_glm_cflags="-I$ax_glm_path/include"
													])
									
									AC_SUBST(GLM_CFLAGS, [$ax_glm_cflags])
									AC_DEFINE([HAVE_GLM], 1, [Define to 1 if you have glm])
					], [
									AC_MSG_ERROR([Make sure glm (libglm-dev) is installed.])
					])
					AC_LANG_POP
	]) dnl if ${ax_glm_want}

	CPPFLAGS="$saved_CPPFLAGS"
	LDFLAGS="$saved_LDFLAGS"
])
