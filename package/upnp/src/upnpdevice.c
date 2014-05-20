#include <stdlib.h>
#include <stdio.h>

#include <uci.h>
#include <libgupnp/gupnp.h>

#include "UpnpRenderingControlService.h"
#include "UpnpConnectionManagerService.h"
#include "UpnpAVTransportService.h"
#include "UpnpMediaPlayer.h"


gint main(gint argc, gchar *argv[]) {

	g_type_init ();

	char *key = strdup(str);
	UCIPtr *ptr = (UCIPtr *)malloc(sizeof(UCIPtr));
	if (uci_lookup_ptr(ctx, ptr, key, true) != UCI_OK) return EXIT_FAIL;
	const gint port = atoi(ptr->o.v->string);

	free(key);
	free(ptr);

	GError *error = NULL;
	GUPnPContext *context = gupnp_context_new(NULL, NULL, port, &error);
	if (error) {
		g_error_free(error);

		return EXIT_FAILURE;
	}

	GUPnPRootDevice *dev = gupnp_root_device_new(context, "MediaRenderer1.xml", "/etc/upnp/");
	gupnp_root_device_set_available(dev, TRUE);

	UpnpMediaPlayer *mediaPlayer = g_object_new(UPNP_TYPE_MEDIA_PLAYER, NULL);

	UpnpRenderingControlService *renderingControlService = g_object_new(UPNP_TYPE_RENDERING_CONTROL_SERVICE, 
																		"device", dev, 
																		"mediaPlayer", mediaPlayer, NULL);
	upnpRenderingControlServiceConnect(renderingControlService);
	
	UpnpConnectionManagerService *connectionManagerService = g_object_new(UPNP_TYPE_CONNECTION_MANAGER_SERVICE, 
																		  "device", dev, 
																		  "mediaPlayer", mediaPlayer, NULL);
	upnpConnectionManagerServiceConnect(connectionManagerService);

	UpnpAVTransportService *avTransportService = g_object_new(UPNP_TYPE_AVTRANSPORT_SERVICE, 
															  "device", dev, 
															  "mediaPlayer", mediaPlayer, NULL);
	upnpAVTransportServiceConnect(avTransportService);

	GMainLoop *main_loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(main_loop);

	g_main_loop_unref(main_loop);
	g_object_unref(avTransportService);
	g_object_unref(connectionManagerService);
	g_object_unref(renderingControlService);
	g_object_unref(dev);
	g_object_unref(context);

	return EXIT_SUCCESS;
}

