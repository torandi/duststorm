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
#include "scene.hpp"
#include "time.hpp"

static const unsigned int framerate = 60;
static const uint64_t per_frame = 1000000 / framerate;
static int frames = 0;
Time global_time(per_frame);

static bool initialized = false;
static bool running = false;
static float aspect = 16.0f / 9.0f;
static glm::mat4 projection;
static GtkWidget* drawing = nullptr;
static GtkLabel* timelabel = nullptr;
static GtkToggleButton* playbutton = nullptr;
static RenderTarget* frame = nullptr;
static std::string scene_name;
static Scene* scene = nullptr;
static float slide_ref;

static void show_fps(int signum){
	fprintf(stderr, "FPS: %d\n", frames);
	frames = 0;
}

static void render_placeholder(){
	frame->bind();
	shaders[SHADER_PASSTHRU]->bind();
	frame->clear(Color::magenta);
	shaders[SHADER_PASSTHRU]->unbind();
	frame->unbind();
}

static void render_scene(){
	scene->render_scene();

	frame->bind();
	shaders[SHADER_PASSTHRU]->bind();
	frame->clear(Color::magenta);
	Shader::upload_projection_view_matrices(frame->ortho(), glm::mat4());
	scene->draw();
	shaders[SHADER_PASSTHRU]->unbind();
	frame->unbind();
}

extern "C" G_MODULE_EXPORT void scenelist_row_activated_cb(GtkTreeView* tree_view, GtkTreePath* path, GtkTreeViewColumn* column, gpointer user_data){
	gint depth;
	gint* tree = gtk_tree_path_get_indices_with_depth(path, &depth);

	if ( depth == 1 ){ /* section selected */
		if ( gtk_tree_view_row_expanded(tree_view, path) ){
			gtk_tree_view_collapse_row(tree_view, path);
		} else {
			gtk_tree_view_expand_row(tree_view, path, FALSE);
		}
		return;
	}

	const gint section = tree[0];
	const gint component = tree[1];
	GtkTreeIter iter;
	GtkTreeModel* model = gtk_tree_view_get_model(tree_view);
	if ( !gtk_tree_model_get_iter(model, &iter, path) ){
		return;
	}

	gchar* name;
	gtk_tree_model_get(model, &iter, 0, &name, -1);

	if ( section == 0 ){ /* scene */
		delete scene;
		scene_name = name;
		scene = Scene::create(std::string(name), frame->size);
		scene->add_time(0,60);
		global_time.reset();
	}

	g_free(name);
}

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

	frames++;

	if ( scene ){
		render_scene();
	} else {
		render_placeholder();
	}

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

	if ( scene ){
		delete scene;
		scene = Scene::create(std::string(scene_name), frame->size);
		scene->add_time(0,60);
	}

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

extern "C" G_MODULE_EXPORT void prev_clicked_cb(GtkWidget* widget, gpointer data){
	global_time.step(-1);
	gtk_toggle_button_set_active(playbutton, FALSE);
}

extern "C" G_MODULE_EXPORT void slower_clicked_cb(GtkWidget* widget, gpointer data){
	global_time.adjust_speed(-10);
	gtk_toggle_button_set_active(playbutton, TRUE);
}

extern "C" G_MODULE_EXPORT void faster_clicked_cb(GtkWidget* widget, gpointer data){
	global_time.adjust_speed(10);
	gtk_toggle_button_set_active(playbutton, TRUE);
}

extern "C" G_MODULE_EXPORT void next_clicked_cb(GtkWidget* widget, gpointer data){
	global_time.step(1);
	gtk_toggle_button_set_active(playbutton, FALSE);
}


extern "C" G_MODULE_EXPORT void play_toggled_cb(GtkWidget* widget, gpointer data){
	running = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	if ( running ){
		gtk_button_set_image(GTK_BUTTON(widget), gtk_image_new_from_stock("gtk-media-pause", GTK_ICON_SIZE_SMALL_TOOLBAR));
		global_time.set_paused(false);
	} else {
		gtk_button_set_image(GTK_BUTTON(widget), gtk_image_new_from_stock("gtk-media-play", GTK_ICON_SIZE_SMALL_TOOLBAR));
		global_time.set_paused(true);
	}
}

extern "C" G_MODULE_EXPORT void timelabel_button_press_event_cb(GtkWidget* widget, GdkEvent* event, gpointer data){
	global_time.set_paused(true);
	gtk_toggle_button_set_active(playbutton, FALSE);

	slide_ref = event->button.x;
	gdk_device_grab(gdk_event_get_device(event), gtk_widget_get_parent_window(widget),
	                GDK_OWNERSHIP_WINDOW, TRUE, GDK_BUTTON_RELEASE_MASK, NULL, event->button.time);
}

extern "C" G_MODULE_EXPORT void timelabel_button_release_event_cb(GtkWidget* widget, GdkEvent* event, gpointer data){
	gdk_device_ungrab(gdk_event_get_device(event), event->button.time);
}

extern "C" G_MODULE_EXPORT void timelabel_motion_notify_event_cb(GtkWidget* widget, GdkEvent* event, gpointer data){
	const float dx = slide_ref - event->motion.x;
	const float adj = dx * 0.6; /* >0.5 so 1px will always count to something */
	const int steps = (int)roundf(adj);

	global_time.step(steps);
	slide_ref = event->motion.x;
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
	if ( !gtk_builder_add_from_file (builder, "editor.xml", &error) ){
		g_print( "Error occured while loading UI file!\n" );
		g_print( "Message: %s\n", error->message );
		g_free(error);
		exit(1);
	}

	GtkWidget* window  = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	drawing            = GTK_WIDGET(gtk_builder_get_object(builder, "drawingarea"));
	timelabel          = GTK_LABEL(gtk_builder_get_object(builder, "timelabel"));
	playbutton         = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "play"));

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

	/* setup scene-list */
	GtkTreeIter toplevel;
	GtkTreeStore* scenestore = GTK_TREE_STORE(gtk_builder_get_object(builder, "scenestore"));
	gtk_tree_store_append(scenestore, &toplevel, NULL);
	gtk_tree_store_set(scenestore, &toplevel, 0, "<b>Scenes</b>", -1);

	for ( auto it = Scene::factory_begin(); it != Scene::factory_end(); ++it ){
		GtkTreeIter child;
		gtk_tree_store_append(scenestore, &child, &toplevel);
		gtk_tree_store_set(scenestore, &child, 0, it->first.c_str(), -1);

		Scene::Metadata* meta = it->second.meta;
		for ( std::pair<std::string, std::string> p: *meta ){
			const std::string& key   = p.first;
			GtkTreeIter cur;
			gtk_tree_store_append(scenestore, &cur, &child);
			gtk_tree_store_set(scenestore, &cur, 0, key.c_str(), -1);
		}
	}

	gtk_tree_store_append(scenestore, &toplevel, NULL);
	gtk_tree_store_set(scenestore, &toplevel, 0, "<b>Compositions</b>", -1);

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
