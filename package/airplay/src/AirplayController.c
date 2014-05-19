#include "AirplayController.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gio/gio.h>

#include "AirplayRenderer.h"
#include "utility.h"
#include "ssl.h"

G_DEFINE_TYPE(AirplayController, airplayController, G_TYPE_OBJECT);

static void airplayControllerDefaultHandler(gpointer instance, const gchar *buffer, gpointer userData);

static void airplayControllerFinalize(GObject *object);
static void airplayControllerGetProperty(GObject *object, guint propertyID, GValue *value, GParamSpec *pspec);
static void airplayControllerSetProperty(GObject *object, guint propertyID, const GValue *value, GParamSpec *pspec);

static gboolean incomingWatch(GSocketService *service, GSocketConnection *connection, GObject *sourceObject, gpointer userData);
static gboolean clientRead(GIOChannel *source, GIOCondition cond, gpointer userData);

static GString* handleMsg(AirplayController *self, gchar *header, gchar *content);
static void writeAppleResponse(AirplayController *self, gchar *header, GString *response);
static void writeCSeqResponse(AirplayController *self, gchar *header, GString *response);

static gchar* getContentLength(AirplayController *self, gchar *header);
static gchar* getCSeq(AirplayController *self, gchar *header);
static gchar* getAppleChallenge(AirplayController *self, gchar *header);
//static gchar* getTransport(AirplayController *self, gchar *header);
static gchar* getRtpInfo(AirplayController *self, gchar *header);
static gint getControlPort(AirplayController *self, gchar *header);
static gint getTimingPort(AirplayController *self, gchar *header);

static gchar* getAESIV(AirplayController *self, gchar *content);
static gchar* getAESKey(AirplayController *self, gchar *content);
static gchar* getFormat(AirplayController *self, gchar *content);
static gchar* getVolume(AirplayController *self, gchar *content);

static gchar* getArgument(gchar *header, gchar *key, gchar *arg);
static gchar* getHeader(gchar *header, gchar *key);

static gint startAvahi(AirplayController *self);

#define AIRPLAY_CONTROLLER_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE((object), AIRPLAY_TYPE_CONTROLLER, AirplayControllerPrivate))

enum PROPERTY_CONTROLLER {
	PROPERTY_BASE,
	N_PROPERTIES
};

typedef struct _AirplayControllerPrivate AirplayControllerPrivate;
struct _AirplayControllerPrivate {
	AirplayRenderer *renderer;
	GSocketFamily socketFamily;

	gint HWID_SIZE;
	gchar *hwID;
	gchar *hwIDHex;
	gchar *SERVER_NAME;
	gint SERVER_PORT;

	GString *buffer;

	GSocketService *socketService;
	gchar *ipBin;
	gint ipBinLen;

	gchar *HEADER_CONTENT_LENGTH;
	gchar *HEADER_CSEQ;
	gchar *HEADER_CHALLENGE;
	gchar *HEADER_TRANSPORT;
	gchar *HEADER_RTP_INFO;

	gchar *ARG_CONTROL_PORT;
	gchar *ARG_TIMING_PORT;
	gchar *ARG_SEQ;
	gchar *ARG_RTPTIME;

	gchar *CONTENT_AESIV;
	gchar *CONTENT_AESKEY;
	gchar *CONTENT_FORMAT;
	gchar *CONTENT_VOLUME;

	gchar *CMD_OPTIONS;
	gchar *CMD_ANNOUNCE;
	gchar *CMD_SETUP;
	gchar *CMD_RECORD;
	gchar *CMD_TEARDOWN;
	gchar *CMD_FLUSH;
	gchar *CMD_SET_PARAMETER;

	gchar *aesKey;
	gint keyLength;	
	gchar *aesIV;
	gint aesIVLength;
	gchar *format;
};

typedef struct _ChannelUserData ChannelUserData;
struct _ChannelUserData {
	AirplayController *self;
	GSocketConnection *connection;
};

static void airplayControllerDefaultHandler(gpointer instance, const gchar *buffer, gpointer userData) {
	g_print("AirplayController: default handler\n");
}

