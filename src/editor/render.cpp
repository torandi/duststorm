#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "editor/editor.hpp"
#include "camera.hpp"
#include "engine.hpp"
#include "globals.hpp"
#include "render_object.hpp"
#include "rendertarget.hpp"
#include "scene.hpp"
#include "shader.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <gdk/gdkgl.h>
#include <gtk/gtkgl.h>

static bool initialized = false;
static float aspect = 16.0f / 9.0f;
static Camera camera(60.f, 1.0f, 0.1f, 100.0f);
static Shader::lights_data_t lights;
static glm::vec2 track_ref;
static glm::vec2 track_delta;
static glm::vec2 track_angle(0.0f, M_PI*0.5);
static float track_distance = 1.0f;
static glm::mat4 projection;
static bool keystate[256] = {false, };

struct uniform {
	GLuint index;
	bool value;
};

static void upload(const struct uniform& u){
	glUniform1i(u.index, u.value ? 1 : 0);
}

static struct {
	RenderObject* model;
	struct uniform use_light_a;
	struct uniform use_light_b;
	struct uniform show_shaded;
	struct uniform show_texture;
	struct uniform show_uv;
	struct uniform show_normal;
	struct uniform show_normalmap;
	struct uniform show_tangent;
	struct uniform show_bitangent;
	struct uniform show_color;
	struct uniform show_diffuse;
	struct uniform show_specular;
} modelview;

namespace Editor {
	void set_message(const char* fmt, ...){
		char* buf = nullptr;
		va_list ap;
		va_start(ap, fmt);
		vasprintf(&buf, fmt, ap);
		va_end(ap);
		gtk_label_set_text(message, buf);
		free(buf);
	}

	void reset(){
		global_time.reset();
		track_angle = glm::vec2(0.0f, M_PI*0.5);
		track_distance = 1.0f;
		Editor::camera_control = Editor::CAMERA_AUTO;
		gtk_label_set_text(message, "");
		camera.set_fov(60.0f);
		camera.set_position(glm::vec3(1.f, 0.f, 0.f));
		camera.look_at(glm::vec3(0.f, 0.f, 0.f));
	}

	void load_model(const std::string& filename){
		delete modelview.model;
		modelview.model = new RenderObject(filename, true);
	}

	void load_scene(const std::string& name){
		delete scene;
		scene = nullptr;

		Editor::scene_name = name;
		Editor::mode = Editor::MODE_SCENE;

		scene = SceneFactory::create(name, frame->texture_size());
		if ( !scene ){
			set_message("Failed to load scene \"%s\"", name.c_str());
			Editor::mode = Editor::MODE_BLANK;
			return;
		}

		scene->add_time(0,3600);
		Editor::sceneprops_populate(scene);
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
	/* Render scene */
	scene->bind();
	switch ( Editor::camera_control ){
	case Editor::CAMERA_AUTO:
		scene->render();
		break;
	case Editor::CAMERA_MANUAL:
		scene->clear(Color::white);
		scene->render_geometry(camera);
		Shader::unbind();
	}
	scene->unbind();

	/* Display on framebuffer */
	Shader::upload_projection_view_matrices(frame->ortho(), glm::mat4());
	frame->with([](){
		RenderTarget::clear(Color::magenta);
		scene->draw(shaders[SHADER_PASSTHRU]);
	});
}

static void render_model(){

	frame->bind();
	shaders[SHADER_MODELVIEWER]->bind();
	upload(modelview.use_light_a);
	upload(modelview.use_light_b);
	upload(modelview.show_shaded);
	upload(modelview.show_texture);
	upload(modelview.show_uv);
	upload(modelview.show_normal);
	upload(modelview.show_normalmap);
	upload(modelview.show_tangent);
	upload(modelview.show_bitangent);
	upload(modelview.show_color);
	upload(modelview.show_diffuse);
	upload(modelview.show_specular);

	frame->clear(Color::white);
	Shader::upload_lights(lights);
	Shader::upload_camera(camera);
	Shader::upload_projection_view_matrices(camera.projection_matrix(), camera.view_matrix());
	modelview.model->render();

	Shader::unbind();
	frame->unbind();
}

void drawingarea_update(){
	if ( keystate[25] /* W */ )	camera.relative_move(glm::vec3( 0.0, 0, 0.1));
	if ( keystate[39] /* S */ )	camera.relative_move(glm::vec3( 0.0, 0,-0.1));
	if ( keystate[38] /* A */ )	camera.relative_move(glm::vec3( 0.1, 0, 0.0));
	if ( keystate[40] /* D */ )	camera.relative_move(glm::vec3(-0.1, 0, 0.0));
}

extern "C" G_MODULE_EXPORT void aspect_changed(GtkWidget* widget, gpointer data){
	if ( !gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(widget)) ) return;

