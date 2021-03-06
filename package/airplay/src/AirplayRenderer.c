#include "AirplayRenderer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <gio/gio.h>

#include <gst/app/gstappsrc.h>

#include "ssl.h"
#include "AirplayMediaPlayer.h"
#include "utility.h"

G_DEFINE_TYPE(AirplayRenderer, airplayRenderer, G_TYPE_OBJECT);

static void airplayRendererFinalize(GObject *object);
static void airplayRendererGetProperty(GObject *object, guint propertyID, GValue *value, GParamSpec *pspec);
static void airplayRendererSetProperty(GObject *object, guint propertyID, const GValue *value, GParamSpec *pspec);

static gint initRTP(AirplayRenderer *self, gint *controlPort, gint *timingPort);
static gboolean dataRead(GIOChannel *source, GIOCondition cond, gpointer userData);
static gboolean controlRead(GIOChannel *source, GIOCondition cond, gpointer userData);
static gboolean timingRead(GIOChannel *source, GIOCondition cond, gpointer userData);
static gboolean resendRequest(gpointer data);

static gboolean removeOutdated(gpointer key, gpointer value, gpointer seq);

#define AIRPLAY_RENDERER_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE((object), AIRPLAY_TYPE_RENDERER, AirplayRendererPrivate))

typedef struct _AudioPkg AudioPkg;
struct _AudioPkg {
	guint8 *data;
	gsize length;
	gushort seq;
};

typedef struct _ResendMark ResendMark;
struct _ResendMark {
	AirplayRenderer *self;
	GSocketAddress *clientAddr;
};

enum PROPERTY_RENDERER {
	PROPERTY_BASE,
	PROPERTY_FAMILY, 
	N_PROPERTIES
};

typedef struct _AirplayRendererPrivate AirplayRendererPrivate;
struct _AirplayRendererPrivate {
	gboolean isRunning;
	GSocketFamily socketFamily;
	AirplayMediaPlayer *mediaPlayer;
	GstAppSrc *appsrc;

	gchar *aesKey;
	gchar *aesIV;

	gint controlPortClient;
	gint timingPortClient;

	GSocket *dataSocket;
	GSocket *controlSocket;
	GSocket *timingSocket;

	GIOChannel *dataChannel;
	GIOChannel *controlChannel;
	GIOChannel *timingChannel;

	guint dataWatchID;
	guint controlWatchID;
	guint timingWatchID;

	gint seq;
	gint rtptime;

	GAsyncQueue *bufferList;
	GHashTable *resendTable;

	guint resendTimer;
};

static void airplayRenderer_class_init(AirplayRendererClass *klass) {
	g_type_class_add_private(klass, sizeof(AirplayRendererPrivate));

	GObjectClass *baseClass = G_OBJECT_CLASS(klass);
	baseClass->set_property = airplayRendererSetProperty;
	baseClass->get_property = airplayRendererGetProperty;
	baseClass->finalize = airplayRendererFinalize;

	GParamSpec *properties[N_PROPERTIES] = {NULL, };
	properties[PROPERTY_FAMILY] = g_param_spec_enum("socketFamily", 
													"Socket Family", 
													"Socket Family", 
													G_TYPE_SOCKET_FAMILY, 
													G_SOCKET_FAMILY_IPV4, 
                                                    G_PARAM_WRITABLE | G_PARAM_CONSTRUCT);
	g_object_class_install_properties(baseClass, N_PROPERTIES, properties);

	g_print("AirplayRenderer: Class INIT\n");
}

static void airplayRenderer_init(AirplayRenderer *self) {
	g_print("AirplayRenderer: INIT\n");

	AirplayRendererPrivate *priv = AIRPLAY_RENDERER_GET_PRIVATE(self);

	priv->isRunning = TRUE;
	priv->socketFamily = G_SOCKET_FAMILY_IPV4;
	priv->mediaPlayer = g_object_new(AIRPLAY_TYPE_MEDIA_PLAYER, "renderer", self, NULL);
	priv->appsrc = NULL;

	priv->aesKey = NULL;
	priv->aesIV = NULL;

	priv->controlPortClient = 0;
	priv->timingPortClient = 0;

	priv->dataSocket = NULL;
	priv->controlSocket = NULL;
	priv->timingSocket = NULL;

	priv->dataChannel = NULL;
	priv->controlChannel = NULL;
	priv->timingChannel = NULL;

	priv->dataWatchID = -1;
	priv->controlWatchID = -1;
	priv->timingWatchID = -1;

	priv->seq = -1;
	priv->rtptime = -1;

	priv->bufferList = g_async_queue_new();
	priv->resendTable = g_hash_table_new(g_int_hash, g_int_equal);

	priv->resendTimer = 0;
}