static void airplayController_class_init(AirplayControllerClass *klass) {
	g_type_class_add_private(klass, sizeof(AirplayControllerPrivate));

	klass->defaultHandler = airplayControllerDefaultHandler;
	g_signal_new("record", 
				 AIRPLAY_TYPE_CONTROLLER, 
				 G_SIGNAL_RUN_FIRST, 
				 G_STRUCT_OFFSET(AirplayControllerClass, defaultHandler), 
				 NULL, 
				 NULL, 
				 g_cclosure_marshal_VOID__STRING, 
				 G_TYPE_NONE, 
				 1, 
				 G_TYPE_STRING);

	GObjectClass *baseClass = G_OBJECT_CLASS(klass);
	baseClass->set_property = airplayControllerSetProperty;
	baseClass->get_property = airplayControllerGetProperty;
	baseClass->finalize = airplayControllerFinalize;

	//GParamSpec *properties[N_PROPERTIES] = {NULL, };

	g_print("AirplayController: Class INIT\n");
}

static void airplayController_init(AirplayController *self) {
	g_print("AirplayController: INIT\n");

	AirplayControllerPrivate *priv = AIRPLAY_CONTROLLER_GET_PRIVATE(self);

	priv->socketFamily = G_SOCKET_FAMILY_IPV4;
	priv->renderer = NULL;

	priv->HWID_SIZE = 6;
	priv->SERVER_NAME = "Haiya-Music";
	priv->SERVER_PORT = 5003;

	priv->hwID = g_malloc(priv->HWID_SIZE * sizeof(gchar));
	priv->hwIDHex = g_malloc((priv->HWID_SIZE * 2 + 1) * sizeof(gchar));

	priv->hwID[0] = 0;
	priv->hwID[1] = 55;
	priv->hwID[2] = 54;
	priv->hwID[3] = 53;
	priv->hwID[4] = 52;
	priv->hwID[5] = 51;

	srandom(time(NULL));
	gint i = 0;
	for(; i<priv->HWID_SIZE; ++i) {
		if(i > 0) {
			// priv->hwID[i] = ((random() % 80) + 33);
		}
		g_snprintf(priv->hwIDHex + i * 2, 3, "%02X", priv->hwID[i]);
	}

	priv->buffer = g_string_new(NULL);

	priv->HEADER_CONTENT_LENGTH = "Content-Length";
	priv->HEADER_CSEQ = "CSeq";
	priv->HEADER_CHALLENGE = "Apple-Challenge";
	priv->HEADER_TRANSPORT = "Transport";
	priv->HEADER_RTP_INFO = "RTP-Info";

	priv->ARG_CONTROL_PORT = "control_port";
	priv->ARG_TIMING_PORT = "timing_port";
	priv->ARG_SEQ = "seq";
	priv->ARG_RTPTIME = "rtptime";

	priv->CONTENT_AESIV = "a=aesiv";
	priv->CONTENT_AESKEY = "a=rsaaeskey";
	priv->CONTENT_FORMAT = "a=fmtp";
	priv->CONTENT_VOLUME = "volume";

	priv->CMD_OPTIONS = "OPTIONS";
	priv->CMD_ANNOUNCE = "ANNOUNCE";
	priv->CMD_SETUP = "SETUP";
	priv->CMD_RECORD = "RECORD";
	priv->CMD_TEARDOWN = "TEARDOWN";
	priv->CMD_FLUSH = "FLUSH";
	priv->CMD_SET_PARAMETER = "SET_PARAMETER";

	priv->aesKey = NULL;
	priv->keyLength = 0;
	priv->aesIV = NULL;
	priv->aesIVLength = 0;
	priv->format = NULL;

	startAvahi(self);

	GError *error = NULL;
	priv->socketService = g_socket_service_new();
	g_socket_listener_add_inet_port((GSocketListener *)(priv->socketService), priv->SERVER_PORT, NULL, &error);
	if(error) {
		g_printerr("Airplay Server Listen Failed: %s", error->message);
	}
	g_signal_connect(priv->socketService, "incoming", G_CALLBACK(incomingWatch), self);
	g_socket_service_start(priv->socketService);
}

