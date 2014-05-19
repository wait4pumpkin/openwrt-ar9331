#include "UpnpMediaPlayer.h"

#include <stdio.h>
#include <stdlib.h>

G_DEFINE_TYPE(UpnpMediaPlayer, upnpMediaPlayer, G_TYPE_OBJECT);

static void upnpMediaPlayerDispose(GObject *object);
static void upnpMediaPlayerGetProperty(GObject *object, guint propertyID, GValue *value, GParamSpec *pspec);
static void upnpMediaPlayerSetProperty(GObject *object, guint propertyID, const GValue *value, GParamSpec *pspec);

#define UPNP_MEDIA_PLAYER_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE((object), UPNP_TYPE_MEDIA_PLAYER, UpnpMediaPlayerPrivate))

enum PROPERTY_MEDIA_PLAYER {
	PROPERTY_BASE,
	// PROPERTY_DEVICE,
	N_PROPERTIES
};

typedef struct _UpnpMediaPlayerPrivate UpnpMediaPlayerPrivate;
struct _UpnpMediaPlayerPrivate {
	GstElement *playbin;
	GstElement *pipeline;
	GstBus *bus;
	
	gboolean isEnable;
};

static gboolean busWatcher(GstBus *bus, GstMessage *message, gpointer data) {
	UpnpMediaPlayer *self = UPNP_MEDIA_PLAYER(data);
	UpnpMediaPlayerPrivate *priv = UPNP_MEDIA_PLAYER_GET_PRIVATE(self);

	switch(GST_MESSAGE_TYPE (message)) {
		case GST_MESSAGE_ERROR: {
			GError *err;
			gchar *debug;

			gst_message_parse_error(message, &err, &debug);
			g_error_free(err);
			g_free(debug);

			break;
		}

		case GST_MESSAGE_EOS:
			gst_element_set_state(priv->playbin, GST_STATE_NULL);	
			break;

		default:
			break;
	}
	
	return TRUE;
}

static void upnpMediaPlayer_class_init(UpnpMediaPlayerClass *klass) {
	g_type_class_add_private(klass, sizeof(UpnpMediaPlayerPrivate));

	GObjectClass *baseClass = G_OBJECT_CLASS(klass);
	baseClass->set_property = upnpMediaPlayerSetProperty;
	baseClass->get_property = upnpMediaPlayerGetProperty;
	baseClass->dispose=upnpMediaPlayerDispose;

	//GParamSpec *properties[N_PROPERTIES] = {NULL, };
	// properties[PROPERTY_DEVICE] = g_param_spec_object("device",
	// 												  "Device",
	// 												  "GUPnPRootDevice",
	// 												  GUPNP_TYPE_ROOT_DEVICE, 
	// 												  G_PARAM_WRITABLE | G_PARAM_CONSTRUCT);

	// g_object_class_install_properties(baseClass, N_PROPERTIES, properties);

	gst_init(NULL, NULL);
}

static void upnpMediaPlayer_init(UpnpMediaPlayer *self) {
	UpnpMediaPlayerPrivate *priv = UPNP_MEDIA_PLAYER_GET_PRIVATE(self);

	priv->playbin = gst_element_factory_make("playbin2", "play");
	priv->pipeline = (GstElement *)GST_PIPELINE(priv->playbin);
	priv->bus = gst_pipeline_get_bus((GstPipeline *)priv->pipeline);
	gst_bus_add_watch(priv->bus, busWatcher, self);
	
	priv->isEnable = TRUE;
}

static void upnpMediaPlayerDispose(GObject *object) {
	UpnpMediaPlayer *self = UPNP_MEDIA_PLAYER(object);
	UpnpMediaPlayerPrivate *priv = UPNP_MEDIA_PLAYER_GET_PRIVATE(self);

	gst_element_set_state(priv->playbin, GST_STATE_NULL);
  	gst_object_unref(GST_OBJECT(priv->bus));
  	gst_object_unref(GST_OBJECT(priv->pipeline));
  	gst_object_unref(GST_OBJECT(priv->playbin));

  	G_OBJECT_CLASS(upnpMediaPlayer_parent_class)->dispose(object);
}

static void upnpMediaPlayerGetProperty(GObject *object, guint propertyID, GValue *value, GParamSpec *pspec) {
//	UpnpMediaPlayer *self = UPNP_MEDIA_PLAYER(object);
//	UpnpMediaPlayerPrivate *priv = UPNP_MEDIA_PLAYER_GET_PRIVATE(self);

	switch(propertyID) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propertyID, pspec);
			break;
	}
}

static void upnpMediaPlayerSetProperty(GObject *object, guint propertyID, const GValue *value, GParamSpec *pspec) {      
//	UpnpMediaPlayer *self = UPNP_MEDIA_PLAYER(object);
//	UpnpMediaPlayerPrivate *priv = UPNP_MEDIA_PLAYER_GET_PRIVATE(self);

	switch(propertyID) {
		// case PROPERTY_DEVICE:
		// 	priv->device = GUPNP_ROOT_DEVICE(g_value_get_object(value));
		// 	break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propertyID, pspec);
			break;
	}
}

