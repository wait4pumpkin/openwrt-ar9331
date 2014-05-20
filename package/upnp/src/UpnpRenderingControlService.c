#include "UpnpRenderingControlService.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "UpnpMediaPlayer.h"

G_DEFINE_TYPE(UpnpRenderingControlService, upnpRenderingControlService, G_TYPE_OBJECT);

static void upnpRenderingControlServiceDispose(GObject *object);
static void upnpRenderingControlServiceGetProperty(GObject *object, guint propertyID, GValue *value, GParamSpec *pspec);
static void upnpRenderingControlServiceSetProperty(GObject *object, guint propertyID, const GValue *value, GParamSpec *pspec);

static void listPresets(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
static void selectPreset(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
static void getMute(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
static void setMute(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
static void getVolume(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
static void setVolume(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
static void getVolumeDB(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
static void setVolumeDB(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
static void getVolumeDBRange(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);

static void queryPresetNameList(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryLastChange(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryMute(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryVolume(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryVolumeDB(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryChannel(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryInstanceID(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryPresetName(GUPnPService *service, char *variable, GValue *value, gpointer userData);

void create_notify(GUPnPService *service, char* variable, const gchar* value, const gchar* channel );

#define UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE((object), \
			UPNP_TYPE_RENDERING_CONTROL_SERVICE, UpnpRenderingControlServicePrivate))
 
enum PROPERTY_RENDERING_CONTROL_SERVICE {
	PROPERTY_BASE,
	PROPERTY_DEVICE,
	PROPERTY_MEDIA_PLAYER,
	N_PROPERTIES
};

typedef struct _UpnpRenderingControlServicePrivate UpnpRenderingControlServicePrivate;
struct _UpnpRenderingControlServicePrivate {
	GUPnPRootDevice *device;
	GUPnPServiceInfo *service;

	UpnpMediaPlayer *mediaPlayer;

	gchar    *presetNameList;
	gchar    *lastChange;
	gboolean Mute;
	guint	 Volume;
	gint   	 VolumeDB;
	gchar    *Channel;
	guint    InstanceID;
	gchar    *PresetName;
};

static void upnpRenderingControlService_class_init(UpnpRenderingControlServiceClass *klass) {
	g_type_class_add_private(klass, sizeof(UpnpRenderingControlServicePrivate));

	GObjectClass *baseClass = G_OBJECT_CLASS(klass);
	baseClass->set_property = upnpRenderingControlServiceSetProperty;
	baseClass->get_property = upnpRenderingControlServiceGetProperty;
	baseClass->dispose=upnpRenderingControlServiceDispose;

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

static void upnpRenderingControlService_init(UpnpRenderingControlService *self) {
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(self);

	priv->mediaPlayer = NULL;
	priv->rtpClient = NULL;

	priv->LastChange = "";
	priv->PresetNameList = "";
	priv->Mute = 0;
	priv->Volume = startvolume();/* 0(min) 20(max) 1(step) */
//	priv->VolumeDB = 0;
//	priv->Loudness = 0;
//	priv->AllowedTransformSettings = "";
//	priv->TransformSettings = "";
//	priv->AllowedDefaultTransformSettings = "";
//	priv->DefaultTransformSettings = "";
	/*A_ARG_TYPE_*/priv->Channel = "Master";
	/*A_ARG_TYPE_*/priv->InstanceID = 0;
	/*A_ARG_TYPE_*/priv->PresetName = "FactoryDefaults";
//	/*A_ARG_TYPE_*/priv->DeviceUDN = "";
//	/*A_ARG_TYPE_*/priv->ServiceType = "";
//	/*A_ARG_TYPE_*/priv->ServiceID = "";
//	/*A_ARG_TYPE_*/priv->StateVariableValuePairs = "";
//	/*A_ARG_TYPE_*/priv->StateVariableList = "";
	/*self-def*/priv->SrcMode="WIFI";
	/*self-def*/priv->rtpClientPort = -1;
}

void upnpRenderingControlServiceConnect(UpnpRenderingControlService *self) {
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(self);
	
	priv->service = gupnp_device_info_get_service(GUPNP_DEVICE_INFO(priv->device), "urn:schemas-upnp-org:service:RenderingControl:1");
	if(!priv->service) {
		exit(EXIT_FAILURE);
	}

	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::ListPresets", G_CALLBACK(listPresets), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SelectPreset", G_CALLBACK(selectPreset), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetMute", G_CALLBACK(getMute), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetMute", G_CALLBACK(setMute), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetVolume", G_CALLBACK(getVolume), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetVolume", G_CALLBACK(setVolume), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetVolumeDB", G_CALLBACK(getVolumeDB), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetVolumeDB", G_CALLBACK(setVolumeDB), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetVolumeDBRange", G_CALLBACK(getVolumeDBRange), self);

	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::PresetNameList", G_CALLBACK(queryPresetNameList), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::LastChange", G_CALLBACK(queryLastChange), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::Mute", G_CALLBACK(queryMute), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::Volume", G_CALLBACK(queryVolume), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::VolumeDB", G_CALLBACK(queryVolumeDB), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_Channel", G_CALLBACK(queryChannel), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_InstanceID", G_CALLBACK(queryInstanceID), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_PresetName", G_CALLBACK(queryPresetName), self);
}

static void upnpRenderingControlServiceDispose(GObject *object) {
	//UpnpRenderingControlService *self = UPNP_RENDERING_CONTROL_SERVICE(object);
	G_OBJECT_CLASS(upnpRenderingControlService_parent_class)->dispose(object);
}

static void upnpRenderingControlServiceGetProperty(GObject *object, guint propertyID, GValue *value, GParamSpec *pspec) {
//	UpnpRenderingControlService *self = UPNP_RENDERING_CONTROL_SERVICE(object);
//	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(self);

	switch(propertyID) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propertyID, pspec);
			break;
	}
}

static void upnpRenderingControlServiceSetProperty(GObject *object, guint propertyID, const GValue *value, GParamSpec *pspec) {      
	UpnpRenderingControlService *self = UPNP_RENDERING_CONTROL_SERVICE(object);
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(self);

	switch(propertyID) {
		case PROPERTY_DEVICE:
			priv->device = GUPNP_ROOT_DEVICE(g_value_get_object(value));
			break;

		case PROPERTY_MEDIA_PLAYER: 
			priv->mediaPlayer = UPNP_MEDIA_PLAYER(g_value_get_object(value));
			break;

		case PROPERTY_RTP_CLIENT:
			priv->rtpClient = RTP_CLIENT(g_value_get_object(value));
			priv->rtpClientPort = rtpClientGetPort(priv->rtpClient);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propertyID, pspec);
			break;
	}
}

/* CALLBACK FUNCTION */

G_MODULE_EXPORT void 
listPresets(GUPnPService *service, GUPnPServiceAction *action, gpointer userData)
{
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(UPNP_RENDERING_CONTROL_SERVICE(userData));
	//gchar * presetnamelist = priv->PresetNameList;
	
	gupnp_service_action_set(action,
							 "CurrentMute", G_TYPE_STRING, priv->PresetNameList,
							 NULL);
	gupnp_service_action_return(action);
}

G_MODULE_EXPORT void 
selectPreset(GUPnPService *service, GUPnPServiceAction *action, gpointer userData)
{
	guint InstanceID;
	gchar *presetName;

	gupnp_service_action_get(action,
							 "InstanceID", G_TYPE_UINT, &InstanceID,
							 NULL);
	gupnp_service_action_get(action,
							 "PresetName", G_TYPE_STRING, &presetName,
							 NULL);

	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(UPNP_RENDERING_CONTROL_SERVICE(userData));
	priv->InstanceID = InstanceID;
	priv->PresetName = presetName;
	create_notify(service, "PresetNameList", presetName, NULL);
	
	gupnp_service_action_return(action);
}

/* optional */ 
G_MODULE_EXPORT void 
getMute(GUPnPService *service, GUPnPServiceAction *action, gpointer userData)
{
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(UPNP_RENDERING_CONTROL_SERVICE(userData));
	gboolean Mute = priv->Mute;
	
	gupnp_service_action_set(action,
							 "CurrentMute", G_TYPE_BOOLEAN, Mute,
							 NULL);
	gupnp_service_action_return(action);
}

G_MODULE_EXPORT void 
setMute(GUPnPService *service, GUPnPServiceAction *action, gpointer userData)
{
	guint InstanceID;
	gchar *Channel;
	gboolean Mute;
	gchar *Value = "0";

	gupnp_service_action_get(action,
							 "InstanceID", G_TYPE_UINT, &InstanceID,
							 NULL);
	gupnp_service_action_get(action,
							 "Channel", G_TYPE_STRING, &Channel,
							 NULL);
	gupnp_service_action_get(action,
							 "DesiredMute", G_TYPE_BOOLEAN, &Mute,
							 NULL);

	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(UPNP_RENDERING_CONTROL_SERVICE(userData));
	priv->InstanceID = InstanceID;
	priv->Channel = Channel;
	priv->Mute = Mute;
	
	if(0 == InstanceID){
		if(Mute == TRUE){
			Value = "1";
		}
		else{
			Value = "0";
		}
	}
	upnpMediaPlayerSetMute(priv->mediaPlayer, priv->Mute);
	create_notify(service, "Mute", Value, "Master");
	gupnp_service_action_return(action);
}

G_MODULE_EXPORT void 
getVolume(GUPnPService *service, GUPnPServiceAction *action, gpointer userData)
{
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(UPNP_RENDERING_CONTROL_SERVICE(userData));
	guint Volume = priv->Volume;
	
	gupnp_service_action_set(action,
							 "CurrentVolume", G_TYPE_UINT, Volume,
							 NULL);
	gupnp_service_action_return(action);
}

G_MODULE_EXPORT void 
queryLastChange(GUPnPService *service, char *variable, GValue *value, gpointer userData)
{
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(UPNP_RENDERING_CONTROL_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->LastChange);
}

G_MODULE_EXPORT void 
queryPresetNameList(GUPnPService *service, char *variable, GValue *value, gpointer userData)
{
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(UPNP_RENDERING_CONTROL_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->PresetNameList);
}

/* optional */ 
G_MODULE_EXPORT void 
queryMute(GUPnPService *service, char *variable, GValue *value, gpointer userData)
{
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(UPNP_RENDERING_CONTROL_SERVICE(userData));

	g_value_init(value, G_TYPE_BOOLEAN);
	g_value_set_uint(value, priv->Mute);
}

/* optional */ 
G_MODULE_EXPORT void 
queryVolume(GUPnPService *service, char *variable, GValue *value, gpointer userData)
{
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(UPNP_RENDERING_CONTROL_SERVICE(userData));

	g_value_init(value, G_TYPE_UINT);
	g_value_set_uint(value, priv->Volume);
}

G_MODULE_EXPORT void 
queryChannel(GUPnPService *service, char *variable, GValue *value, gpointer userData)
{
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(UPNP_RENDERING_CONTROL_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->Channel);
}

/*A_ARG_TYPE_*/ 
G_MODULE_EXPORT void 
queryInstanceID(GUPnPService *service, char *variable, GValue *value, gpointer userData)
{
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(UPNP_RENDERING_CONTROL_SERVICE(userData));

	g_value_init(value, G_TYPE_UINT);
	g_value_set_uint(value, priv->InstanceID);
}

G_MODULE_EXPORT void 
queryPresetName(GUPnPService *service, char *variable, GValue *value, gpointer userData)
{
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(UPNP_RENDERING_CONTROL_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->PresetName);
}

void create_notify(GUPnPService *service, char* variable, const gchar* value, const gchar* channel )
{
	gchar *lastchange = "";
	if(channel != NULL){
		lastchange = g_strdup_printf("<Event xmlns=\"urn:schemas-upnp-org:metadata-1-0/RCS/\"><InstanceID val=\"0\"><%s val=\"%s\" channel=\"%s\"/></InstanceID></Event>", variable, value, channel);
	}
	else{
		lastchange = g_strdup_printf("<Event xmlns=\"urn:schemas-upnp-org:metadata-1-0/RCS/\"><InstanceID val=\"0\"><%s val=\"%s\"/></InstanceID></Event>", variable, value);
	}
	gupnp_service_notify(service, "LastChange", G_TYPE_STRING, lastchange, NULL);
	free(lastchange);	
}