static void airplayRendererFinalize(GObject *object) {
	g_print("AirplayRenderer: Finalize\n");

	AirplayRenderer *self = AIRPLAY_RENDERER(object);
	AirplayRendererPrivate *priv = AIRPLAY_RENDERER_GET_PRIVATE(self);

	priv->isRunning = FALSE;
	g_object_unref(priv->mediaPlayer);
	
	if (priv->aesKey != NULL) {
		g_free(priv->aesKey);
		priv->aesKey = NULL;
	}
	if (priv->aesIV != NULL) {
		g_free(priv->aesIV);
		priv->aesKey = NULL;
	}

	g_object_unref(priv->dataSocket);
	g_object_unref(priv->controlSocket);
	g_object_unref(priv->timingSocket);

	g_io_channel_unref(priv->dataChannel);
	g_io_channel_unref(priv->controlChannel);
	g_io_channel_unref(priv->timingChannel);

	g_source_remove(priv->dataWatchID);
	g_source_remove(priv->controlWatchID);
	g_source_remove(priv->timingWatchID);

	g_async_queue_unref(priv->bufferList);
	g_hash_table_unref(priv->resendTable);

	if(priv->resendTimer > 0) g_source_remove(priv->resendTimer);

	G_OBJECT_CLASS(airplayRenderer_parent_class)->dispose(object);
}

static void airplayRendererGetProperty(GObject *object, guint propertyID, GValue *value, GParamSpec *pspec) {
	g_print("AirplayRenderer: Get Property\n");

	//AirplayRenderer *self = AIRPLAY_RENDERER(object);
	//AirplayRendererPrivate *priv = AIRPLAY_RENDERER_GET_PRIVATE(self);

	switch(propertyID) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propertyID, pspec);
			break;
	}
}

static void airplayRendererSetProperty(GObject *object, guint propertyID, const GValue *value, GParamSpec *pspec) {	  
	g_print("AirplayRenderer: Set Property\n");

	AirplayRenderer *self = AIRPLAY_RENDERER(object);
	AirplayRendererPrivate *priv = AIRPLAY_RENDERER_GET_PRIVATE(self);

	switch(propertyID) {
		case PROPERTY_FAMILY: 
			priv->socketFamily = g_value_get_enum(value);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propertyID, pspec);
			break;
	}
}

void airplayRendererSetAES(AirplayRenderer *self, gchar *aesKey, gint keyLength, gchar *aesIV, gint ivLength) {
	AirplayRendererPrivate *priv = AIRPLAY_RENDERER_GET_PRIVATE(self);

	priv->aesKey = g_malloc(keyLength * sizeof(gchar));
	memcpy(priv->aesKey, aesKey, keyLength * sizeof(gchar));

	priv->aesIV = g_malloc(ivLength * sizeof(gchar));
	memcpy(priv->aesIV, aesIV, ivLength * sizeof(gchar));
}

void airplayRendererRecord(gpointer *instance, gchar *buffer, gpointer userData) {
	AirplayRendererPrivate *priv = AIRPLAY_RENDERER_GET_PRIVATE(userData);

	if(!buffer) return;

	gchar **splitArr = g_strsplit(buffer, ";", 20);
	
	gint size = 0;
	gchar *value = NULL;
	gint *target = NULL;
	while(TRUE) {
		if(splitArr[size] == NULL) break;

		if(g_ascii_strncasecmp(splitArr[size], "seq", strlen("seq")) == 0) {
			target = &priv->seq;
		} else if(g_ascii_strncasecmp(splitArr[size], "rtptime", strlen("rtptime")) == 0) {
			target = &priv->rtptime;
		}

		gchar **keyValueArr = g_strsplit(splitArr[size], "=", 2);
		value = trim(keyValueArr[1]);
		*target = atoi(value);
		g_free(value);
		g_strfreev(keyValueArr);

		++size;
	}
	g_strfreev(splitArr);

	g_print("AirplayRenderer: record %d %d\n", priv->seq, priv->rtptime);
}