static void airplayControllerFinalize(GObject *object) {
	g_print("AirplayController: Finalize\n");

	AirplayController *self = AIRPLAY_CONTROLLER(object);
	AirplayControllerPrivate *priv = AIRPLAY_CONTROLLER_GET_PRIVATE(self);

	g_object_unref(priv->renderer);

	g_free(priv->hwID);
	g_free(priv->hwIDHex);
	g_free(priv->ipBin);
	
	g_string_free(priv->buffer, TRUE);
	g_object_unref(priv->socketService);

	if(priv->aesIV != NULL) {
		g_free(priv->aesIV);
	}	
	if(priv->aesKey != NULL) {
		g_free(priv->aesKey);
	}
	if(priv->format != NULL) {
		g_free(priv->format);
	}

	G_OBJECT_CLASS(airplayController_parent_class)->dispose(object);
}

static void airplayControllerGetProperty(GObject *object, guint propertyID, GValue *value, GParamSpec *pspec) {
	g_print("AirplayController: Get Property\n");

	//AirplayController *self = AIRPLAY_CONTROLLER(object);
	//AirplayControllerPrivate *priv = AIRPLAY_CONTROLLER_GET_PRIVATE(self);

	switch(propertyID) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propertyID, pspec);
			break;
	}
}

static void airplayControllerSetProperty(GObject *object, guint propertyID, const GValue *value, GParamSpec *pspec) {      
	g_print("AirplayController: Set Property\n");

	//AirplayController *self = AIRPLAY_CONTROLLER(object);
	//AirplayControllerPrivate *priv = AIRPLAY_CONTROLLER_GET_PRIVATE(self);

	switch(propertyID) {

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propertyID, pspec);
			break;
	}
}

G_MODULE_EXPORT gboolean 
incomingWatch(GSocketService *service, GSocketConnection *connection, GObject *sourceObject, gpointer userData) {
	g_print("Airplay: Received Connection\n");
	
	AirplayController *self = userData;
	AirplayControllerPrivate *priv = AIRPLAY_CONTROLLER_GET_PRIVATE(self);

	GSocket *socket = g_socket_connection_get_socket(connection);
	gint fd = g_socket_get_fd(socket);

	GError *error = NULL;
	GSocketAddress *socketAddr = g_socket_get_local_address(socket, &error);
	if(error) {
		g_print("Soket Get Local Addr Failed: %s\n", error->message);
		return FALSE;
	}
	
	priv->socketFamily = g_socket_address_get_family(socketAddr);
	if(priv->socketFamily == G_SOCKET_FAMILY_IPV4) {
		priv->ipBinLen = 4;
	} else if(priv->socketFamily == G_SOCKET_FAMILY_IPV6) {
		priv->ipBinLen = 16;
	} else {
		return FALSE;
	}
	GInetSocketAddress *addr = (GInetSocketAddress *)socketAddr;
	const guint8 *addrByte = g_inet_address_to_bytes(g_inet_socket_address_get_address(addr));
	if(priv->ipBin != NULL) g_free(priv->ipBin);
	priv->ipBin = g_malloc(priv->ipBinLen * sizeof(gchar));
	gint i = 0;
	for(; i<priv->ipBinLen; ++i) {
		g_print("%02x\n", addrByte[i]);
	}
	memcpy(priv->ipBin, addrByte, priv->ipBinLen);
	
	GIOChannel *channel = g_io_channel_unix_new(fd);
	ChannelUserData *data = g_malloc(sizeof(ChannelUserData));
	data->self = self;
	data->connection = connection;
	g_io_add_watch(channel, G_IO_IN, (GIOFunc)clientRead, data);

	g_print("\n");

	g_io_channel_unref(channel);
	g_object_unref(socketAddr);
	g_object_ref(connection);
	return TRUE;
}

