#include "AirplayMediaPlayer.h"

G_DEFINE_TYPE(AirplayMediaPlayer, airplayMediaPlayer, G_TYPE_OBJECT);

static void airplayMediaPlayerFinalize(GObject *object);
static void airplayMediaPlayerGetProperty(GObject *object, guint propertyID, GValue *value, GParamSpec *pspec);
static void airplayMediaPlayerSetProperty(GObject *object, guint propertyID, const GValue *value, GParamSpec *pspec);

#define AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE((object), AIRPLAY_TYPE_MEDIA_PLAYER, AirplayMediaPlayerPrivate))

enum PROPERTY_MEDIA_PLAYER {
	PROPERTY_BASE,
	N_PROPERTIES
};

typedef struct _AirplayMediaPlayerPrivate AirplayMediaPlayerPrivate;
struct _AirplayMediaPlayerPrivate {
	GstElement *playbin;
	GstBus *bus;
	guint busWatchID;
};

static gboolean busWatcher(GstBus *bus, GstMessage *message, gpointer data) {
	g_print("Got %s message\n", GST_MESSAGE_TYPE_NAME (message));

//	AirplayMediaPlayer *self = AIRPLAY_MEDIA_PLAYER(data);
//	AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);

	switch(GST_MESSAGE_TYPE (message)) {
		case GST_MESSAGE_ERROR: {
			GError *err;
			gchar *debug;

			gst_message_parse_error(message, &err, &debug);
			g_print("Error: %s\n", err->message);
			g_error_free(err);
			g_free(debug);

			break;
		}

		case GST_MESSAGE_EOS:
			break;

		default:
			break;
	}
	
	return TRUE;
}

static void airplayMediaPlayer_class_init(AirplayMediaPlayerClass *klass) {
	g_type_class_add_private(klass, sizeof(AirplayMediaPlayerPrivate));

	GObjectClass *baseClass = G_OBJECT_CLASS(klass);
	baseClass->set_property = airplayMediaPlayerSetProperty;
	baseClass->get_property = airplayMediaPlayerGetProperty;
	baseClass->finalize=airplayMediaPlayerFinalize;

	//GParamSpec *properties[N_PROPERTIES] = {NULL, };

	g_print("AirplayMediaPlayer: Class INIT\n");
}

static void airplayMediaPlayer_init(AirplayMediaPlayer *self) {
	AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);
	priv->playbin = gst_element_factory_make("playbin2", "playbin");
	//GstElement *sink = gst_element_factory_make("autoaudiosink", "audiosink");
//	g_object_set(G_OBJECT(sink), "sync", FALSE, NULL);
	g_object_set(G_OBJECT(priv->playbin), "buffer-size", 40960, NULL);
	GstElement *sink = gst_element_factory_make("alsasink", "alsasink");
	g_object_set(G_OBJECT(sink), "sync", FALSE, NULL);
	g_object_set(G_OBJECT(priv->playbin), "audio-sink", sink, NULL);
	priv->bus = gst_pipeline_get_bus(GST_PIPELINE(priv->playbin));
	priv->busWatchID = gst_bus_add_watch(priv->bus, busWatcher, self);

	g_print("AirplayMediaPlayer: INIT\n");
}

static void airplayMediaPlayerFinalize(GObject *object) {
	g_print("AirplayMediaPlayer: Finalize\n");

	AirplayMediaPlayer *self = AIRPLAY_MEDIA_PLAYER(object);
	AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);

	gst_element_set_state(priv->playbin, GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(priv->playbin));
	g_source_remove(priv->busWatchID);

	G_OBJECT_CLASS(airplayMediaPlayer_parent_class)->dispose(object);
}

static void airplayMediaPlayerGetProperty(GObject *object, guint propertyID, GValue *value, GParamSpec *pspec) {
	g_print("AirplayMediaPlayer: Get Property\n");

	//AirplayMediaPlayer *self = AIRPLAY_MEDIA_PLAYER(object);
	//AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);

	switch(propertyID) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propertyID, pspec);
			break;
	}
}

static void airplayMediaPlayerSetProperty(GObject *object, guint propertyID, const GValue *value, GParamSpec *pspec) {      
	g_print("AirplayMediaPlayer: Set Property\n");

	//AirplayMediaPlayer *self = AIRPLAY_MEDIA_PLAYER(object);
	//AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);

	switch(propertyID) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propertyID, pspec);
			break;
	}
}

void airplayMediaPlayerConnect(AirplayMediaPlayer *self, gchar *signal, GCallback callback, gpointer userData) {
	AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);
	
	g_signal_connect(priv->playbin, signal, callback, userData);
}

void airplayMediaPlayerSetURI(AirplayMediaPlayer *self, gchar *uri) {
	g_print("AirplayMediaPlayer: SetURI\n");

	AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);

	g_object_set(G_OBJECT(priv->playbin), "uri", uri, NULL);
}

void airplayMediaPlayerPlay(AirplayMediaPlayer *self) {
	g_print("AirplayMediaPlayer: Play\n");

	AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);
	gst_element_set_state(priv->playbin, GST_STATE_PLAYING);
}

void airplayMediaPlayerPause(AirplayMediaPlayer *self) {
	g_print("AirplayMediaPlayer: Pause\n");

	AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);
	gst_element_set_state(priv->playbin, GST_STATE_PAUSED);
}

void airplayMediaPlayerStop(AirplayMediaPlayer *self) {
	g_print("AirplayMediaPlayer: Stop\n");

	AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);

	gst_element_set_state(priv->playbin, GST_STATE_NULL);
}

void airplayMediaPlayerMute(AirplayMediaPlayer *self, gboolean mute) {
	g_print("AirplayMediaPlayer: Mute: %d\n", mute);

	AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);

	g_object_set(G_OBJECT(priv->playbin), "mute", mute, NULL);
}

void airplayMediaPlayerSetVolume(AirplayMediaPlayer *self, gdouble volume) {
	g_print("AirplayMediaPlayer: Volume: %f\n", volume);

	AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);

	g_object_set(G_OBJECT(priv->playbin), "volume", volume, NULL);
}
