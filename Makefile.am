ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -Wall

bin_PROGRAMS = demo

demo_CXXFLAGS = ${ASSIMP_CFLAGS} ${GL_CFLAGS} ${GLU_CFLAGS} ${GLEW_CFLAGS} ${GLM_CFLAGS} ${SDL_CFLAGS}
demo_LDADD = ${ASSIMP_LIBS} ${GL_LIBS} ${GLU_LIBS} ${GLEW_LIBS} ${SDL_LIBS} -lOpenCL
demo_SOURCES = \
	src/main.cpp \
	src/color.cpp src/color.hpp \
	src/globals.cpp src/globals.hpp \
	src/rendertarget.cpp src/rendertarget.hpp \
	src/shader.cpp src/shader.hpp \
	src/movable_object.cpp src/movable_object.hpp \
	src/camera.cpp src/camera.hpp \
	src/light.cpp src/light.hpp \
	src/utils.cpp src/utils.hpp \
	src/render_object.cpp src/render_object.hpp \
	src/scene.cpp src/scene.hpp \
	src/time.cpp src/time.hpp \
	src/cl.cpp src/cl.hpp

if HAVE_SUBDIR
distclean-local:
	rm -f shaders models
endif
