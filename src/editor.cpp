#include <cstdio>
#include <cstdlib>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkgl.h>
#include <gtk/gtkgl.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "color.hpp"
#include "engine.hpp"
#include "globals.hpp"
#include "rendertarget.hpp"
#include "time.hpp"

static const unsigned int framerate = 60;
static const uint64_t per_frame = 1000000 / framerate;
Time global_time(per_frame);

static bool initialized = false;
static float aspect = 16.0f / 9.0f;
static glm::mat4 projection;
static GtkWidget* drawing = nullptr;
static RenderTarget* frame = nullptr;

extern "C" G_MODULE_EXPORT void aspect_changed(GtkWidget* widget, gpointer data){
	if ( !gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(widget)) ) return;

	int w,h;
	const gchar* label = gtk_menu_item_get_label(GTK_MENU_ITEM(widget));
	sscanf(label, "%d:%d", &w, &h);
	aspect = (float)w / (float)h;

	gtk_widget_queue_resize(drawing);
}

extern "C" G_MODULE_EXPORT gboolean drawingarea_draw_cb(GtkWidget* widget, gpointer data){
	if ( !initialized ) return FALSE;
	if (!gtk_widget_begin_gl (widget)) return FALSE;

	frame->bind();
	shaders[SHADER_NORMAL]->bind();
	frame->clear(Color::magenta);
	shaders[SHADER_NORMAL]->unbind();
	frame->unbind();

	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


	const glm::ivec2 center = (resolution-frame->size) / 2;
	Shader::upload_state(resolution);
	Shader::upload_projection_view_matrices(projection, glm::mat4());
	glViewport(0, 0, resolution.x, resolution.y);
	shaders[SHADER_PASSTHRU]->bind();
	frame->draw(center);
	shaders[SHADER_PASSTHRU]->unbind();

	gtk_widget_end_gl(widget, TRUE);
	return TRUE;
}

extern "C" G_MODULE_EXPORT gboolean drawingarea_configure_event_cb(GtkWidget* widget, gpointer data){
	if ( !initialized ) return FALSE;
  if ( !gtk_widget_begin_gl (widget) ) return FALSE;

  GtkAllocation allocation;
  gtk_widget_get_allocation (widget, &allocation);

  resolution.x = allocation.width;
  resolution.y = allocation.height;

	projection = glm::ortho(0.0f, (float)resolution.x, 0.0f, (float)resolution.y, -1.0f, 1.0f);
	projection = glm::scale(projection, glm::vec3(1.0f, -1.0f, 1.0f));
	projection = glm::translate(projection, glm::vec3(0.0f, -(float)resolution.y, 0.0f));


  /* fit frame into current resolution while preserving aspect */
  const float a = (float)resolution.x / (float)resolution.y;
  glm::ivec2 size = resolution;
  if ( a < aspect ){
	  size.y = resolution.x / aspect;
  } else {
	  size.x = resolution.y * aspect;
  }

	delete frame;
	frame = new RenderTarget(size, false, true, GL_NEAREST);

  gtk_widget_end_gl (widget, FALSE);
  return TRUE;
}

extern "C" G_MODULE_EXPORT void drawingarea_realize_cb(GtkWidget* widget, gpointer data){
  if (!gtk_widget_begin_gl (widget)) return;

	int ret;
	if ( (ret=glewInit()) != GLEW_OK ){
		fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(ret));
		exit(1);
	}

	Engine::setup_opengl();
	Engine::load_shaders();

  gtk_widget_end_gl(widget, FALSE);
  gtk_widget_queue_resize(widget);
  initialized = true;
}

int main (int argc, char* argv[]){
	gtk_init (&argc, &argv);
	gdk_gl_init(&argc, &argv);
	gtk_gl_init(&argc, &argv);

	verbose = fopen("/dev/null", "w");

	GtkBuilder* builder = gtk_builder_new();
	gtk_builder_add_from_file (builder, "editor.xml", NULL);

	GtkWidget* window  = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	drawing            = GTK_WIDGET(gtk_builder_get_object(builder, "drawingarea"));

	/* enable opengl on drawingarea */
	int attrib[] = {
		GDK_GL_RGBA,
		GDK_GL_DOUBLEBUFFER,
		GDK_GL_ATTRIB_LIST_NONE,
	};
	GdkGLConfig* config = gdk_gl_config_new(attrib);
	gtk_widget_set_gl_capability(drawing, config, NULL, TRUE, GDK_GL_RGBA_TYPE);

	gtk_builder_connect_signals (builder, NULL);
	g_object_unref(G_OBJECT(builder));

	// ctrl+q shortcut
	guint key;
	GdkModifierType mods;
	GtkAccelGroup* accel_group = gtk_accel_group_new();
	gtk_accelerator_parse("<Ctrl>q", &key, &mods);
	gtk_accel_map_add_entry("<editor>/quit", key, mods);
	gtk_accel_group_connect_by_path(accel_group, "<editor>/quit", g_cclosure_new(gtk_main_quit, NULL, NULL));
	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

	gtk_widget_show(window);
	gtk_main();

	return 0;
}