	int w,h;
	const gchar* label = gtk_menu_item_get_label(GTK_MENU_ITEM(widget));
	sscanf(label, "%d:%d", &w, &h);
	aspect = (float)w / (float)h;

	gtk_widget_queue_resize(drawing);
}

extern "C" G_MODULE_EXPORT void light_a_toggle(GtkWidget* widget, gpointer data){
	modelview.use_light_a.value = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(widget));;
}

extern "C" G_MODULE_EXPORT void light_b_toggle(GtkWidget* widget, gpointer data){
	modelview.use_light_b.value = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(widget));;
}

extern "C" G_MODULE_EXPORT void show_shaded(GtkWidget* widget, gpointer data){
	modelview.show_shaded.value = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(widget));;
}

extern "C" G_MODULE_EXPORT void show_texture(GtkWidget* widget, gpointer data){
	modelview.show_texture.value = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(widget));;
}

extern "C" G_MODULE_EXPORT void show_uv(GtkWidget* widget, gpointer data){
	modelview.show_uv.value = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(widget));
}

extern "C" G_MODULE_EXPORT void show_normal(GtkWidget* widget, gpointer data){
	modelview.show_normal.value = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(widget));
}

extern "C" G_MODULE_EXPORT void show_normalmap(GtkWidget* widget, gpointer data){
	modelview.show_normalmap.value = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(widget));
}

extern "C" G_MODULE_EXPORT void show_tangent(GtkWidget* widget, gpointer data){
	modelview.show_tangent.value = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(widget));
}

extern "C" G_MODULE_EXPORT void show_bitangent(GtkWidget* widget, gpointer data){
	modelview.show_bitangent.value = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(widget));
}

extern "C" G_MODULE_EXPORT void show_color(GtkWidget* widget, gpointer data){
	modelview.show_color.value = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(widget));
}

extern "C" G_MODULE_EXPORT void show_diffuse(GtkWidget* widget, gpointer data){
	modelview.show_diffuse.value = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(widget));
}

extern "C" G_MODULE_EXPORT void show_specular(GtkWidget* widget, gpointer data){
	modelview.show_specular.value = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(widget));
}

static void recalc_camera(){
	switch ( Editor::mode ){
	case Editor::MODE_MODEL:
		camera.set_position(glm::vec3(
			track_distance * sinf(track_angle.y) * cosf(-track_angle.x),
			track_distance * cosf(track_angle.y),
			track_distance * sinf(track_angle.y) * sinf(-track_angle.x)));
		break;

	case Editor::MODE_SCENE:
		camera.absolute_rotate(glm::vec3(0,1,0),  track_delta.x * 0.01);
		camera.relative_rotate(glm::vec3(1,0,0), -track_delta.y * 0.01);
		break;

	default:
		break;
	}
}