gint airplayRendererStart(AirplayRenderer *self, gint *controlPort, gint *timingPort) {
	AirplayRendererPrivate *priv = AIRPLAY_RENDERER_GET_PRIVATE(self);

	priv->controlPortClient = *controlPort;
	priv->timingPortClient = *timingPort;

	gint port = initRTP(self, controlPort, timingPort);

	airplayMediaPlayerPlay(priv->mediaPlayer);

	return port;
}

void airplayRendererFlush(AirplayRenderer *self) {
	AirplayRendererPrivate *priv = AIRPLAY_RENDERER_GET_PRIVATE(self);

	priv->isRunning = FALSE;
	airplayMediaPlayerStop(priv->mediaPlayer);
	g_hash_table_remove_all(priv->resendTable);
	while(g_async_queue_length(priv->bufferList) > 0) {
		g_async_queue_pop(priv->bufferList);
	}
	priv->isRunning = TRUE;
	airplayMediaPlayerPlay(priv->mediaPlayer);
}

gboolean airplayRendererFeedData(GstElement *appsrc, guint size, gpointer *object) {
	AirplayRenderer *self = AIRPLAY_RENDERER(object);
	AirplayRendererPrivate *priv = AIRPLAY_RENDERER_GET_PRIVATE(self);

g_print("AirplayMediaPlayer: feed data %d\n", priv->seq);

	AudioPkg *pkg = g_async_queue_try_pop(priv->bufferList);
	while(pkg == NULL) {
		if(!priv->isRunning) return FALSE;
		
		pkg = g_hash_table_lookup(priv->resendTable, &priv->seq);
g_print("Look up: %d %d\n", priv->seq, pkg == NULL);
		if(pkg) {
			priv->seq = (priv->seq + 1) % 65536;
			break;
		}
		
		usleep(50000);
		sched_yield();
		pkg = g_async_queue_try_pop(priv->bufferList);
g_print("Sleep: %d\n", priv->seq);
	}
	
	GstBuffer *buffer = gst_buffer_new();
	gst_buffer_set_data(buffer, pkg->data, pkg->length);
	GST_BUFFER_SIZE(buffer) = pkg->length;
	GST_BUFFER_MALLOCDATA(buffer) = pkg->data;
	GST_BUFFER_DATA(buffer) = GST_BUFFER_MALLOCDATA(buffer);

	gst_app_src_push_buffer((GstAppSrc *)appsrc, buffer);
	g_free(pkg);
	// gst_buffer_unref(buffer);
		
	return TRUE;
}

