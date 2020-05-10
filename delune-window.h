#ifndef _DELUNE_WINDOW_H_
#define _DELUNE_WINDOW_H_

#include <gtk/gtk.h>

#include "delune-application.h"

#define DELUNE_WINDOW_TYPE (delune_window_get_type())
G_DECLARE_FINAL_TYPE(DeluneWindow, delune_window, DELUNE, WINDOW, GtkApplicationWindow)

DeluneWindow *delune_window_new(DeluneApplication *app);

#endif /* _DELUNE-WINDOW_H_ */
