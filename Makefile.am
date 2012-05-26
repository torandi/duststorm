ACLOCAL_AMFLAGS = -I m4
AM_CXXFLAGS = -Wall -I${top_srcdir}/src

bin_PROGRAMS = demo

demo_CXXFLAGS = ${AM_CXXFLAGS} ${ASSIMP_CFLAGS} ${GL_CFLAGS} ${GLU_CFLAGS} ${GLEW_CFLAGS} ${GLM_CFLAGS} ${SDL_CFLAGS} ${CL_CFLAGS}
demo_LDADD = ${ASSIMP_LIBS} ${GL_LIBS} ${GLU_LIBS} ${GLEW_LIBS} ${SDL_LIBS} ${CL_LIBS} 
demo_SOURCES = \
	src/main.cpp \
	src/camera.cpp src/camera.hpp \
	src/cl.cpp src/cl.hpp \
	src/color.cpp src/color.hpp \
	src/engine.cpp src/engine.hpp \
	src/globals.cpp src/globals.hpp \
	src/light.cpp src/light.hpp \
	src/mesh.cpp src/mesh.hpp \
	src/movable_object.cpp src/movable_object.hpp \
	src/particle_system.cpp src/particle_system.hpp \
	src/rendertarget.cpp src/rendertarget.hpp \
	src/render_object.cpp src/render_object.hpp \
	src/scene.cpp src/scene.hpp \
	src/shader.cpp src/shader.hpp \
	src/time.cpp src/time.hpp \
	src/utils.cpp src/utils.hpp \
	src/quad.cpp src/quad.hpp \
	src/scene/tv.cpp \
	autoload.cpp autoload.h

if HAVE_SUBDIR
distclean-local:
	rm -f shaders models cl_programs
endif

.PHONY: .autoload.tmp
BUILT_SOURCES = .autoload.tmp autoload.h autoload.cpp
CLEANFILES = .autoload.tmp autoload.lst autoload.cpp autoload.h stamp-autoload

.autoload.tmp: Makefile
	@grep "^[[:blank:]]*REGISTER_SCENE_TYPE" -r ${top_srcdir}/src/scene/*.cpp | sed 's/.*(\(.*\),[ ]*\"\(.*\)\").*/_register_\1:\2/g' > .autoload.tmp
	@cmp -s autoload.lst .autoload.tmp || touch stamp-autoload

autoload.lst: stamp-autoload
	$(AM_V_GEN)cp .autoload.tmp autoload.lst

autoload.h: autoload.lst Makefile
	$(AM_V_GEN)sed 's/\(.*\):\(.*\)/void \1();/' autoload.lst > autoload.h

autoload.cpp: autoload.lst Makefile
	@echo '#include "autoload.h"' > autoload.cpp
	@echo '#include "globals.hpp"' >> autoload.cpp
	@echo '#include <cstdio>' >> autoload.cpp
	@echo 'namespace Engine { void autoload_scenes(){' >> autoload.cpp
	@echo ' 	fprintf(verbose, "Autoloading scenes.\n");' >> autoload.cpp
	$(AM_V_GEN)sed 's/\(.*\):\(.*\)/	fprintf(verbose, "  %s -> %s\\n", \"\2\", \"\1\"); \1();/' autoload.lst >> autoload.cpp
	@echo "}}" >> autoload.cpp
