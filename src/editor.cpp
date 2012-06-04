#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
#include "camera.hpp"
#include "color.hpp"
#include "engine.hpp"
#include "globals.hpp"
#include "rendertarget.hpp"
#include "render_object.hpp"
#include "scene.hpp"
#include "time.hpp"

enum MODE {
	MODE_BLANK,
	MODE_SCENE,
	MODE_PATH,
	MODE_MODEL,
} mode = MODE_BLANK;

enum COL {
	COL_ICON = 0,
	COL_TITLE,
	COL_FILENAME,
	COL_TYPE,
};

enum TYPE: gint {
	TYPE_CAT = 0,
	TYPE_SCENE,
	TYPE_COMPOSITION,
	TYPE_PATH,
	TYPE_MODEL,
	TYPE_UNKNOWN,
};

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
static RenderObject* model = nullptr;
static Camera camera(60.f, 1.0f, 0.1f, 100.0f);
static float slide_ref;
static glm::vec2 track_ref;
static glm::vec2 track_angle(0.0f, M_PI*0.5);
static float track_distance = 1.0f;

/* icons */
static GdkPixbuf* icon_cat_scene = nullptr;
static GdkPixbuf* icon_scene     = nullptr;
static GdkPixbuf* icon_model     = nullptr;
static GdkPixbuf* icon_path      = nullptr;

static void show_fps(int signum){
	//fprintf(stderr, "FPS: %d\n", frames);
	frames = 0;
}

namespace Engine {
	RenderTarget* rendertarget_by_name(const std::string& fullname){
		/* not implemented */
		return nullptr;
	}
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

	Shader::upload_projection_view_matrices(frame->ortho(), glm::mat4());
	frame->with([](){
		RenderTarget::clear(Color::magenta);
		scene->draw(shaders[SHADER_PASSTHRU]);
	});
}

static void render_model(){
	frame->bind();
	shaders[SHADER_MODELVIEWER]->bind();
	frame->clear(Color::white);
	Shader::upload_projection_view_matrices(camera.projection_matrix(), camera.view_matrix());
	model->render(shaders[SHADER_MODELVIEWER]);
	Shader::unbind();
	frame->unbind();
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
	TYPE type;
	gtk_tree_model_get(treemodel, &iter, COL_TITLE, &name, COL_TYPE, &type, COL_FILENAME, &filename, -1);

	switch ( type ){
	case TYPE_CAT:
		if ( gtk_tree_view_row_expanded(tree_view, path) ){
			gtk_tree_view_collapse_row(tree_view, path);
		} else {
			gtk_tree_view_expand_row(tree_view, path, FALSE);
		}
		break;

	case TYPE_SCENE:
		mode = MODE_SCENE;
		delete scene;
		scene_name = name;
		scene = SceneFactory::create(std::string(name), frame->texture_size());
		assert(scene);
		scene->add_time(0,60);
		global_time.reset();
		break;

	case TYPE_PATH:
		break;

	case TYPE_MODEL:
		mode = MODE_MODEL;
		delete model;
		model = new RenderObject(filename);
		track_angle = glm::vec2(0.0f, M_PI*0.5);
		track_distance = 1.0f;
		camera.set_position(glm::vec3(1.f, 0.f, 0.f));
		camera.look_at(glm::vec3(0.f, 0.f, 0.f));
		break;

	default:
		printf("type: %d\n", type);
		break;
	}

	g_free(name);
	g_free(filename);
}

extern "C" G_MODULE_EXPORT void aspect_changed(GtkWidget* widget, gpointer data){
	if ( !gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(widget)) ) return;

	int w,h;
	const gchar* label = gtk_menu_item_get_label(GTK_MENU_ITEM(widget));
	sscanf(label, "%d:%d", &w, &h);
	aspect = (float)w / (float)h;

	gtk_widget_queue_resize(drawing);
}

static void recalc_camera(){
	const glm::vec3 point(
		track_distance * sinf(track_angle.y) * cosf(-track_angle.x),
		track_distance * cosf(track_angle.y),
		track_distance * sinf(track_angle.y) * sinf(-track_angle.x));
	camera.set_position(point);
}

extern "C" G_MODULE_EXPORT void drawingarea_motion_notify_event_cb(GtkWidget* widget, GdkEvent* event, gpointer data){
	const glm::vec2 p = glm::vec2(event->motion.x, event->motion.y);
	const glm::vec2 d = (track_ref - p) * 0.3f;
	track_angle += glm::radians(d);
	while ( track_angle.x < 0.0f   ) track_angle.x += 2*M_PI;
	while ( track_angle.x > 2*M_PI ) track_angle.x -= 2*M_PI;
	track_angle.y = glm::clamp(track_angle.y, 0.01f, (float)(M_PI-0.01f));
	track_ref = p;
	recalc_camera();
}

