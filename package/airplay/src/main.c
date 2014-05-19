#include <stdlib.h>
#include <stdio.h>

#include <glib.h>

#include "AirplayController.h"

// static gboolean foundSource(GObject *object, GObject *orig, GParamSpec *pspec, gpointer *app) {
// 	GstElement *appsrc = NULL;
// 	g_object_get(orig, pspec->name, &appsrc, NULL);
// 	g_signal_connect(appsrc, "need-data", G_CALLBACK(feedData), app);

// 	g_print("AirplayMediaPlayer: found source\n");

// 	return FALSE;
// }

// static gboolean feedData(GstElement *appsrc, guint size, gpointer *object) {
// 	AirplayRenderer *self = AIRPLAY_RENDERER(object);
// 	AirplayRendererPrivate *priv = AIRPLAY_RENDERER_GET_PRIVATE(self);

// 	g_print("AirplayMediaPlayer: feed data\n");

// 	if(g_slist_length(priv->bufferList)) {
// 		guint8 *data = g_malloc(1024);
// 		guint length = 1024;

// 		GstBuffer *buffer = gst_buffer_new_wrapped(data, length);

// 		GstFlowReturn ret;
// 		g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);

// 		gst_buffer_unref(buffer);
// 		return TRUE;
// 	}

// 	guint8 *data = g_slist_nth_data(priv->bufferList, 0);
// 	guint length = sizeof(data);

// 	GstBuffer *buffer = gst_buffer_new_wrapped(data, length);

// 	GstFlowReturn ret;
// 	g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);

// 	g_slist_remove(priv->bufferList, data);
// 	// g_free(data);
// 	gst_buffer_unref(buffer);
// 	return TRUE;
// }

#include "AirplayMediaPlayer.h"

gint main(gint argc, gchar *argv[]) {
	g_type_init();
	gst_init(&argc, &argv);

	AirplayController *controller = g_object_new(AIRPLAY_TYPE_CONTROLLER, NULL);

	// AirplayMediaPlayer *mediaPlayer = g_object_new(AIRPLAY_TYPE_MEDIA_PLAYER, NULL);
	// airplayMediaPlayerSetURI(mediaPlayer, "file:///home/pumpkin/project/9331/host/airplay/build/abc.m4a");
	// airplayMediaPlayerConnect(mediaPlayer, "deep-notify::source", foundSource, self);
	// airplayMediaPlayerPlay(mediaPlayer);

	GMainLoop *loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);

	g_object_unref(controller);
	g_main_loop_unref(loop);
	
	return EXIT_SUCCESS;
}
