#include "AirplayMediaPlayer.h"

#include <string.h>
#include <gst/app/gstappsrc.h>

#include "AirplayRenderer.h"

G_DEFINE_TYPE(AirplayMediaPlayer, airplayMediaPlayer, G_TYPE_OBJECT);

static void airplayMediaPlayerFinalize(GObject *object);
static void airplayMediaPlayerGetProperty(GObject *object, guint propertyID, GValue *value, GParamSpec *pspec);
static void airplayMediaPlayerSetProperty(GObject *object, guint propertyID, const GValue *value, GParamSpec *pspec);

#define AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE((object), AIRPLAY_TYPE_MEDIA_PLAYER, AirplayMediaPlayerPrivate))

enum PROPERTY_MEDIA_PLAYER {
	PROPERTY_BASE,
	PROPERTY_RENDERER, 
	N_PROPERTIES
};

typedef struct _AirplayMediaPlayerPrivate AirplayMediaPlayerPrivate;
struct _AirplayMediaPlayerPrivate {
	AirplayRenderer *renderer;

	GstBus *bus;
	guint busWatchID;

	guint codecDataSize;
	guint8 *codecData;

	GstElement *pipeline;
	GstElement *appsrc;
	GstElement *queue;
	GstElement *decode;
	GstElement *volume;
	GstElement *audioSink;
};

static gboolean busWatcher(GstBus *bus, GstMessage *message, gpointer data) {
	AirplayMediaPlayer *self = AIRPLAY_MEDIA_PLAYER(data);
	AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);

	gint percent = 0;
	switch(GST_MESSAGE_TYPE (message)) {
		case GST_MESSAGE_BUFFERING:
			gst_message_parse_buffering(message, &percent);
			g_print("Buffer: %d\n", percent);
			if(percent < 100) {
				gst_element_set_state(priv->pipeline, GST_STATE_PAUSED);
			} else {
				gst_element_set_state(priv->pipeline, GST_STATE_PLAYING);
			}

			break;

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
			g_print("Got %s message\n", GST_MESSAGE_TYPE_NAME (message));
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

	GParamSpec *properties[N_PROPERTIES] = {NULL, };
	properties[PROPERTY_RENDERER] = g_param_spec_object("renderer",
														"Renderer",
														"Airplay Renderer",
														AIRPLAY_TYPE_RENDERER, 
														G_PARAM_WRITABLE | G_PARAM_CONSTRUCT);
	g_object_class_install_properties(baseClass, N_PROPERTIES, properties);

	g_print("AirplayMediaPlayer: Class INIT\n");
}

static void airplayMediaPlayer_init(AirplayMediaPlayer *self) {
	AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);

	priv->renderer = NULL;
	priv->codecDataSize = 36;
	priv->codecData = g_malloc(priv->codecDataSize * sizeof(guint8));

	priv->pipeline = gst_pipeline_new("pipeline");
	priv->appsrc = gst_element_factory_make("appsrc", "source");
	priv->queue = gst_element_factory_make("queue2", "queue");
	priv->decode = gst_element_factory_make("ffdec_alac", "decode");
	g_object_set(G_OBJECT(priv->queue), "use-buffering", TRUE, "max-size-buffers", 250, "low-percent", 10, NULL);
	priv->volume = gst_element_factory_make("volume", "volume");
	priv->audioSink = gst_element_factory_make("alsasink", "alsasink");
	g_object_set(G_OBJECT(priv->audioSink), "sync", FALSE, NULL);

	guint size = priv->codecDataSize;
	guint8 *codecData = priv->codecData;
 	memset(codecData, 0, size);
 	gint pos = 0;
	codecData[pos++] = (size >> 24) & 0xFF;
	codecData[pos++] = (size >> 16) & 0xFF;
	codecData[pos++] = (size >> 8) & 0xFF;
	codecData[pos++] = size & 0xFF;

	codecData[pos++] = 'a';
	codecData[pos++] = 'l';
	codecData[pos++] = 'a';
	codecData[pos++] = 'c';

	pos += 4;

	const guint32 samplesPerFrame = 352;
	codecData[pos++] = (samplesPerFrame >> 24) & 0xFF;
	codecData[pos++] = (samplesPerFrame >> 16) & 0xFF;
	codecData[pos++] = (samplesPerFrame >> 8) & 0xFF;
	codecData[pos++] = samplesPerFrame & 0xFF;

	pos += 1;

	const guint8 sampleSize = 16;
	codecData[pos++] = sampleSize;

	codecData[pos++] = 40;
	codecData[pos++] = 10;
	codecData[pos++] = 14;

	const guint8 channels = 2;
	codecData[pos++] = channels;

	const guint16 maxRun = 255;
	codecData[pos++] = (maxRun >> 8) & 0xFF;
	codecData[pos++] = maxRun & 0xFF;

	pos += 8;

	const guint32 sampleRate = 44100;
	codecData[pos++] = (sampleRate >> 24) & 0xFF;
	codecData[pos++] = (sampleRate >> 16) & 0xFF;
	codecData[pos++] = (sampleRate >> 8) & 0xFF;
	codecData[pos++] = sampleRate & 0xFF;

	GstBuffer *buffer = gst_buffer_new();
	gst_buffer_set_data(buffer, codecData, size);
	GstCaps *caps = gst_caps_new_simple("audio/x-alac", 
										"codec_data", GST_TYPE_BUFFER, buffer, 
										"rate", G_TYPE_INT, 44100, 
										"channels", G_TYPE_INT, 2, 
										NULL);
	g_object_set(G_OBJECT(priv->appsrc), "format", GST_FORMAT_BUFFERS, "do-timestamp", TRUE, "caps", caps, NULL);
	gst_caps_unref(caps);
	gst_buffer_unref(buffer);

	gst_bin_add_many(GST_BIN(priv->pipeline), priv->appsrc, priv->queue, priv->decode, priv->volume, priv->audioSink, NULL);
	gst_element_link_many(priv->appsrc, priv->queue, priv->decode, priv->volume, priv->audioSink, NULL);

	priv->bus = gst_pipeline_get_bus(GST_PIPELINE(priv->pipeline));
	priv->busWatchID = gst_bus_add_watch(priv->bus, busWatcher, self);

	g_print("AirplayMediaPlayer: INIT\n");
}

