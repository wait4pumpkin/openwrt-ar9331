#ifndef _AIRPLAY_CONTROLLER_H
#define _AIRPLAY_CONTROLLER_H

#include <glib-object.h>

#define AIRPLAY_TYPE_CONTROLLER (airplayController_get_type())
#define AIRPLAY_CONTROLLER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), AIRPLAY_TYPE_CONTROLLER, AirplayController))
#define AIRPLAY_IS_CONTROLLER(obj) G_TYPE_CHECK_INSTANCE_TYPE((obj), AIRPLAY_TYPE_CONTROLLER))
#define AIRPLAY_CONTROLLER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), AIRPLAY_TYPE_CONTROLLER, AirplayControllerClass))
#define AIRPLAY_IS_CONTROLLER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), AIRPLAY_CONTROLLER))
#define AIRPLAY_CONTROLLER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), AIRPLAY_TYPE_CONTROLLER, AirplayControllerClass))

typedef struct _AirplayController AirplayController;
struct _AirplayController {
	GObject parent;
};
 
typedef struct _AirplayControllerClass AirplayControllerClass;
struct _AirplayControllerClass {
	GObjectClass parentClass;
	void (*defaultHandler) (gpointer instance, const gchar *buffer, gpointer userdata);
};

GType airplayController_get_type(void);

#endif
