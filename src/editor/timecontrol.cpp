#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "editor/globals.hpp"
#include <cmath>

static float slide_ref;

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
