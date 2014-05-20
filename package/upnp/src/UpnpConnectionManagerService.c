#include "UpnpConnectionManagerService.h"

#include "UpnpMediaPlayer.h"

G_DEFINE_TYPE(UpnpConnectionManagerService, upnpConnectionManagerService, G_TYPE_OBJECT);

static void upnpConnectionManagerServiceDispose(GObject *object);
static void upnpConnectionManagerServiceGetProperty(GObject *object, guint propertyID, GValue *value, GParamSpec *pspec);
static void upnpConnectionManagerServiceSetProperty(GObject *object, guint propertyID, const GValue *value, GParamSpec *pspec);

/* action connect function */
static void getProtocolInfo(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
static void getCurrentConnectionIDs(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
static void getCurrentConnectionInfo(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);

/* query connect function */
static void querySourceProtocolInfo(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void querySinkProtocolInfo(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryCurrentConnectionIDs(GUPnPService *service, char *variable, GValue *value, gpointer userData);
//static void queryFeatureList(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryClockUpdateID(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryDeviceClockInfoUpdates(GUPnPService *service, char *variable, GValue *value, gpointer userData);
/* A_ARG_TYPE_ */static void queryConnectionStatus(GUPnPService *service, char *variable, GValue *value, gpointer userData);
/* A_ARG_TYPE_ */static void queryConnectionManager(GUPnPService *service, char *variable, GValue *value, gpointer userData);
/* A_ARG_TYPE_ */static void queryDirection(GUPnPService *service, char *variable, GValue *value, gpointer userData);
/* A_ARG_TYPE_ */static void queryProtocolInfo(GUPnPService *service, char *variable, GValue *value, gpointer userData);
/* A_ARG_TYPE_ */static void queryConnectionID(GUPnPService *service, char *variable, GValue *value, gpointer userData);
/* A_ARG_TYPE_ */static void queryAVTransportID(GUPnPService *service, char *variable, GValue *value, gpointer userData);
/* A_ARG_TYPE_ */static void queryRcsID(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryItemInfoFilter(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryResult(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryRenderingInfoList(GUPnPService *service, char *variable, GValue *value, gpointer userData);

#define UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE((object), \
			UPNP_TYPE_CONNECTION_MANAGER_SERVICE, UpnpConnectionManagerServicePrivate))
 
enum PROPERTY_CONNECTION_MANAGER_SERVICE {
	PROPERTY_BASE,
	PROPERTY_DEVICE,
	PROPERTY_MEDIA_PLAYER,
	N_PROPERTIES
};

typedef struct _UpnpConnectionManagerServicePrivate UpnpConnectionManagerServicePrivate;
struct _UpnpConnectionManagerServicePrivate {
	GUPnPRootDevice *device;
	GUPnPServiceInfo *service;

	UpnpMediaPlayer *mediaPlayer;

	gchar *sourceProtocolInfo;
	gchar *sinkProtocolInfo;
	gchar *currentConnectionIDs;
//	gchar *featureList;
//	/* optional */guint clockUpdateID;
//	/* optional */gchar *deviceClockInfoUpdates;
	/* A_ARG_TYPE_ */gchar *connectionStatus; /* OK ContentFormatMismatch InsufficientBandwidth UnreliableChannel Unknown */
	/* A_ARG_TYPE_ */gchar *connectionManager;
	/* A_ARG_TYPE_ */gchar *direction; /* Input Output */
	/* A_ARG_TYPE_ */gchar *protocolInfo;
	/* A_ARG_TYPE_ */gint connectionID;
	/* A_ARG_TYPE_ */gint avTransportID;
	/* A_ARG_TYPE_ */gint rcsID;
//	/* optional */gchar *itemInfoFilter;
//	/* optional */gchar *result;
//	/* optional */gchar *renderingInfoList;
};

static void upnpConnectionManagerService_class_init(UpnpConnectionManagerServiceClass *klass) {
	g_type_class_add_private(klass, sizeof(UpnpConnectionManagerServicePrivate));

	GObjectClass *baseClass = G_OBJECT_CLASS(klass);
	baseClass->set_property = upnpConnectionManagerServiceSetProperty;
	baseClass->get_property = upnpConnectionManagerServiceGetProperty;
	baseClass->dispose=upnpConnectionManagerServiceDispose;

	GParamSpec *properties[N_PROPERTIES] = {NULL, };
	properties[PROPERTY_DEVICE] = g_param_spec_object("device",
													  "Device",
													  "GUPnPRootDevice",
													  GUPNP_TYPE_ROOT_DEVICE, 
													  G_PARAM_WRITABLE | G_PARAM_CONSTRUCT);
	properties[PROPERTY_MEDIA_PLAYER] = g_param_spec_object("mediaPlayer",
															"Media Player",
															"UpnpMediaPlayer",
															UPNP_TYPE_MEDIA_PLAYER, 
															G_PARAM_WRITABLE | G_PARAM_CONSTRUCT);

	g_object_class_install_properties(baseClass, N_PROPERTIES, properties);
}

static void upnpConnectionManagerService_init(UpnpConnectionManagerService *self) {
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(self);

	priv->sourceProtocolInfo = "";
	priv->sinkProtocolInfo = "http-get:*:*:*, rtsp-rtp-udp:*:*:*, internal:*:*:*";
	priv->currentConnectionIDs = "0";
//	priv->featureList = 0;
//	/* optional */priv->clockUpdateID = 0;
//	/* optional */priv->deviceClockInfoUpdates = 0;
	/* A_ARG_TYPE_ */priv->connectionStatus = "OK"; /* OK ContentFormatMismatch InsufficientBandwidth UnreliableChannel Unknown */
	/* A_ARG_TYPE_ */priv->connectionManager = NULL;
	/* A_ARG_TYPE_ */priv->direction = "Input"; /* Input Output */
	/* A_ARG_TYPE_ */priv->protocolInfo = NULL;
	/* A_ARG_TYPE_ */priv->connectionID = -1;
	/* A_ARG_TYPE_ */priv->avTransportID = 0;
	/* A_ARG_TYPE_ */priv->rcsID = 0;
//	/* optional */priv->itemInfoFilter = 0;
//	/* optional */priv->result = 0;
//	/* optional */priv->renderingInfoList = 0;
}

void upnpConnectionManagerServiceConnect(UpnpConnectionManagerService *self) {
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(self);
	
	priv->service = gupnp_device_info_get_service(GUPNP_DEVICE_INFO(priv->device), "urn:schemas-upnp-org:service:ConnectionManager:3");
	if(!priv->service) {
//		g_printerr("Cannot get ConnectionManager service\n");
		
		exit(EXIT_FAILURE);
	}


/* ACTION CONNECT */
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetProtocolInfo", G_CALLBACK(getProtocolInfo), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetCurrentConnectionIDs", G_CALLBACK(getCurrentConnectionIDs), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetCurrentConnectionInfo", G_CALLBACK(getCurrentConnectionInfo), self);

/* QUERY CONNECT */
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::SourceProtocolInfo", G_CALLBACK(querySourceProtocolInfo), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::SinkProtocolInfo", G_CALLBACK(querySinkProtocolInfo), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::CurrentConnectionIDs", G_CALLBACK(queryCurrentConnectionIDs), self);
//	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::FeatureList", G_CALLBACK(queryFeatureList), self);
//	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::ClockUpdateID", G_CALLBACK(queryClockUpdateID), self);
//	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::DeviceClockInfoUpdates", G_CALLBACK(queryDeviceClockInfoUpdates), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_ConnectionStatus", G_CALLBACK(queryConnectionStatus), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_ConnectionManager", G_CALLBACK(queryConnectionManager), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_Direction", G_CALLBACK(queryDirection), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_ProtocolInfo", G_CALLBACK(queryProtocolInfo), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_ConnectionID", G_CALLBACK(queryConnectionID), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_AVTransportID", G_CALLBACK(queryAVTransportID), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_RcsID", G_CALLBACK(queryRcsID), self);
//	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_ItemInfoFilter", G_CALLBACK(queryItemInfoFilter), self);
//	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_Result", G_CALLBACK(queryResult), self);
//	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_RenderingInfoList", G_CALLBACK(queryRenderingInfoList), self);
}

static void upnpConnectionManagerServiceDispose(GObject *object) {
//	UpnpConnectionManagerService *self = UPNP_CONNECTION_MANAGER_SERVICE(object);
	G_OBJECT_CLASS(upnpConnectionManagerService_parent_class)->dispose(object);
}

static void upnpConnectionManagerServiceGetProperty(GObject *object, guint propertyID, GValue *value, GParamSpec *pspec) {
//	UpnpConnectionManagerService *self = UPNP_CONNECTION_MANAGER_SERVICE(object);
//	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(self);

	switch(propertyID) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propertyID, pspec);
			break;
	}
}

static void upnpConnectionManagerServiceSetProperty(GObject *object, guint propertyID, const GValue *value, GParamSpec *pspec) {      
	UpnpConnectionManagerService *self = UPNP_CONNECTION_MANAGER_SERVICE(object);
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(self);

	switch(propertyID) {
		case PROPERTY_DEVICE:
			priv->device = GUPNP_ROOT_DEVICE(g_value_get_object(value));
			break;

		case PROPERTY_MEDIA_PLAYER: 
			priv->mediaPlayer = UPNP_MEDIA_PLAYER(g_value_get_object(value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propertyID, pspec);
			break;
	}
}



/* action function */

G_MODULE_EXPORT void
getProtocolInfo(GUPnPService *service, GUPnPServiceAction *action, gpointer userData) {
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(UPNP_CONNECTION_MANAGER_SERVICE(userData));
	gchar *SourceProtocolInfo = priv->sourceProtocolInfo;
	gchar *SinkProtocolInfo = priv->sinkProtocolInfo;

	gupnp_service_action_set(action,
							 "Source", G_TYPE_STRING, SourceProtocolInfo,
							 NULL);
	gupnp_service_action_set(action,
							 "Sink", G_TYPE_STRING, SinkProtocolInfo,
							 NULL);

	gupnp_service_action_return(action);
}

G_MODULE_EXPORT void 
getCurrentConnectionIDs(GUPnPService *service, GUPnPServiceAction *action, gpointer userData) {
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(UPNP_CONNECTION_MANAGER_SERVICE(userData));
	gchar *CurrentConnectionIDs = priv->currentConnectionIDs;

	gupnp_service_action_set(action,
							 "ConnectionIDs", G_TYPE_STRING, CurrentConnectionIDs,
							 NULL);

	gupnp_service_action_return(action);
}

G_MODULE_EXPORT void
getCurrentConnectionInfo(GUPnPService *service, GUPnPServiceAction *action, gpointer userData) {
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(UPNP_CONNECTION_MANAGER_SERVICE(userData));
	gchar *CurrentConnectionIDs;
	gint  RcsID = priv->rcsID;
	gint  AVTransportID = priv->avTransportID;
	gchar *ProtocolInfo = priv->protocolInfo;
	gchar *ConnectionManager = priv->connectionManager;
	gint  ConnectionID = priv->connectionID;
	gchar *Direction = priv->direction;
	gchar *Status = priv->connectionStatus;

	gupnp_service_action_get(action,
							 "ConnectionIDs", G_TYPE_STRING, &CurrentConnectionIDs,
							 NULL);
	gupnp_service_action_set(action,
							 "RcsID", G_TYPE_INT, RcsID,
							 NULL);
	gupnp_service_action_set(action,
							 "AVTransportID", G_TYPE_INT, AVTransportID,
							 NULL);
	gupnp_service_action_set(action,
							 "ProtocolInfo", G_TYPE_STRING, ProtocolInfo,
							 NULL);
	gupnp_service_action_set(action,
							 "ConnectionManager", G_TYPE_STRING, ConnectionManager,
							 NULL);
	gupnp_service_action_set(action,
							 "ConnectionID", G_TYPE_INT, ConnectionID,
							 NULL);
	gupnp_service_action_set(action,
							 "Direction", G_TYPE_STRING, Direction,
							 NULL);
	gupnp_service_action_set(action,
							 "Status", G_TYPE_STRING, Status,
							 NULL);
	gupnp_service_action_return(action);
}


/* query function */

G_MODULE_EXPORT void 
querySourceProtocolInfo(GUPnPService *service, char *variable, GValue *value, gpointer userData) {
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(UPNP_CONNECTION_MANAGER_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->sourceProtocolInfo);
}

G_MODULE_EXPORT void 
querySinkProtocolInfo(GUPnPService *service, char *variable, GValue *value, gpointer userData) {
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(UPNP_CONNECTION_MANAGER_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->sinkProtocolInfo);
}

G_MODULE_EXPORT void 
queryCurrentConnectionIDs(GUPnPService *service, char *variable, GValue *value, gpointer userData) {
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(UPNP_CONNECTION_MANAGER_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->currentConnectionIDs);
	
}

/*G_MODULE_EXPORT void 
queryFeatureList(GUPnPService *service, char *variable, GValue *value, gpointer userData) {
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(UPNP_CONNECTION_MANAGER_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->featureList);
}*/

/* optional */
/*G_MODULE_EXPORT void 
queryClockUpdateID(GUPnPService *service, char *variable, GValue *value, gpointer userData) {
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(UPNP_CONNECTION_MANAGER_SERVICE(userData));

	g_value_init(value, G_TYPE_UINT);
	g_value_set_uint(value, priv->clockUpdateID);
}*/

/* optional */
/*G_MODULE_EXPORT void 
queryDeviceClockInfoUpdates(GUPnPService *service, char *variable, GValue *value, gpointer userData) {
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(UPNP_CONNECTION_MANAGER_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->deviceClockInfoUpdates);
}*/

/* A_ARG_TYPE_ */
G_MODULE_EXPORT void 
queryConnectionStatus(GUPnPService *service, char *variable, GValue *value, gpointer userData) {
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(UPNP_CONNECTION_MANAGER_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->connectionStatus);
}

/* A_ARG_TYPE_ */
G_MODULE_EXPORT void 
queryConnectionManager(GUPnPService *service, char *variable, GValue *value, gpointer userData) {
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(UPNP_CONNECTION_MANAGER_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->connectionManager);
}

/* A_ARG_TYPE_ */
G_MODULE_EXPORT void 
queryDirection(GUPnPService *service, char *variable, GValue *value, gpointer userData) {
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(UPNP_CONNECTION_MANAGER_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->direction);
}

/* A_ARG_TYPE_ */
G_MODULE_EXPORT void 
queryProtocolInfo(GUPnPService *service, char *variable, GValue *value, gpointer userData) {
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(UPNP_CONNECTION_MANAGER_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->protocolInfo);
}

/* A_ARG_TYPE_ */
G_MODULE_EXPORT void 
queryConnectionID(GUPnPService *service, char *variable, GValue *value, gpointer userData) {
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(UPNP_CONNECTION_MANAGER_SERVICE(userData));

	g_value_init(value, G_TYPE_INT);
	g_value_set_int(value, priv->connectionID);
}

/* A_ARG_TYPE_ */
G_MODULE_EXPORT void 
queryAVTransportID(GUPnPService *service, char *variable, GValue *value, gpointer userData) {
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(UPNP_CONNECTION_MANAGER_SERVICE(userData));

	g_value_init(value, G_TYPE_INT);
	g_value_set_int(value, priv->avTransportID);
}

/* A_ARG_TYPE_ */
G_MODULE_EXPORT void 
queryRcsID(GUPnPService *service, char *variable, GValue *value, gpointer userData) {
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(UPNP_CONNECTION_MANAGER_SERVICE(userData));

	g_value_init(value, G_TYPE_INT);
	g_value_set_int(value, priv->rcsID);
}

/* optional */
/*G_MODULE_EXPORT void 
queryItemInfoFilter(GUPnPService *service, char *variable, GValue *value, gpointer userData) {
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(UPNP_CONNECTION_MANAGER_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->itemInfoFilter);
}*/

/* optional */
/*G_MODULE_EXPORT void 
queryResult(GUPnPService *service, char *variable, GValue *value, gpointer userData) {
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(UPNP_CONNECTION_MANAGER_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->result);
}*/

/* optional */
/*G_MODULE_EXPORT void 
queryRenderingInfoList(GUPnPService *service, char *variable, GValue *value, gpointer userData) {
	UpnpConnectionManagerServicePrivate *priv = UPNP_CONNECTION_MANAGER_SERVICE_GET_PRIVATE(UPNP_CONNECTION_MANAGER_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->renderingInfoList);
}*/