G_MODULE_EXPORT gboolean
clientRead(GIOChannel *source, GIOCondition cond, gpointer userData) {
	ChannelUserData *data = (ChannelUserData *)userData;
	AirplayController *self = data->self;
	AirplayControllerPrivate *priv = AIRPLAY_CONTROLLER_GET_PRIVATE(self);

	GString *buffer = g_string_new("");
	GError *error = NULL;
	gchar *splitTag = "\r\n\r\n";
	while(TRUE) {
		GIOStatus ret = g_io_channel_read_line_string(source, buffer, NULL, &error);
		
		if(ret == G_IO_STATUS_ERROR) {
			g_error("Error reading: %s\n", error->message);
			goto fail;
		} else if(ret == G_IO_STATUS_AGAIN) {
			g_string_free(buffer, TRUE);
			return TRUE;
		} else if(ret == G_IO_STATUS_EOF) {
			g_print("EOF\n");
			goto fail;
		} else {
			g_string_append(priv->buffer, buffer->str);
			gchar *res = g_strstr_len(priv->buffer->str, priv->buffer->len, splitTag);
			if(res == NULL) continue;

			gint headerLength = res - priv->buffer->str + strlen(splitTag);
			gchar *msgHeader = g_malloc(headerLength);
			gchar *msgContent = NULL;
			g_strlcpy(msgHeader, priv->buffer->str, headerLength);

			gchar *contentLengthStr = getContentLength(self, msgHeader);
			if(contentLengthStr) {
				gint contentLength = atoi(contentLengthStr);
				if(strlen(priv->buffer->str) - strlen(msgHeader) < contentLength) {
					continue;
				} else {
					msgContent = g_malloc(contentLength + 1);
					g_strlcpy(msgContent, priv->buffer->str + headerLength, contentLength);
					g_string_erase(priv->buffer, 0, headerLength + contentLength);
				}
			} else {
				g_string_erase(priv->buffer, 0, headerLength);
			}
			g_free(contentLengthStr);

			GString *response = handleMsg(self, msgHeader, msgContent);
			if(response) {
				g_print("%s\n%s\n\n", msgHeader, msgContent);

				gsize byteWritten = 0;
				gsize byteRemain = response->len;
				gint pos = 0;
				while(byteRemain > 0) {
					g_io_channel_write_chars(source, response->str + pos, byteRemain, &byteWritten, &error);
					if(error) {
						g_printerr("Response Error: %s\n", error->message);
						goto fail;
					} else if(byteWritten != response->len) {
						byteRemain -= byteWritten;
						pos += byteWritten;
					} else {
						break;
					}
				}

				g_io_channel_flush(source, &error);
				if(error) {
					g_printerr("Response Error: %s\n", error->message);
					goto fail;
				}
				g_print("%s\n\n", response->str);
				g_print("\n");
			}
			g_free(msgHeader);
			if(!msgContent) {
				g_free(msgContent);
			}
			g_string_free(response, TRUE);
		}
	}

fail: 
	g_object_unref(data->connection);
	g_free(data);
	g_string_free(buffer, TRUE);
	return FALSE;
}

