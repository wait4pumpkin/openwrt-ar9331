#include <stdlib.h>
#include <stdio.h>

#include <libgupnp/gupnp.h>

#include "UpnpRenderingControlService.h"
#include "UpnpConnectionManagerService.h"
#include "UpnpAVTransportService.h"
//#include "UpnpMediaPlayer.h"


gint main(gint argc, gchar *argv[]) {
	GError *error = NULL;
	
	g_type_init ();

	GUPnPContext *context = gupnp_context_new(NULL, NULL, 25087, &error);
	if (error) {
//		g_printerr("Error creating the GUPnP context: %s\n", error->message);
		g_error_free(error);

		return EXIT_FAILURE;
	}

	GUPnPRootDevice *dev = gupnp_root_device_new(context, "MediaRenderer3.xml", "/etc/webxml/");
	gupnp_root_device_set_available(dev, TRUE);

	UpnpMediaPlayer *mediaPlayer = g_object_new(UPNP_TYPE_MEDIA_PLAYER, NULL);
	RtpClient *rtpClient = g_object_new(RTP_TYPE_CLIENT, NULL);

	UpnpRenderingControlService *renderingControlService = g_object_new(UPNP_TYPE_RENDERING_CONTROL_SERVICE, 
																		"device", dev, 
																		"mediaPlayer", mediaPlayer, 
																		"rtpClient", rtpClient, NULL);
	upnpRenderingControlServiceConnect(renderingControlService);
	
	UpnpConnectionManagerService *connectionManagerService = g_object_new(UPNP_TYPE_CONNECTION_MANAGER_SERVICE, "device", dev, "mediaPlayer", mediaPlayer, NULL);
	upnpConnectionManagerServiceConnect(connectionManagerService);
	UpnpAVTransportService *avTransportService = g_object_new(UPNP_TYPE_AVTRANSPORT_SERVICE, "device", dev, "mediaPlayer", mediaPlayer, NULL);
	upnpAVTransportServiceConnect(avTransportService);
	GMainLoop *main_loop = g_main_loop_new(NULL, FALSE);
	g_timeout_add(5000, storevolume, renderingControlService);
	g_main_loop_run(main_loop);

	g_main_loop_unref(main_loop);
	g_object_unref(renderingControlService);
	g_object_unref(connectionManagerService);
	g_object_unref(dev);
	g_object_unref(avTransportService);
	g_object_unref(context);
	g_object_unref(rtpClient);

	return EXIT_SUCCESS;
}

