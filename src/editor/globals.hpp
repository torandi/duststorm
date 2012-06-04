#ifndef EDITOR_GLOBALS_H
#define EDITOR_GLOBALS_H

#include "time.hpp"
#include <gtk/gtk.h>

extern Time global_time;
extern bool running;

/* editor widgets */
extern GtkWidget* drawing;
extern GtkLabel* timelabel;
extern GtkToggleButton* playbutton;
extern GtkListStore* propstore;
extern RenderTarget* frame;

#endif /* EDITOR_GLOBALS_H */
