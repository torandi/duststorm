#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkgl.h>
#include <gtk/gtkgl.h>
#include <GL/glew.h>

extern "C" G_MODULE_EXPORT gboolean drawingarea_draw_cb(GtkWidget* widget, gpointer data){
	if (!gtk_widget_begin_gl (widget)) return FALSE;

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	gtk_widget_end_gl(widget, TRUE);
	return TRUE;
}

extern "C" G_MODULE_EXPORT gboolean drawingarea_configure_event_cb(GtkWidget* widget, gpointer data){
  if (!gtk_widget_begin_gl (widget)) return FALSE;

  GtkAllocation allocation;
  gtk_widget_get_allocation (widget, &allocation);

  glViewport (0, 0, allocation.width, allocation.height);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();

  gtk_widget_end_gl (widget, FALSE);
  return TRUE;
}

extern "C" G_MODULE_EXPORT void drawingarea_realize_cb(GtkWidget* widget, gpointer data){
  if (!gtk_widget_begin_gl (widget)) return;

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  gtk_widget_end_gl (widget, FALSE);
}

int main (int argc, char* argv[]){
	gtk_init (&argc, &argv);
	gdk_gl_init(&argc, &argv);
	gtk_gl_init(&argc, &argv);

	GtkBuilder* builder = gtk_builder_new();
	gtk_builder_add_from_file (builder, "editor.xml", NULL);

	GtkWidget* window  = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	GtkWidget* drawing = GTK_WIDGET(gtk_builder_get_object(builder, "drawingarea"));

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