static GString* handleMsg(AirplayController *self, gchar *header, gchar *content) {
	AirplayControllerPrivate *priv = AIRPLAY_CONTROLLER_GET_PRIVATE(self);

	GString *writeBuffer = g_string_new("RTSP/1.0 200 OK\r\n");
	writeAppleResponse(self, header, writeBuffer);
	if(g_ascii_strncasecmp(header, priv->CMD_OPTIONS, strlen(priv->CMD_OPTIONS)) == 0) {
		writeCSeqResponse(self, header, writeBuffer);
		g_string_append(writeBuffer, "Public: ANNOUNCE, SETUP, RECORD, PAUSE, FLUSH, TEARDOWN, OPTIONS, GET_PARAMETER, SET_PARAMETER\r\n");
	} else if(g_ascii_strncasecmp(header, priv->CMD_ANNOUNCE, strlen(priv->CMD_ANNOUNCE)) == 0) {
		gchar *aesIVStr = getAESIV(self, content);
		if(aesIVStr != NULL) {
			gchar *aesIVStrTrimed = trim(aesIVStr);
			g_free(aesIVStr);
			priv->aesIVLength = 0;
			gchar *aesIV =  decodeBase64(aesIVStrTrimed, strlen(aesIVStrTrimed), &priv->aesIVLength);
			g_free(aesIVStrTrimed);

			gchar *aesKey = getAESKey(self, content);
			gchar *aesKeyTrimed = trim(aesKey);
			g_free(aesKey);
			gint aesKeyLength = 0;
			aesKey = decodeBase64(aesKeyTrimed, strlen(aesKeyTrimed), &aesKeyLength);
			g_free(aesKeyTrimed);

			gchar *formatStr = getFormat(self, content);
			gchar *format = trim(formatStr);
			g_free(formatStr);

			priv->keyLength = 0;
			gchar *decryptKey = rsaDecrypt(aesKey, aesKeyLength, &priv->keyLength);
			g_free(aesKey);

			if(priv->aesIV != NULL) {
				g_free(priv->aesIV);
			}
			if(priv->aesKey != NULL) {
				g_free(priv->aesKey);
			}
			if(priv->format != NULL) {
				g_free(priv->format);
			}
			priv->aesKey = decryptKey;
			priv->aesIV = aesIV;
			priv->format = format;

			writeCSeqResponse(self, header, writeBuffer);
		}
	} else if(g_ascii_strncasecmp(header, priv->CMD_SETUP, strlen(priv->CMD_SETUP)) == 0) {
		if(priv->renderer != NULL) {
			g_object_unref(priv->renderer);
		}
		priv->renderer = g_object_new(AIRPLAY_TYPE_RENDERER, "socketFamily", priv->socketFamily, NULL);
		airplayRendererSetAES(priv->renderer, priv->aesKey, priv->keyLength, priv->aesIV, priv->aesIVLength);

		gint controlPort = getControlPort(self, header);
		gint timingPort = getTimingPort(self, header);
		gint serverPort = airplayRendererStart(priv->renderer, &controlPort, &timingPort);

		g_signal_connect(self, "record", G_CALLBACK(airplayRendererRecord), priv->renderer);

		g_string_append(writeBuffer, "Transport: RTP/AVP/UDP;unicast;mode=record;");
		gchar portStr[100];
		sprintf(portStr, "control_port=%d;timing_port=%d;server_port=%d", controlPort, timingPort, serverPort);
		g_string_append(writeBuffer, portStr);
		g_string_append(writeBuffer, "\r\nSession: TIYAANTING\r\n");

	} else if(g_ascii_strncasecmp(header, priv->CMD_RECORD, strlen(priv->CMD_RECORD)) == 0) {
		gchar *info = getRtpInfo(self, header);
		g_signal_emit_by_name(self, "record", info, G_TYPE_NONE);
		g_free(info);

		writeCSeqResponse(self, header, writeBuffer);
	} else if(g_ascii_strncasecmp(header, priv->CMD_TEARDOWN, strlen(priv->CMD_TEARDOWN)) == 0) {
		g_string_append(writeBuffer, "Connection: close\r\n");

		g_object_unref(priv->renderer);
		priv->renderer = NULL;

		writeCSeqResponse(self, header, writeBuffer);
	} else if(g_ascii_strncasecmp(header, priv->CMD_FLUSH, strlen(priv->CMD_FLUSH)) == 0) {
		airplayRendererFlush(priv->renderer);

		writeCSeqResponse(self, header, writeBuffer);
	} else if(g_ascii_strncasecmp(header, priv->CMD_SET_PARAMETER, strlen(priv->CMD_SET_PARAMETER)) == 0) {
		writeCSeqResponse(self, header, writeBuffer);
		gchar *value = getVolume(self, content);
		gdouble volume = atof(value);

		if(volume < -30) {
			airplayRendererMute(priv->renderer, TRUE);	
		} else {
			airplayRendererMute(priv->renderer, FALSE);
			airplayRendererSetVolume(priv->renderer, (volume + 30) / 30.);
		}

		g_free(value);
	} else {
		writeCSeqResponse(self, header, writeBuffer);
	}

	g_string_append(writeBuffer, "\r\n");
	return writeBuffer;
}

