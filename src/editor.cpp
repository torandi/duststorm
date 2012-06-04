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

enum COL {
	COL_ICON = 0,
	COL_TITLE,
	COL_FILENAME,
	COL_TYPE,
};

static const unsigned int framerate = 60;
static const uint64_t per_frame = 1000000 / framerate;

namespace Editor {
	MODE mode = MODE_BLANK;
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
GtkToggleButton* playbutton = nullptr;
GtkListStore* propstore = nullptr;
RenderTarget* frame = nullptr;

/* icons */
static GdkPixbuf* icon_cat_scene = nullptr;
static GdkPixbuf* icon_scene     = nullptr;
static GdkPixbuf* icon_model     = nullptr;
static GdkPixbuf* icon_path      = nullptr;
static GdkPixbuf* icon_light     = nullptr;

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

extern "C" G_MODULE_EXPORT void scenelist_row_activated_cb(GtkTreeView* tree_view, GtkTreePath* path, GtkTreeViewColumn* column, gpointer user_data){
	//gint depth;
	//gint* tree = gtk_tree_path_get_indices_with_depth(path, &depth);

	GtkTreeIter iter;
	GtkTreeModel* treemodel = gtk_tree_view_get_model(tree_view);
	if ( !gtk_tree_model_get_iter(treemodel, &iter, path) ){
		return;
	}

	gchar* name;
	gchar* filename;
	Editor::TYPE type;
	gtk_tree_model_get(treemodel, &iter, COL_TITLE, &name, COL_TYPE, &type, COL_FILENAME, &filename, -1);

	switch ( type ){
	case Editor::TYPE_CAT:
		if ( gtk_tree_view_row_expanded(tree_view, path) ){
			gtk_tree_view_collapse_row(tree_view, path);
		} else {
			gtk_tree_view_expand_row(tree_view, path, FALSE);
		}
		break;

	case Editor::TYPE_SCENE:
		Editor::mode = Editor::MODE_SCENE;
		delete scene;
		Editor::scene_name = name;
		scene = SceneFactory::create(std::string(name), frame->texture_size());
		assert(scene);
		scene->add_time(0,60);
		global_time.reset();

		{
			GtkTreeIter iter;
			gtk_list_store_clear(propstore);
			SceneFactory::Metadata* meta = SceneFactory::find(name)->second.meta;
			for ( std::pair<std::string, std::string> p: *meta ){
				const std::string& key   = p.first;
				const std::string& value = p.second;
				gtk_list_store_append(propstore, &iter);
				gtk_list_store_set(propstore, &iter,
				                   0, key.c_str(),
				                   1, value.c_str(),
				                   -1);

			}
		}


		break;

	case Editor::TYPE_PATH:
		break;

	case Editor::TYPE_MODEL:
		Editor::mode = Editor::MODE_MODEL;
		Editor::reset();
		Editor::load_model(filename);
		break;

	default:
		printf("type: %d\n", type);
		break;
	}

	g_free(name);
	g_free(filename);
}

void update(){
	global_time.update();

	if ( scene ){
		scene->update_scene(global_time.get(), global_time.dt());
	}

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
	playbutton         = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "play"));
	propstore          = GTK_LIST_STORE(gtk_builder_get_object(builder, "propstore"));

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
	icon_scene     = gdk_pixbuf_new_from_file(PATH_ICON "picture.png", NULL);
	icon_model     = gdk_pixbuf_new_from_file(PATH_ICON "brick.png", NULL);
	icon_path      = gdk_pixbuf_new_from_file(PATH_ICON "map.png", NULL);
	icon_light     = gdk_pixbuf_new_from_file(PATH_ICON "lightbulb.png", NULL);

	/* setup scene-list */
	GtkTreeIter toplevel;
	GtkTreeStore* scenestore = GTK_TREE_STORE(gtk_builder_get_object(builder, "scenestore"));
	gtk_tree_store_append(scenestore, &toplevel, NULL);
	gtk_tree_store_set(scenestore, &toplevel,
	                   COL_ICON, icon_cat_scene,
	                   COL_TITLE, "<b>Scenes</b>",
	                   COL_TYPE, Editor::TYPE_CAT,
	                   -1);

	for ( auto it = SceneFactory::begin(); it != SceneFactory::end(); ++it ){
		GtkTreeIter child;
		gtk_tree_store_append(scenestore, &child, &toplevel);
		gtk_tree_store_set(scenestore, &child,
		                   COL_ICON, icon_scene,
		                   COL_TITLE, it->first.c_str(),
		                   COL_TYPE, Editor::TYPE_SCENE,
		                   -1);

		SceneFactory::Metadata* meta = it->second.meta;
		for ( std::pair<std::string, std::string> p: *meta ){
			const std::string& key   = p.first;
			const std::string& value = p.second;
			std::string filename = "";
			GdkPixbuf* icon = nullptr;
			gint type = Editor::TYPE_UNKNOWN;

			char* data = strdup(value.c_str());
			char* t = strtok(data, ":");
			char* d = strtok(NULL, ":");

			if ( strcmp(t, "path") == 0 ){
				type = Editor::TYPE_PATH;
				icon = icon_path;
				filename = std::string(d?d:"<nil>");
			} else if ( strcmp(t, "model") == 0 ){
				type = Editor::TYPE_MODEL;
				icon = icon_model;
				filename = std::string(d?d:"<nil>");
			} else if ( strcmp(t, "light") == 0 ){
				type = Editor::TYPE_LIGHT;
				icon = icon_light;
				filename = std::string(d?d:"<nil>");
			}

			free(data);

			GtkTreeIter cur;
			gtk_tree_store_append(scenestore, &cur, &child);
			gtk_tree_store_set(scenestore, &cur,
			                   COL_ICON, icon,
			                   COL_TITLE, key.c_str(),
			                   COL_FILENAME, filename.c_str(),
			                   COL_TYPE, type,
			                   -1);
		}
	}

	gtk_tree_store_append(scenestore, &toplevel, NULL);
	gtk_tree_store_set(scenestore, &toplevel, COL_TITLE, "<b>Compositions</b>", -1);

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
