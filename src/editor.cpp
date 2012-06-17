#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "editor/editor.hpp"
#include <cstdio>
#include <cstdlib>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkgl.h>
#include <gtk/gtkgl.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "camera.hpp"
#include "color.hpp"
#include "engine.hpp"
#include "globals.hpp"
#include "rendertarget.hpp"
#include "render_object.hpp"
#include "scene.hpp"
#include "time.hpp"

static const unsigned int framerate = 60;
static const uint64_t per_frame = 1000000 / framerate;
void drawingarea_update(); /* hack */

namespace Editor {
	MODE mode = MODE_BLANK;
	CAMERA_MODE camera_control = CAMERA_AUTO;
	unsigned int frames = 0;
	std::string scene_name;
}

/* globals */
bool running = false;
Time global_time(per_frame);
Scene* scene = nullptr;

/* editor widgets */
GtkWidget* drawing = nullptr;
GtkLabel* timelabel = nullptr;
GtkLabel* message = nullptr;
GtkToggleButton* playbutton = nullptr;
GtkTreeStore* scenestore = nullptr;
GtkTreeStore* propstore = nullptr;
RenderTarget* frame = nullptr;

/* icons */
GdkPixbuf* icon_cat_scene = nullptr;
GdkPixbuf* icon_cat_model = nullptr;
GdkPixbuf* icon_scene     = nullptr;
GdkPixbuf* icon_model     = nullptr;
GdkPixbuf* icon_path      = nullptr;
GdkPixbuf* icon_light     = nullptr;

static void show_fps(int signum){
	//fprintf(stderr, "FPS: %d\n", frames);
	Editor::frames = 0;
}

namespace Engine {
	RenderTarget* rendertarget_by_name(const std::string& fullname){
		/* not implemented */
		return nullptr;
	}
}

void update(){
	global_time.update();

	if ( scene ){
		scene->update_scene(global_time.get(), global_time.dt());
	}

	drawingarea_update();

	char buf[64];
	sprintf(buf, "%02.3f\n%d%%", global_time.get(), global_time.current_scale());
	gtk_label_set_text(timelabel, buf);
}

int main (int argc, char* argv[]){
	gtk_init (&argc, &argv);
	gdk_gl_init(&argc, &argv);
	gtk_gl_init(&argc, &argv);

	/* init engine */
	verbose = fopen("/dev/null", "w");
	Engine::autoload_scenes();

	GError* error = nullptr;
	GtkBuilder* builder = gtk_builder_new();
	if ( !gtk_builder_add_from_file (builder, PATH_SRC "editor.xml", &error) ){
		g_print( "Error occured while loading UI file!\n" );
		g_print( "Message: %s\n", error->message );
		g_free(error);
		exit(1);
	}

	GtkWidget* window  = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	drawing            = GTK_WIDGET(gtk_builder_get_object(builder, "drawingarea"));
	timelabel          = GTK_LABEL(gtk_builder_get_object(builder, "timelabel"));
	message            = GTK_LABEL(gtk_builder_get_object(builder, "message"));
	playbutton         = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "play"));
	scenestore         = GTK_TREE_STORE(gtk_builder_get_object(builder, "scenestore"));
	propstore          = GTK_TREE_STORE(gtk_builder_get_object(builder, "propstore"));

	/* style message label */
	GdkColor color;
	gdk_color_parse ("#000000", &color);
	gtk_widget_modify_bg(GTK_WIDGET(gtk_builder_get_object(builder, "messagebox")), GTK_STATE_NORMAL, &color);
	gdk_color_parse ("#ffff00", &color);
	gtk_widget_modify_fg(GTK_WIDGET(message), GTK_STATE_NORMAL, &color);
	gtk_label_set_text(message, "");

	/* enable opengl on drawingarea */
	int attrib[] = {
		GDK_GL_RGBA,
		GDK_GL_DOUBLEBUFFER,
		GDK_GL_ATTRIB_LIST_NONE,
	};
	GdkGLConfig* config = gdk_gl_config_new(attrib);
	gtk_widget_set_gl_capability(drawing, config, NULL, TRUE, GDK_GL_RGBA_TYPE);

	// ctrl+q shortcut
	guint key;
	GdkModifierType mods;
	GtkAccelGroup* accel_group = gtk_accel_group_new();
	gtk_accelerator_parse("<Ctrl>q", &key, &mods);
	gtk_accel_map_add_entry("<editor>/quit", key, mods);
	gtk_accel_group_connect_by_path(accel_group, "<editor>/quit", g_cclosure_new(gtk_main_quit, NULL, NULL));
	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

	/* load icons */
	icon_cat_scene = gdk_pixbuf_new_from_file(PATH_ICON "pictures.png", NULL);
	icon_cat_model = gdk_pixbuf_new_from_file(PATH_ICON "bricks.png", NULL);
	icon_scene     = gdk_pixbuf_new_from_file(PATH_ICON "picture.png", NULL);
	icon_model     = gdk_pixbuf_new_from_file(PATH_ICON "brick.png", NULL);
	icon_path      = gdk_pixbuf_new_from_file(PATH_ICON "map.png", NULL);
	icon_light     = gdk_pixbuf_new_from_file(PATH_ICON "lightbulb.png", NULL);

	/* setup scene-list */
	Editor::scenelist_populate();

	g_timeout_add(per_frame/1000, [](gpointer data) -> gboolean {
		update();
		gtk_widget_queue_draw(drawing);
		return TRUE;
	}, NULL);

	/* setup FPS alarm handler */
	{
		struct itimerval difftime;
		difftime.it_interval.tv_sec = 1;
		difftime.it_interval.tv_usec = 0;
		difftime.it_value.tv_sec = 1;
		difftime.it_value.tv_usec = 0;
		signal(SIGALRM, show_fps);
		setitimer(ITIMER_REAL, &difftime, NULL);
	}

	gtk_builder_connect_signals (builder, NULL);
	g_object_unref(G_OBJECT(builder));

  gtk_widget_show(window);
	gtk_main();

	return 0;
}
