ACLOCAL_AMFLAGS = -I m4

engine_CXXFLAGS = ${GL_CFLAGS} ${GLU_CFLAGS} ${GLEW_CFLAGS} ${GLM_CFLAGS} ${SDL_CFLAGS} ${CL_CFLAGS}
engine_LIBS = ${VENDOR_LIBS} ${GL_LIBS} ${GLU_LIBS} ${GLEW_LIBS} ${SDL_LIBS} ${CL_LIBS} -lSDL_image -lfmodex -lassimp 
AM_CXXFLAGS = -Wall -I${top_srcdir}/src ${engine_CXXFLAGS} ${VENDOR_CFLAGS}
AM_CFLAGS = -I${top_srcdir}/src ${engine_CXXFLAGS} ${VENDOR_CFLAGS}

bin_PROGRAMS = fubar

fubar_LDADD = ${engine_LIBS}
fubar_SOURCES =	src/main.cpp \
								src/bindable.hpp \
								src/camera.cpp src/camera.hpp \
								src/cl.cpp src/cl.hpp \
								src/config.cpp src/config.hpp \
								src/color.cpp src/color.hpp \
								src/data.cpp src/data.hpp \
								src/engine.cpp src/engine.hpp \
								src/enemy.cpp src/enemy.hpp \
								src/enemy_template.cpp src/enemy_template.hpp \
								src/game.cpp src/game.hpp \
								src/globals.cpp src/globals.hpp \
								src/hitting_particles.cpp src/hitting_particles.hpp \
								src/highscore.cpp src/highscore.hpp \
								src/input.cpp src/input.hpp \
								src/light.cpp src/light.hpp \
								src/lights_data.cpp src/lights_data.hpp \
								src/material.cpp src/material.hpp \
								src/mesh.cpp src/mesh.hpp \
								src/movable_object.cpp src/movable_object.hpp \
								src/movable_light.cpp src/movable_light.hpp \
								src/sound.cpp src/sound.hpp \
								src/particle_system.cpp src/particle_system.hpp \
								src/path.cpp src/path.hpp \
								src/player.cpp src/player.hpp \
								src/rails.cpp src/rails.hpp \
								src/rendertarget.cpp src/rendertarget.hpp \
								src/render_object.cpp src/render_object.hpp \
								src/shader.cpp src/shader.hpp \
								src/skybox.cpp src/skybox.hpp \
								src/terrain.cpp src/terrain.hpp \
								src/texture.cpp src/texture.hpp \
								src/text.cpp src/text.hpp \
								src/utils.cpp src/utils.hpp \
								src/quad.hpp src/quad.cpp \
								vendor/src/nanosvg.c
release: all
	@test "x${prefix}" = "x/" || (echo "Error: --prefix must be / when creating release (currently ${prefix})"; exit 1)
	mkdir -p release-dist
	$(MAKE) install DESTDIR=`pwd`/release-dist