static gint initRTP(AirplayRenderer *self, gint *controlPort, gint *timingPort) {
	AirplayRendererPrivate *priv = AIRPLAY_RENDERER_GET_PRIVATE(self);

	if(priv->dataSocket) {
		g_object_unref(priv->dataSocket);
		priv->dataSocket = NULL;
	}
	if(priv->controlSocket) {
		g_object_unref(priv->controlSocket);
		priv->controlSocket = NULL;   
	}
	if(priv->timingSocket) {
		g_object_unref(priv->timingSocket);
		priv->timingSocket = NULL;
	}

	GError *error = NULL;
	struct sockaddr_in addr4;
	struct sockaddr_in6 addr6;
	gushort *sinPort = NULL;
	if(priv->socketFamily == G_SOCKET_FAMILY_IPV4) {
		memset(&addr4, 0, sizeof(addr4));
		addr4.sin_family = AF_INET;
		addr4.sin_addr.s_addr = htonl(INADDR_ANY);
		sinPort = &(addr4.sin_port);
	} else {
		memset(&addr6, 0, sizeof(addr6));
		addr6.sin6_family = AF_INET6;
		addr6.sin6_addr = in6addr_any;
		addr6.sin6_flowinfo = 0;
		sinPort = &(addr6.sin6_port);
	}

	gint port = 6000;
	GSocket *dataSocket = NULL;
	GSocket *controlSocket = NULL;
	GSocket *timingSocket = NULL;
	gint serverPort = -1;
	for(; TRUE; port += 3) {
		if(!dataSocket) {
			dataSocket = g_socket_new(priv->socketFamily, G_SOCKET_TYPE_DATAGRAM, G_SOCKET_PROTOCOL_UDP, &error);
			if(error) {
				g_printerr("AirplayRenderer: data socket create failed: %s\n", error->message);
				return 0;
			}
		}

		if(!controlSocket) {
			controlSocket = g_socket_new(priv->socketFamily, G_SOCKET_TYPE_DATAGRAM, G_SOCKET_PROTOCOL_UDP, &error);
			if(error) {
				g_printerr("AirplayRenderer: control socket create failed: %s\n", error->message);
				return 0;
			}
		}

		if(!timingSocket) {
			timingSocket = g_socket_new(priv->socketFamily, G_SOCKET_TYPE_DATAGRAM, G_SOCKET_PROTOCOL_UDP, &error);
			if(error) {
				g_printerr("AirplayRenderer: timing socket create failed: %s\n", error->message);
				return 0;
			}
		}

		*sinPort = htons(port);
		GSocketAddress *addr = NULL;
		if(priv->socketFamily == G_SOCKET_FAMILY_IPV4) {
			addr = g_socket_address_new_from_native(&addr4, sizeof(addr4));
		} else {
			addr = g_socket_address_new_from_native(&addr6, sizeof(addr6));
		}
		if(!g_socket_bind(dataSocket, addr, TRUE, &error)) {
			g_socket_close(dataSocket, &error);
			continue;
		} else {
			serverPort = port;
		}
		g_object_unref(addr);
		
		*sinPort = htons(port + 1);
		if(priv->socketFamily == G_SOCKET_FAMILY_IPV4) {
			addr = g_socket_address_new_from_native(&addr4, sizeof(addr4));
		} else {
			addr = g_socket_address_new_from_native(&addr6, sizeof(addr6));
		}
		if(!g_socket_bind(controlSocket, addr, TRUE, &error)) {
			g_socket_close(controlSocket, &error);
			continue;
		} else {
			*controlPort = port + 1;
		}
		g_object_unref(addr);

		*sinPort = htons(port + 2);
		if(priv->socketFamily == G_SOCKET_FAMILY_IPV4) {
			addr = g_socket_address_new_from_native(&addr4, sizeof(addr4));
		} else {
			addr = g_socket_address_new_from_native(&addr6, sizeof(addr6));
		}
		if(!g_socket_bind(timingSocket, addr, TRUE, &error)) {
			g_socket_close(timingSocket, &error);
			continue;
		} else {
			*timingPort = port + 2;
		}
		g_object_unref(addr);

		break;
	}

	priv->dataSocket = dataSocket;
	priv->controlSocket = controlSocket;
	priv->timingSocket = timingSocket;

	gint fd = g_socket_get_fd(dataSocket);
	gint nRcvbuf = 81920 * 4;
	setsockopt(fd, SOL_SOCKET,SO_RCVBUF, (char *)&nRcvbuf, sizeof(gint));
	// gint nNetTimeout = 5000;
	// setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout, sizeof(gint));
	priv->dataChannel = g_io_channel_unix_new(fd);
	priv->dataWatchID = g_io_add_watch(priv->dataChannel, G_IO_IN, (GIOFunc)dataRead, self);

	fd = g_socket_get_fd(controlSocket);
	priv->controlChannel = g_io_channel_unix_new(fd);
	priv->controlWatchID = g_io_add_watch(priv->controlChannel, G_IO_IN, (GIOFunc)controlRead, self);

	fd = g_socket_get_fd(timingSocket);
	priv->timingChannel = g_io_channel_unix_new(fd);
	priv->timingWatchID = g_io_add_watch(priv->timingChannel, G_IO_IN, (GIOFunc)timingRead, self);

	return serverPort;
}

