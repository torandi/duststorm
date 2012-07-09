ACLOCAL_AMFLAGS = -I m4

engine_CXXFLAGS = ${GL_CFLAGS} ${GLU_CFLAGS} ${GLEW_CFLAGS} ${GLM_CFLAGS} ${SDL_CFLAGS} ${CL_CFLAGS}
engine_LIBS = ${VENDOR_LIBS} ${GL_LIBS} ${GLU_LIBS} ${GLEW_LIBS} ${SDL_LIBS} ${CL_LIBS} -lSDL_image -lfmodex -lassimp
AM_CXXFLAGS = -Wall -I${top_srcdir}/src ${engine_CXXFLAGS} ${VENDOR_CFLAGS}

noinst_LIBRARIES = libfrob.a
bin_PROGRAMS = demo nox2
TESTS =

if BUILD_EDITOR
bin_PROGRAMS += editor
TESTS += test/editor_classify
endif

if BUILD_TESTS
check_PROGRAMS = ${TESTS}
endif

libfrob_a_SOURCES = \
	src/bindable.hpp \
	src/camera.cpp src/camera.hpp \
	src/cl.cpp src/cl.hpp \
	src/color.cpp src/color.hpp \
	src/data.cpp src/data.hpp \
	src/engine.cpp src/engine.hpp \
	src/globals.cpp src/globals.hpp \
	src/light.cpp src/light.hpp \
	src/lights_data.cpp src/lights_data.hpp \
	src/material.cpp src/material.hpp \
	src/mesh.cpp src/mesh.hpp \
	src/meta.cpp src/meta.hpp \
	src/movable_light.cpp src/movable_light.hpp \
	src/movable_object.cpp src/movable_object.hpp \
	src/music.cpp src/music.hpp \
	src/particle_system.cpp src/particle_system.hpp \
	src/rendertarget.cpp src/rendertarget.hpp \
	src/render_object.cpp src/render_object.hpp \
	src/scene.cpp src/scenefactory.cpp src/scene.hpp \
	src/shader.cpp src/shader.hpp \
	src/skybox.cpp src/skybox.hpp \
	src/terrain.cpp src/terrain.hpp \
	src/texture.cpp src/texture.hpp \
	src/time.cpp src/time.hpp \
	src/timetable.cpp src/timetable.hpp \
	src/utils.cpp src/utils.hpp \
	src/quad.cpp src/quad.hpp \
	src/scene/nox.cpp \
	src/scene/particles.cpp \
	src/scene/test.cpp \
	src/scene/tunnel.cpp \
	src/scene/tv.cpp \
	src/scene/water.cpp \
	autoload.cpp autoload.h

if INPUT_ENABLED
libfrob_a_SOURCES += src/input.cpp src/input.hpp
endif

demo_CXXFLAGS = ${AM_CXXFLAGS} -DNAME="\"demo\"" -DTITLE="\"Experimental demo\""
demo_LDADD = libfrob.a ${engine_LIBS}
demo_SOURCES = src/main.cpp src/testing.cpp
demo_LDFLAGS = -pthread

nox2_CXXFLAGS = ${AM_CXXFLAGS} -DNAME="\"nox2\"" -DTITLE="\"NördtroXy II Invitation\""
nox2_LDADD = libfrob.a ${engine_LIBS}
nox2_LDFLAGS = -pthread
nox2_SOURCES = src/main.cpp src/nox2.cpp

editor_CXXFLAGS = ${AM_CXXFLAGS} $(gtk3_CFLAGS)
editor_LDFLAGS = -Wl,--export-dynamic -pthread
editor_LDADD = libfrob.a ${engine_LIBS} $(gtk3_LIBS)
editor_SOURCES = \
	src/editor.cpp \
	src/editor/classify.cpp \
	src/editor/render.cpp \
	src/editor/scenelist.cpp \
	src/editor/timecontrol.cpp

test_editor_classify_CXXFLAGS = ${AM_CXXFLAGS} $(CPPUNIT_CFLAGS) $(gtk3_CFLAGS)
test_editor_classify_LDFLAGS = $(CPPUNIT_LIBS)
test_editor_classify_SOURCES = test/editor_classify.cpp src/editor/classify.cpp

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

release: all
	@test "x${prefix}" = "x/" || (echo "Error: --prefix must be / when creating release (currently ${prefix})"; exit 1)
	mkdir -p release-dist
	$(MAKE) install DESTDIR=`pwd`/release-dist