static void writeAppleResponse(AirplayController *self, gchar *header, GString *response) {
	AirplayControllerPrivate *priv = AIRPLAY_CONTROLLER_GET_PRIVATE(self);

	gchar *challenge = getAppleChallenge(self, header);
	if(!challenge) return;

	gint decodedDataLength = 0;
	gchar *decodedData = decodeBase64(challenge, strlen(challenge), &decodedDataLength);

	gint pos = 0;
	guchar resp[38];
	memset(resp, 0, sizeof(resp));

	memcpy(resp, decodedData, decodedDataLength);
	pos += decodedDataLength;
	memcpy(resp + pos, priv->ipBin, priv->ipBinLen);
	pos += priv->ipBinLen;
	memcpy(resp + pos, priv->hwID, priv->HWID_SIZE);
	pos += priv->HWID_SIZE;

	gint pad = 32 - pos;
	if(pad > 0) {
		memset(resp + pos, 0, pad);
		pos += pad;
	}
	gint i = 0;
	for(; i<pos; ++i) {
		g_print("%02x ", resp[i]);
	}

	gchar *rsaStr = rsaEncrypt((gchar *)resp, sizeof(resp), &pos);
    gchar *base64Str = encodeBase64(rsaStr, pos);

	gint len = strlen(base64Str);
	while(len > 1 && base64Str[len-1] == '=') {
		base64Str[len-1] = '\0';
	}

	g_string_append(response, "Apple-Response: ");
	g_string_append(response, base64Str);
	g_string_append(response, "\r\n");

	g_free(decodedData);
	g_free(base64Str);
	g_free(rsaStr);
	// g_free(challenge);
}

static void writeCSeqResponse(AirplayController *self, gchar *header, GString *response) {
	//AirplayControllerPrivate *priv = AIRPLAY_CONTROLLER_GET_PRIVATE(self);

	gchar *cseq = getCSeq(self, header);
	g_string_append(response, "Audio-Jack-Status: connected; type=analog\r\n");
	g_string_append(response, "CSeq: ");
	g_string_append(response, cseq);
	g_string_append(response, "\r\n");
	g_free(cseq);
}

static gchar* getContentLength(AirplayController *self, gchar *header) {
	AirplayControllerPrivate *priv = AIRPLAY_CONTROLLER_GET_PRIVATE(self);
	return getHeader(header, priv->HEADER_CONTENT_LENGTH);
}

static gchar* getCSeq(AirplayController *self, gchar *header) {
	AirplayControllerPrivate *priv = AIRPLAY_CONTROLLER_GET_PRIVATE(self);
	return getHeader(header, priv->HEADER_CSEQ);
}

static gchar* getAppleChallenge(AirplayController *self, gchar *header) {
	AirplayControllerPrivate *priv = AIRPLAY_CONTROLLER_GET_PRIVATE(self);
	return getHeader(header, priv->HEADER_CHALLENGE);
}
/*
static gchar* getTransport(AirplayController *self, gchar *header) {
	AirplayControllerPrivate *priv = AIRPLAY_CONTROLLER_GET_PRIVATE(self);
	return getHeader(header, priv->HEADER_TRANSPORT);
}
*/
static gchar* getRtpInfo(AirplayController *self, gchar *header) {
	AirplayControllerPrivate *priv = AIRPLAY_CONTROLLER_GET_PRIVATE(self);
	return getHeader(header, priv->HEADER_RTP_INFO);	
}

static gint getControlPort(AirplayController *self, gchar *header) {
	AirplayControllerPrivate *priv = AIRPLAY_CONTROLLER_GET_PRIVATE(self);

	gchar *value = getArgument(header, priv->HEADER_TRANSPORT, priv->ARG_CONTROL_PORT);
	gint port = atoi(value);
	g_free(value);

	return port;
}

static gint getTimingPort(AirplayController *self, gchar *header) {
	AirplayControllerPrivate *priv = AIRPLAY_CONTROLLER_GET_PRIVATE(self);

	gchar *value = getArgument(header, priv->HEADER_TRANSPORT, priv->ARG_TIMING_PORT);
	gint port = atoi(value);
	g_free(value);

	return port;
}