void upnpMediaPlayerSetURI(UpnpMediaPlayer *self, gchar *uri) {
	UpnpMediaPlayerPrivate *priv = UPNP_MEDIA_PLAYER_GET_PRIVATE(self);

	g_object_set(G_OBJECT(priv->playbin), "uri", uri, NULL);
}

void upnpMediaPlayerPlay(UpnpMediaPlayer *self) {
	UpnpMediaPlayerPrivate *priv = UPNP_MEDIA_PLAYER_GET_PRIVATE(self);

	// g_object_set(G_OBJECT(priv->playbin), "uri", "http://localhost/abc.mp3", NULL);
	if (priv->isEnable) {
		gst_element_set_state(priv->playbin, GST_STATE_PLAYING);
	}
}

void upnpMediaPlayerStop(UpnpMediaPlayer *self) {
	UpnpMediaPlayerPrivate *priv = UPNP_MEDIA_PLAYER_GET_PRIVATE(self);

	// g_object_set(G_OBJECT(priv->playbin), "uri", "http://localhost/abc.mp3", NULL);
	gst_element_set_state(priv->playbin, GST_STATE_NULL);
}

void upnpMediaPlayerPause(UpnpMediaPlayer *self) {
	UpnpMediaPlayerPrivate *priv = UPNP_MEDIA_PLAYER_GET_PRIVATE(self);
	gst_element_set_state(priv->playbin, GST_STATE_PAUSED);	
}

GstState upnpMediagetstate(UpnpMediaPlayer *self) {
	GstState state;
	UpnpMediaPlayerPrivate *priv = UPNP_MEDIA_PLAYER_GET_PRIVATE(self);
	gst_element_get_state(priv->pipeline, &state, NULL, GST_CLOCK_TIME_NONE);
//	g_print("the state is: %d\n", state);
	return state;
}

void upnpMediaPlayerNext(UpnpMediaPlayer *self) {
	UpnpMediaPlayerPrivate *priv = UPNP_MEDIA_PLAYER_GET_PRIVATE(self);

	// g_object_set(G_OBJECT(priv->playbin), "uri", "http://localhost/abc.mp3", NULL);
	gst_element_set_state(priv->playbin, GST_STATE_NULL);
}

void upnpMediaPlayerPrev(UpnpMediaPlayer *self) {

	UpnpMediaPlayerPrivate *priv = UPNP_MEDIA_PLAYER_GET_PRIVATE(self);
	gst_element_set_state(priv->playbin, GST_STATE_PAUSED);	
}

void upnpMediaPlayerSetMute(UpnpMediaPlayer *self, gboolean mute)
{
	UpnpMediaPlayerPrivate *priv = UPNP_MEDIA_PLAYER_GET_PRIVATE(self);
		g_object_set(G_OBJECT(priv->playbin), "mute", mute, NULL);
}

void upnpMediaPlayerSetVolume(UpnpMediaPlayer *self, gdouble volume)
{
	UpnpMediaPlayerPrivate *priv = UPNP_MEDIA_PLAYER_GET_PRIVATE(self);
	g_object_set(G_OBJECT(priv->playbin), "volume", volume, NULL);
}

void upnpMediaPlayerSetLoudness(UpnpMediaPlayer *self, gboolean loudness)
{

}

gint64 upnpMediaPlayerQueryDuration(UpnpMediaPlayer *self) {
	
	gint64 length = -1;

	GstFormat fm = GST_FORMAT_TIME;
	UpnpMediaPlayerPrivate *priv = UPNP_MEDIA_PLAYER_GET_PRIVATE(self);
	gst_element_query_duration(priv->pipeline, &fm, &length);
	
	return length;
}

gint64 upnpMediaPlayerQueryPosition(UpnpMediaPlayer *self) {
	gint64 position = -1;

	GstFormat fm = GST_FORMAT_TIME;
	UpnpMediaPlayerPrivate *priv = UPNP_MEDIA_PLAYER_GET_PRIVATE(self);
	gst_element_query_position(priv->pipeline, &fm, &position);
	
	return position;
}

void upnpMediaPlayerSeek(UpnpMediaPlayer *self, gint64 time) {
	GstFormat fm = GST_FORMAT_TIME;
	UpnpMediaPlayerPrivate *priv = UPNP_MEDIA_PLAYER_GET_PRIVATE(self);
	gst_element_seek(priv->pipeline, 1.0, fm, GST_SEEK_FLAG_FLUSH,
					 GST_SEEK_TYPE_SET, time,
					 GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
}

void upnpMediaPlayerEnable(UpnpMediaPlayer *self, gboolean enable) {
	UpnpMediaPlayerPrivate *priv = UPNP_MEDIA_PLAYER_GET_PRIVATE(self);
	
	priv->isEnable = enable;
	if (!enable) {
		upnpMediaPlayerStop(self);
	}
}

