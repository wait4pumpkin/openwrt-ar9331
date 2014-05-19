#ifndef _AIRPLAY_RENDERER_H
#define _AIRPLAY_RENDERER_H

#include <glib-object.h>

#define AIRPLAY_TYPE_RENDERER (airplayRenderer_get_type())
#define AIRPLAY_RENDERER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), AIRPLAY_TYPE_RENDERER, AirplayRenderer))
#define AIRPLAY_IS_RENDERER(obj) G_TYPE_CHECK_INSTANCE_TYPE((obj), AIRPLAY_TYPE_RENDERER))
#define AIRPLAY_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), AIRPLAY_TYPE_RENDERER, AirplayRendererClass))
#define AIRPLAY_IS_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), AIRPLAY_RENDERER))
#define AIRPLAY_RENDERER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), AIRPLAY_TYPE_RENDERER, AirplayRendererClass))

typedef struct _AirplayRenderer AirplayRenderer;
struct _AirplayRenderer {
	GObject parent;
};
 
typedef struct _AirplayRendererClass AirplayRendererClass;
struct _AirplayRendererClass {
	GObjectClass parentClass;
};

GType airplayRenderer_get_type(void);
void airplayRendererRecord(gpointer *instance, gchar *buffer, gpointer userData);
gint airplayRendererStart(AirplayRenderer *self, gint *controlPort, gint *timingPort);
void airplayRendererFlush(AirplayRenderer *self);
void airplayRendererMute(AirplayRenderer *self, gboolean mute);
void airplayRendererSetVolume(AirplayRenderer *self, gdouble volume);

#endif
