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
	src/scene/tv.cpp

if HAVE_SUBDIR
distclean-local:
	rm -f shaders models cl_programs
endif
