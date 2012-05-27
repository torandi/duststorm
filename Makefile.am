ACLOCAL_AMFLAGS = -I m4
AM_CXXFLAGS = -Wall -I${top_srcdir}/src ${ASSIMP_CFLAGS} ${GL_CFLAGS} ${GLU_CFLAGS} ${GLEW_CFLAGS} ${GLM_CFLAGS} ${SDL_CFLAGS} ${CL_CFLAGS}

noinst_LIBRARIES = libfrob.a
bin_PROGRAMS = demo

if BUILD_EDITOR
bin_PROGRAMS += editor
endif

libfrob_a_SOURCES = \
	src/bindable.hpp \
	src/camera.cpp src/camera.hpp \
	src/cl.cpp src/cl.hpp \
	src/color.cpp src/color.hpp \
	src/engine.cpp src/engine.hpp \
	src/globals.cpp src/globals.hpp \
	src/light.cpp src/light.hpp \
	src/lights_data.cpp src/lights_data.hpp \
	src/material.cpp src/material.hpp \
	src/mesh.cpp src/mesh.hpp \
	src/movable_light.cpp src/movable_light.hpp \
	src/movable_object.cpp src/movable_object.hpp \
	src/particle_system.cpp src/particle_system.hpp \
	src/rendertarget.cpp src/rendertarget.hpp \
	src/render_object.cpp src/render_object.hpp \
	src/scene.cpp src/scenefactory.cpp src/scene.hpp \
	src/shader.cpp src/shader.hpp \
	src/skybox.cpp src/skybox.hpp \
	src/texture.cpp src/texture.hpp \
	src/time.cpp src/time.hpp \
	src/timetable.cpp src/timetable.hpp \
	src/utils.cpp src/utils.hpp \
	src/quad.cpp src/quad.hpp \
	src/scene/particles.cpp \
	src/scene/test.cpp \
	src/scene/tv.cpp \
	src/scene/water.cpp \
	autoload.cpp autoload.h


if INPUT_ENABLED
libfrob_a_SOURCES += src/input.cpp src/input.hpp
endif

demo_LDADD = libfrob.a ${ASSIMP_LIBS} ${GL_LIBS} ${GLU_LIBS} ${GLEW_LIBS} ${SDL_LIBS} ${CL_LIBS} -lSDL_image
demo_SOURCES = src/main.cpp

editor_CXXFLAGS = ${AM_CXXFLAGS} $$(pkg-config gtk+-3.0 gtkglext-3.0 --cflags)
editor_LDFLAGS = $$(pkg-config gtk+-3.0 gtkglext-3.0 --libs) -Wl,--export-dynamic
editor_LDADD = libfrob.a ${ASSIMP_LIBS} ${GL_LIBS} ${GLU_LIBS} ${GLEW_LIBS} ${CL_LIBS}
editor_SOURCES = src/editor.cpp

.PHONY: .autoload.tmp
BUILT_SOURCES = .autoload.tmp autoload.h autoload.cpp
CLEANFILES = .autoload.tmp autoload.lst autoload.cpp autoload.h stamp-autoload

.autoload.tmp: Makefile
	@grep "^[[:blank:]]*REGISTER_SCENE_TYPE" -r ${top_srcdir}/src/scene/*.cpp | sed 's/.*(\(.*\),[ \t]*\"\(.*\)\"[ \t]*,.*).*/_register_\1:\2/g' > .autoload.tmp
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