static void airplayMediaPlayerFinalize(GObject *object) {
	g_print("AirplayMediaPlayer: Finalize\n");

	AirplayMediaPlayer *self = AIRPLAY_MEDIA_PLAYER(object);
	AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);

	gst_element_set_state(priv->pipeline, GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(priv->pipeline));
	g_source_remove(priv->busWatchID);

	g_free(priv->codecData);

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

	AirplayMediaPlayer *self = AIRPLAY_MEDIA_PLAYER(object);
	AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);

	switch(propertyID) {
		case PROPERTY_RENDERER:
			priv->renderer = g_value_get_object(value);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propertyID, pspec);
			break;
	}
}

void airplayMediaPlayerConnect(AirplayMediaPlayer *self, gchar *signal, GCallback callback, gpointer userData) {
	AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);
	
	g_signal_connect(priv->pipeline, signal, callback, userData);
}

void airplayMediaPlayerSetURI(AirplayMediaPlayer *self, gchar *uri) {
	g_print("AirplayMediaPlayer: SetURI\n");

	AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);

	g_object_set(G_OBJECT(priv->pipeline), "uri", uri, NULL);
}

void airplayMediaPlayerPlay(AirplayMediaPlayer *self) {
	g_print("AirplayMediaPlayer: Play\n");

	AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);
	g_signal_connect(priv->appsrc, "need-data", G_CALLBACK(airplayRendererFeedData), priv->renderer);
	gst_element_set_state(priv->pipeline, GST_STATE_PLAYING);
}

void airplayMediaPlayerPause(AirplayMediaPlayer *self) {
	g_print("AirplayMediaPlayer: Pause\n");

	AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);
	gst_element_set_state(priv->pipeline, GST_STATE_PAUSED);
}

void airplayMediaPlayerStop(AirplayMediaPlayer *self) {
	g_print("AirplayMediaPlayer: Stop\n");

	AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);
	gst_element_set_state(priv->pipeline, GST_STATE_NULL);
}

void airplayMediaPlayerMute(AirplayMediaPlayer *self, gboolean mute) {
	g_print("AirplayMediaPlayer: Mute: %d\n", mute);

	AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);

	g_object_set(G_OBJECT(priv->volume), "mute", mute, NULL);
}

void airplayMediaPlayerSetVolume(AirplayMediaPlayer *self, gdouble volume) {
	g_print("AirplayMediaPlayer: Volume: %f\n", volume);

	AirplayMediaPlayerPrivate *priv = AIRPLAY_MEDIA_PLAYER_GET_PRIVATE(self);

	g_object_set(G_OBJECT(priv->volume), "volume", volume, NULL);
}