extern "C" G_MODULE_EXPORT void drawingarea_motion_notify_event_cb(GtkWidget* widget, GdkEvent* event, gpointer data){
	/* Enable manual control of camera */
	if ( Editor::mode == Editor::MODE_SCENE && Editor::camera_control == Editor::CAMERA_AUTO ){
		Editor::camera_control = Editor::CAMERA_MANUAL;
		camera = scene->get_current_camera();
		gtk_label_set_text(message, "Manual camera control (press SPACE to reset, ENTER to print coordinates)");
	}

	const glm::vec2 p = glm::vec2(event->motion.x, event->motion.y);
	track_delta  = (track_ref - p) * 0.3f;
	track_angle += glm::radians(track_delta);
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
	/* grab focus so keyboard events goes to drawingarea */
	gtk_widget_grab_focus(widget);

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

extern "C" G_MODULE_EXPORT void drawingarea_key_event_cb(GtkWidget* widget, GdkEventKey* event, gpointer data){
	if ( Editor::mode != Editor::MODE_SCENE ) return;
	if ( event->hardware_keycode > 255 || event->is_modifier ) return;

	/* Reset camera */
	if ( event->hardware_keycode == 65 ){
		for ( int i = 0; i < 256; i++ ) keystate[i] = false;
		Editor::camera_control = Editor::CAMERA_AUTO;
		gtk_label_set_text(message, "");
		return;
	}

	/* Print camera coordinates */
	if ( event->hardware_keycode == 36 && event->type == GDK_KEY_PRESS ){
		static int n = 1;
		fprintf(stderr, "[%3d] Camera position=(%f,%f,%f)\n",
		        n++, camera.position().x, camera.position().y, camera.position().z);
		return;
	}

	/* Enable manual control of camera */
	if ( Editor::camera_control == Editor::CAMERA_AUTO ){
		Editor::camera_control = Editor::CAMERA_MANUAL;
		camera = scene->get_current_camera();
		gtk_label_set_text(message, "Manual camera control (press SPACE to reset, ENTER to print coordinates)");
	}

	keystate[event->hardware_keycode] = event->type == GDK_KEY_PRESS;

}

extern "C" G_MODULE_EXPORT gboolean drawingarea_draw_cb(GtkWidget* widget, gpointer data){
	if ( !initialized ) return FALSE;
	if (!gtk_widget_begin_gl (widget)) return FALSE;

	Editor::frames++;

	switch ( Editor::mode ){
	case Editor::MODE_SCENE:
		render_scene();
		break;

	case Editor::MODE_MODEL:
		render_model();
		break;

	case Editor::MODE_BLANK:
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
	frame = new RenderTarget(size, GL_RGB8, true, GL_NEAREST);

	if ( Editor::scene_name != "" ){
		delete scene;
		scene = SceneFactory::create(Editor::scene_name, frame->texture_size());
		if ( scene ){
			Editor::mode = Editor::MODE_SCENE; /* may not be set, for instance when loaded from main */
			scene->add_time(0,60);
		} else {
			Editor::set_message("Failed to load scene \"%s\"", Editor::scene_name.c_str());
			Editor::mode = Editor::MODE_BLANK;
		}
	}

	camera.set_aspect(aspect);

	/* initialize modelviewer */
	Shader* s = shaders[SHADER_MODELVIEWER];
	modelview.model = nullptr;
	modelview.use_light_a    = {s->uniform_location("use_light[0]"), true};
	modelview.use_light_b    = {s->uniform_location("use_light[1]"), false};
	modelview.show_shaded    = {s->uniform_location("show_shaded"), true};
	modelview.show_texture   = {s->uniform_location("show_texture"), false};
	modelview.show_uv        = {s->uniform_location("show_uv"), false};
	modelview.show_normal    = {s->uniform_location("show_normal"), false};
	modelview.show_normalmap = {s->uniform_location("show_normalmap"), false};
	modelview.show_tangent   = {s->uniform_location("show_tangent"), false};
	modelview.show_bitangent = {s->uniform_location("show_bitangent"), false};
	modelview.show_color     = {s->uniform_location("show_color"), false};
	modelview.show_diffuse   = {s->uniform_location("show_diffuse"), true};
	modelview.show_specular  = {s->uniform_location("show_specular"), true};

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

	lights.ambient_intensity = glm::vec3(0.0f);
	lights.num_lights = 2;

	lights.lights[0].constant_attenuation = 1.0f;
	lights.lights[0].linear_attenuation = 0.01f;
	lights.lights[0].quadratic_attenuation = 0.002f;
	lights.lights[0].type = Light::POINT_LIGHT;
	lights.lights[0].intensity = glm::vec3(0.7f);
	lights.lights[0].position = glm::vec3(2.0f, 1.5f, 2.0f);

	lights.lights[1].constant_attenuation = 0.1f;
	lights.lights[1].linear_attenuation = 0.1f;
	lights.lights[1].quadratic_attenuation = 0.0f;
	lights.lights[1].type = Light::POINT_LIGHT;
	lights.lights[1].intensity = glm::vec3(0.3f, 0.3f, 0.9f);
	lights.lights[1].position = glm::vec3(-2.0f, 1.3f, 4.0f);

  gtk_widget_end_gl(widget, FALSE);
  gtk_widget_queue_resize(widget);
  initialized = true;
}