G_MODULE_EXPORT gboolean
dataRead(GIOChannel *source, GIOCondition cond, gpointer userData) {
	AirplayRenderer *self = AIRPLAY_RENDERER(userData);
	AirplayRendererPrivate *priv = AIRPLAY_RENDERER_GET_PRIVATE(self);

	gssize length = -1;
	GSocketAddress *clientAddr = NULL;
	gchar buffer[1500];
	GError *error = NULL;
	length = g_socket_receive_from(priv->dataSocket, &clientAddr, buffer, sizeof(buffer), NULL, &error);
	if(length < 0) {
		g_printerr("AirplayRenderer: reading socket: NOTHING\n");
		return TRUE;
	}
	if(error) {
		g_printerr("AirplayRenderer: error reading socket: %s\n", error->message);
		return FALSE;
	}

	gint seqno = 0;
	seqno += (buffer[2] << 8) & 0xFF00;
	seqno += buffer[3] & 0xFF;
g_print("Rev: %d\n", seqno);
	if(priv->seq < 0) priv->seq = seqno;
	if(seqno - priv->seq > 200) priv->seq = (priv->seq + 200) % 65536;

	if(seqno < priv->seq && abs(seqno - priv->seq) < 65500) {
		g_print("Useless: %d %d\n", seqno, priv->seq);
		if(abs(priv->seq - seqno) > 500) priv->seq = -1;
		return TRUE;
	}

	length -= 12;
	guint8 *data = aesBlockDecrypt(priv->aesKey, priv->aesIV, buffer + 12, (gint)length);

	AudioPkg *pkg = g_malloc(sizeof(AudioPkg));
	pkg->data = data;
	pkg->length = length;
	pkg->seq = seqno;

	if(seqno == priv->seq) {
		priv->seq = (priv->seq + 1) % 65536;
		g_async_queue_push(priv->bufferList, pkg);
g_print("Push: %d\n", seqno);
	} else {
		gint *key = g_new(gint, 1);
		*key = seqno;
		g_hash_table_insert(priv->resendTable, key, pkg);
g_print("Hash: %d\n", seqno);
		if(priv->resendTimer < 0) {
			ResendMark *mark = g_malloc(sizeof(ResendMark));
			mark->self = self;
			mark->clientAddr = clientAddr;
			priv->resendTimer = g_timeout_add(100, resendRequest, mark);
		}
	}

	return TRUE;
}

G_MODULE_EXPORT gboolean
controlRead(GIOChannel *source, GIOCondition cond, gpointer userData) {
	AirplayRenderer *self = AIRPLAY_RENDERER(userData);
	AirplayRendererPrivate *priv = AIRPLAY_RENDERER_GET_PRIVATE(self);

	gssize length = -1;
	GSocketAddress *clientAddr = NULL;
	gchar buffer[2048];
	GError *error = NULL;
	length = g_socket_receive_from(priv->controlSocket, &clientAddr, buffer, sizeof(buffer), NULL, &error);
	if(length < 0) return TRUE;
	if(error) {
		g_printerr("AirplayRenderer: error reading socket: %s\n", error->message);
		return FALSE;
	}

	guint8 type = buffer[1] & ~0x80;
	if(type == 0x54) {
g_print("Control: 0x54\n");
	} else if(type == 0x56) {
		gint seqno = 0;
		seqno += (buffer[6] << 8) & 0xFF00;
		seqno += buffer[7] & 0xFF;

		if(seqno < priv->seq  && abs(seqno - priv->seq) < 65500) {
			return TRUE;
		}

		length -= 16;
		guint8 *data = aesBlockDecrypt(priv->aesKey, priv->aesIV, buffer + 16, (gint)length);

		AudioPkg *pkg = g_malloc(sizeof(AudioPkg));
		pkg->data = data;
		pkg->length = length;
		pkg->seq = seqno;

		if(seqno == priv->seq) {
			priv->seq = (priv->seq + 1) % 65536;
			g_async_queue_push(priv->bufferList, pkg);
		} else {
			gint *key = g_new(gint, 1);
			*key = seqno;
			g_hash_table_insert(priv->resendTable, key, pkg);
g_print("Hash: %d\n", seqno);
		}
g_print("Control: %d %d\n", seqno, priv->seq);
	} else {
		g_print("Control: Unknown\n");
	}

	return TRUE;
}

G_MODULE_EXPORT gboolean
timingRead(GIOChannel *source, GIOCondition cond, gpointer userData) {
	AirplayRenderer *self = AIRPLAY_RENDERER(userData);
	AirplayRendererPrivate *priv = AIRPLAY_RENDERER_GET_PRIVATE(self);

	gssize length = -1;
	GSocketAddress *clientAddr = NULL;
	gchar buffer[512];
	GError *error = NULL;
	length = g_socket_receive_from(priv->timingSocket, &clientAddr, buffer, sizeof(buffer), NULL, &error);
	if(length < 0) return TRUE;
	if(error) {
		g_printerr("AirplayRenderer: error reading socket: %s\n", error->message);
		return FALSE;
	}

	g_print("Timing: %d\n", (gint)length);

	return TRUE;
}

// static void resendRequest(AirplayRenderer *self, GSocketAddress *addr, gint seqNumStart, gint seqNumEnd) {
// 	AirplayRendererPrivate *priv = AIRPLAY_RENDERER_GET_PRIVATE(self);

// gint oldStart = seqNumStart;
// gint oldEnd = seqNumEnd;