static gchar* getAESIV(AirplayController *self, gchar *content) {
	AirplayControllerPrivate *priv = AIRPLAY_CONTROLLER_GET_PRIVATE(self);
	return getHeader(content, priv->CONTENT_AESIV);
}

static gchar* getAESKey(AirplayController *self, gchar *content) {
	AirplayControllerPrivate *priv = AIRPLAY_CONTROLLER_GET_PRIVATE(self);
	return getHeader(content, priv->CONTENT_AESKEY);
}

static gchar* getFormat(AirplayController *self, gchar *content) {
	AirplayControllerPrivate *priv = AIRPLAY_CONTROLLER_GET_PRIVATE(self);
	return getHeader(content, priv->CONTENT_FORMAT);
}

static gchar* getVolume(AirplayController *self, gchar *content) {
	AirplayControllerPrivate *priv = AIRPLAY_CONTROLLER_GET_PRIVATE(self);
	return getHeader(content, priv->CONTENT_VOLUME);
}

static gchar* getHeader(gchar *header, gchar *key) {
	gchar **splitArr = g_strsplit(header, "\r\n", 20);
	gint size = 0;
	gchar *value = NULL;
	
	gchar *splitTag = ":";
	while(TRUE) {
		if(splitArr[size] == NULL) break;

		if(g_ascii_strncasecmp(splitArr[size], key, strlen(key)) == 0) {
			gchar **keyValueArr = g_strsplit(splitArr[size], splitTag, 2);
			value = trim(keyValueArr[1]);
			g_strfreev(keyValueArr);

			break;
		}
		++size;
	}

	g_strfreev(splitArr);
	return value;
}

static gchar* getArgument(gchar *header, gchar *key, gchar *arg) {
	gchar *line = getHeader(header, key);
	gchar **splitArr = g_strsplit(line, ";", 20);
	
	gint size = 0;
	gchar *value = NULL;
	while(TRUE) {
		if(splitArr[size] == NULL) break;

		if(g_ascii_strncasecmp(splitArr[size], arg, strlen(arg)) == 0) {
			gchar **keyValueArr = g_strsplit(splitArr[size], "=", 2);
			value = trim(keyValueArr[1]);
			g_strfreev(keyValueArr);

			break;
		}
		++size;
	}

	g_strfreev(splitArr);
	g_free(line);
	return value;
}

static gint startAvahi(AirplayController *self) {
	AirplayControllerPrivate *priv = AIRPLAY_CONTROLLER_GET_PRIVATE(self);

	gint pid = fork();
	if(pid == 0) {
		gchar *name = g_strconcat(priv->hwIDHex, "@", priv->SERVER_NAME, NULL);
		gchar port[10];
		g_snprintf(port, 10, "%d", priv->SERVER_PORT);

		g_print("Avahi: %s %s\n", name, port);

		execlp("avahi-publish-service", "avahi-publish-service", name, 
			   "_raop._tcp", port, "tp=UDP","sm=false","sv=false","ek=1","et=0,1",
			   "cn=0,1","ch=2","ss=16","sr=44100","pw=false","vn=3","txtvers=1", NULL);
		execlp("dns-sd", "dns-sd", "-R", name,
			   "_raop._tcp", ".", port, "tp=UDP","sm=false","sv=false","ek=1","et=0,1",
			   "cn=0,1","ch=2","ss=16","sr=44100","pw=false","vn=3","txtvers=1", NULL);
		execlp("mDNSPublish", "mDNSPublish", name,
		     "_raop._tcp", port, "tp=UDP","sm=false","sv=false","ek=1","et=0,1",
		     "cn=0,1","ch=2","ss=16","sr=44100","pw=false","vn=3","txtvers=1", NULL);
		
		g_printerr("Airplay: Failed to run Avahi\n");
		exit(EXIT_FAILURE);
	} else {
		g_print("Avahi started on PID: %d\n", pid);
	}
	
	return pid;
}