extern "C" G_MODULE_EXPORT void drawingarea_scroll_event_cb(GtkWidget* widget, GdkEvent* event, gpointer data){
	if ( event->scroll.direction == GDK_SCROLL_UP && track_distance > 0.2f){
		track_distance -= 0.1f;
	} else if ( event->scroll.direction == GDK_SCROLL_DOWN  && track_distance < 10.f ){
		track_distance += 0.1f;
	}
	recalc_camera();
}

extern "C" G_MODULE_EXPORT void drawingarea_button_press_event_cb(GtkWidget* widget, GdkEvent* event, gpointer data){
	if ( event->button.button != 1 ) return;

	track_ref.x = event->button.x;
	track_ref.y = event->button.y;
	gdk_device_grab(gdk_event_get_device(event), gtk_widget_get_parent_window(widget),
	                GDK_OWNERSHIP_WINDOW, TRUE, GDK_POINTER_MOTION_MASK, NULL, event->button.time);
}

extern "C" G_MODULE_EXPORT void drawingarea_button_release_event_cb(GtkWidget* widget, GdkEvent* event, gpointer data){
	if ( event->button.button != 1 ) return;

	const glm::vec2 p = glm::vec2(event->motion.x, event->motion.y);
	const glm::vec2 d = track_ref - p;
	track_angle += glm::radians(d) * 0.3f;
	track_angle.y = glm::clamp(track_angle.y, 0.01f, (float)(M_PI-0.01f));

	gdk_device_ungrab(gdk_event_get_device(event), event->button.time);
}

extern "C" G_MODULE_EXPORT gboolean drawingarea_draw_cb(GtkWidget* widget, gpointer data){
	if ( !initialized ) return FALSE;
	if (!gtk_widget_begin_gl (widget)) return FALSE;

	frames++;

	switch ( mode ){
	case MODE_SCENE:
		render_scene();
		break;

	case MODE_MODEL:
		render_model();
		break;

	case MODE_BLANK:
		render_placeholder();
		break;
	}

	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	const glm::ivec2 center = (resolution - frame->texture_size()) / 2;
	Shader::upload_state(resolution);
	Shader::upload_projection_view_matrices(projection, glm::mat4());
	glViewport(0, 0, resolution.x, resolution.y);
	frame->draw(shaders[SHADER_PASSTHRU], center);

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
		scene = SceneFactory::create(std::string(scene_name), frame->texture_size());
		scene->add_time(0,60);
	}

	camera.set_aspect(aspect);

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
	delete opencl;
	opencl = new CL();

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

	/* load icons */
	icon_cat_scene = gdk_pixbuf_new_from_file(PATH_SRC "pictures.png", NULL);
	icon_scene     = gdk_pixbuf_new_from_file(PATH_SRC "picture.png", NULL);
	icon_model     = gdk_pixbuf_new_from_file(PATH_SRC "brick.png", NULL);
	icon_path      = gdk_pixbuf_new_from_file(PATH_SRC "map.png", NULL);

	/* setup scene-list */
	GtkTreeIter toplevel;
	GtkTreeStore* scenestore = GTK_TREE_STORE(gtk_builder_get_object(builder, "scenestore"));
	gtk_tree_store_append(scenestore, &toplevel, NULL);
	gtk_tree_store_set(scenestore, &toplevel,
	                   COL_ICON, icon_cat_scene,
	                   COL_TITLE, "<b>Scenes</b>",
	                   COL_TYPE, TYPE_CAT,
	                   -1);

	for ( auto it = SceneFactory::begin(); it != SceneFactory::end(); ++it ){
		GtkTreeIter child;
		gtk_tree_store_append(scenestore, &child, &toplevel);
		gtk_tree_store_set(scenestore, &child,
		                   COL_ICON, icon_scene,
		                   COL_TITLE, it->first.c_str(),
		                   COL_TYPE, TYPE_SCENE,
		                   -1);

		SceneFactory::Metadata* meta = it->second.meta;
		for ( std::pair<std::string, std::string> p: *meta ){
			const std::string& key   = p.first;
			const std::string& value = p.second;
			std::string filename = "";
			GdkPixbuf* icon = nullptr;
			gint type = TYPE_UNKNOWN;

			char* data = strdup(value.c_str());
			char* t = strtok(data, ":");
			char* d = strtok(NULL, ":");

			if ( strcmp(t, "path") == 0 ){
				type = TYPE_PATH;
				icon = icon_path;
				filename = std::string(d?d:"<nil>");
			} else if ( strcmp(t, "model") == 0 ){
				type = TYPE_MODEL;
				icon = icon_model;
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
