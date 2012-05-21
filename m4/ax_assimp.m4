AC_DEFUN([AX_ASSIMP], [
	saved_CPPFLAGS="$CPPFLAGS"
	saved_LDFLAGS="$LDFLAGS"
	
	case "x$1" in
			xyes | "x")
					ax_assimp_path=
					ax_assimp_want="yes"
					;;
			xno)
					ax_assimp_want="no"
					;;
			*)
					ax_assimp_path="$1"
					ax_assimp_want="yes"
					CPPFLAGS="$CPPFLAGS -I$ax_assimp_path/include"
					LDFLAGS="$LDFLAGS -I$ax_assimp_path/lib"
					;;
	esac
	
	AS_IF([test "x${ax_assimp_want}" == "xyes"], [
					AC_CHECK_HEADER([assimp/assimp.h],[
									AC_CHECK_LIB([assimp], [aiImportFile], [
													ax_assimp_cflags=
													ax_assimp_libs=-lassimp
													
													AS_IF([test "x$ax_assimp_path" != "x"], [
																	ax_assimp_cflags="-I$ax_assimp_path/include"
																	ax_assimp_libs="-L$ax_assimp_path/lib $ax_assimp_libs"
													])
													
													AC_SUBST(ASSIMP_CFLAGS, [$ax_assimp_cflags])
													AC_SUBST(ASSIMP_LIBS, [$ax_assimp_libs])
													AC_DEFINE([HAVE_ASSIMP], 1, [Define to 1 if you have assimp])
									], [
													AC_MSG_ERROR([Make sure assimp (libassim-dev) is installed.])
													])
									], [
													AC_MSG_ERROR([Make sure assimp (libassim-dev) is installed.])
									])
					]) dnl if ${ax_assimp_want}
	
	CPPFLAGS="$saved_CPPFLAGS"
	LDFLAGS="$saved_LDFLAGS"
])