// 	if(seqNumEnd < priv->seq) return;
// 	if(seqNumStart < priv->seq) seqNumStart = priv->seq;

// 	gint i = (seqNumStart + 1) % 65536;
// 	for(; i<seqNumEnd; i=++i%65536) {
// 		AudioPkg *pkg = g_hash_table_lookup(priv->resendTable, &i);
// 		if(pkg != NULL) {
// 			seqNumEnd = i - 1;
// 			break;
// 		}
// 	}

// 	if(seqNumStart > seqNumEnd && abs(seqNumStart - seqNumEnd) < 65500) return;

// 	gchar request[8];
// 	request[0] = 0x80;
// 	request[1] = 0x55 | 0x80;
// 	*(gushort *)(request + 2) = htons(1);
// 	*(gushort *)(request + 4) = htons(seqNumStart);
// 	*(gushort *)(request + 6) = htons(seqNumEnd - seqNumStart + 1);

// 	GError *error = NULL;
// 	g_socket_send_to(priv->controlSocket, 
// 					 g_inet_socket_address_new(g_inet_socket_address_get_address((GInetSocketAddress *)addr), priv->controlPortClient), 
// 					 request, 
// 					 sizeof(request), 
// 					 NULL, &error);
// 	if(error) {
// 		g_printerr("AirplayRenderer: resendRequest failed: %s\n", error->message);
// 	}

// g_print("Resend Quest : %d %d %d %d\n", seqNumStart, seqNumEnd, oldStart, oldEnd);
// }

static gboolean removeOutdated(gpointer key, gpointer value, gpointer seq) {
	if(*(gint *)key < *(gint *)seq && abs(*(gint *)key - *(gint *)seq) < 65500) return TRUE;
	return FALSE;
}

/*
static gint intCompareFunc(gconstpointer a, gconstpointer b) {
	const gint aValue = *(gint *)a;
	const gint bValue = *(gint *)b;
	if(aValue < bValue) return -1;
	if(aValue == bValue) return 0;
	return 1;
}
*/

void airplayRendererMute(AirplayRenderer *self, gboolean mute) {
	AirplayRendererPrivate *priv = AIRPLAY_RENDERER_GET_PRIVATE(self);

	airplayMediaPlayerMute(priv->mediaPlayer, mute);
}

void airplayRendererSetVolume(AirplayRenderer *self, gdouble volume) {
	AirplayRendererPrivate *priv = AIRPLAY_RENDERER_GET_PRIVATE(self);

	airplayMediaPlayerSetVolume(priv->mediaPlayer, volume);
}

static gboolean resendRequest(gpointer data) {
	ResendMark *mark = (ResendMark *)data;
	AirplayRendererPrivate *priv = AIRPLAY_RENDERER_GET_PRIVATE(mark->self);

	guint nRemove = g_hash_table_foreach_remove(priv->resendTable, removeOutdated, &priv->seq);
	g_print("Resend Table Removed: %d\n", nRemove);

	GList *keys = g_hash_table_get_keys(priv->resendTable);
	guint length = g_list_length(keys);
	if(length <= 2) {
		goto free;
	}

	int i = 1;
	for(; i<length-1; ++i) {
		gint before = *(gint *)g_list_nth_data(keys, i-1);
		gint idx = *(gint *)g_list_nth_data(keys, i);

		if(idx != before + 1) {
			gint seqNumStart = before + 1;
			gint seqNumEnd = idx - 1;

			gchar request[8];
			request[0] = 0x80;
			request[1] = 0x55 | 0x80;
			*(gushort *)(request + 2) = htons(1);
			*(gushort *)(request + 4) = htons(seqNumStart);
			*(gushort *)(request + 6) = htons(seqNumEnd - seqNumStart + 1);

			GError *error = NULL;
			g_socket_send_to(priv->controlSocket, 
							 g_inet_socket_address_new(g_inet_socket_address_get_address((GInetSocketAddress *)mark->clientAddr), priv->controlPortClient), 
							 request, 
							 sizeof(request), 
							 NULL, &error);
			if(error) {
				g_printerr("AirplayRenderer: resendRequest failed: %s\n", error->message);
			}

g_print("Resend Quest : %d %d\n", seqNumStart, seqNumEnd);
		}
	}

free: 
	g_list_free(keys);
	g_free(mark);
	g_source_remove(priv->resendTimer);
	priv->resendTimer = 0;

	return FALSE;
}

