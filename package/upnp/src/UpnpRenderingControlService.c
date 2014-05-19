#include "UpnpRenderingControlService.h"

#include "UpnpMediaPlayer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <asm-generic/ioctl.h>
#include <asm/types.h>
#include <signal.h>

#include "RtpClient.h"

G_DEFINE_TYPE(UpnpRenderingControlService, upnpRenderingControlService, G_TYPE_OBJECT);

void create_notify(GUPnPService *service, char* variable, const gchar* value, const gchar* channel );

static void upnpRenderingControlServiceDispose(GObject *object);
static void upnpRenderingControlServiceGetProperty(GObject *object, guint propertyID, GValue *value, GParamSpec *pspec);
static void upnpRenderingControlServiceSetProperty(GObject *object, guint propertyID, const GValue *value, GParamSpec *pspec);

static void startIRMonitor(UpnpRenderingControlService *self);

/*ACTION*/

static void listPresets(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
static void selectPreset(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void getBrightness(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void setBrightness(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void getContrast(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void setContrast(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void getSharpness(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void setSharpness(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void getRedVideoGain(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void setRedVideoGain(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void getGreenVideoGain(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void setGreenVideoGain(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void getBlueVideoGain(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void setBlueVideoGain(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void getRedVideoBlackLevel(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void setRedVideoBlackLevel(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void getGreenVideoBlackLevel(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void setGreenVideoBlackLevel(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void getBlueVideoBlackLevel(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void setBlueVideoBlackLevel(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void getColorTemperature(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void setColorTemperature(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void getHorizontalKeystone(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void setHorizontalKeystone(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void getVerticalKeystone(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void setVerticalKeystone(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
/* optional */ static void getMute(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
/* optional */ static void setMute(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
/* optional */ static void getVolume(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
/* optional */ static void setVolume(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
/* self-def */ static void getSrcMode(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
/* self-def */ static void setSrcMode(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
/* self-def */ static void getPortNumber(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
/* self-def */ static void update(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
/* self-def */ static void setVolumeTV(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void getVolumeDB(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
////* optional */ static void setVolumeDB(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void getVolumeDBRange(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void getLoudness(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void setLoudness(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void getStateVariables(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void setStateVariables(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void getAllowedTransforms(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void setTransforms(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void getTransforms(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void getAllAvailableTransforms(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void getAllowedDefaultTransforms(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void getDefaultTransforms(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
///* optional */ static void setDefaultTransforms(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);

/*STATE VARIABLES*/

static void queryLastChange(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryPresetNameList(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryBrightness(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryContrast(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void querySharpness(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryRedVideoGain(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryGreenVideoGain(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryBlueVideoGain(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryRedVideoBlackLevel(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryGreenVideoBlackLevel(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryBlueVideoBlackLevel(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryColorTemperature(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryHorizontalKeystone(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryVerticalKeystone(GUPnPService *service, char *variable, GValue *value, gpointer userData);
/* optional */static void queryMute(GUPnPService *service, char *variable, GValue *value, gpointer userData);
/* optional */static void queryVolume(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryVolumeDB(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryLoudness(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryAllowedTransformSettings(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryTransformSettings(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryAllowedDefaultTransformSettings(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional */static void queryDefaultTransformSettings(GUPnPService *service, char *variable, GValue *value, gpointer userData);
/* optional *//*A_ARG_TYPE_*/static void queryChannel(GUPnPService *service, char *variable, GValue *value, gpointer userData);
/*A_ARG_TYPE_*/static void queryInstanceID(GUPnPService *service, char *variable, GValue *value, gpointer userData);
/*A_ARG_TYPE_*/static void queryPresetName(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional *//*A_ARG_TYPE_*/static void queryDeviceUDN(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional *//*A_ARG_TYPE_*/static void queryServiceType(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional *//*A_ARG_TYPE_*/static void queryServiceID(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional *//*A_ARG_TYPE_*/static void queryStateVariableValuePairs(GUPnPService *service, char *variable, GValue *value, gpointer userData);
///* optional *//*A_ARG_TYPE_*/static void queryStateVariableList(GUPnPService *service, char *variable, GValue *value, gpointer userData);
/* self-def */static void querySrcMode(GUPnPService *service, char *variable, GValue *value, gpointer userData);
/* self-def */static void queryrtpClientPort(GUPnPService *service, char *variable, GValue *value, gpointer userData);
/* self-def */static void queryVolumeTV(GUPnPService *service, char *variable, GValue *value, gpointer userData);

#define UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE((object), UPNP_TYPE_RENDERING_CONTROL_SERVICE, UpnpRenderingControlServicePrivate))
 
enum PROPERTY_RENDERING_CONTROL_SERVICE {
	PROPERTY_BASE,
	PROPERTY_DEVICE,
	PROPERTY_MEDIA_PLAYER,
	PROPERTY_RTP_CLIENT, 
	N_PROPERTIES
};

uint startvolume(void) {
	int volume = 0;	
	FILE *fp=fopen("/etc/webxml/mediaconfig","r");
	if(fp == NULL) return 0;
	fscanf(fp,"volume=%d",&volume);
	fclose(fp);
	return volume;	
}

typedef struct _UpnpRenderingControlServicePrivate UpnpRenderingControlServicePrivate;
struct _UpnpRenderingControlServicePrivate {
	GUPnPRootDevice *device;
	GUPnPServiceInfo *service;

	UpnpMediaPlayer *mediaPlayer;
	RtpClient *rtpClient;

	gchar    *LastChange;
	gchar    *PresetNameList;
//	guint  Brightness;
//	guint  Contrast;
//	guint  Sharpness;
//	guint  RedVideoGain;
//	guint  GreenVideoGain;
//	guint  BlueVideoGain;
//	guint  RedVideoBlackLevel;
//	guint  GreenVideoBlackLevel;
//	guint  BlueVideoBlackLevel;
//	guint  ColorTemperature;
//	gint   HorizontalKeystone;
//	gint   VerticalKeystone;	
	gboolean Mute;
	guint  Volume;
//	gint   VolumeDB;
//	gboolean Loudness;
//	gchar    *AllowedTransformSettings;
//	gchar    *TransformSettings;
//	gchar    *AllowedDefaultTransformSettings;
//	gchar    *DefaultTransformSettings;
	/*A_ARG_TYPE_*/gchar    *Channel;
	/*A_ARG_TYPE_*/guint    InstanceID;
	/*A_ARG_TYPE_*/gchar    *PresetName;
//	/*A_ARG_TYPE_*/gchar    *DeviceUDN;
//	/*A_ARG_TYPE_*/gchar    *ServiceType;
//	/*A_ARG_TYPE_*/gchar    *ServiceID;
//	/*A_ARG_TYPE_*/gchar    *StateVariableValuePairs;
//	/*A_ARG_TYPE_*/gchar    *StateVariableList;
	/*self-def*/gchar    *SrcMode;
	/*self-def*/guint    rtpClientPort;
	
	guint volumeTV;	
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
	properties[PROPERTY_RTP_CLIENT] = g_param_spec_object("rtpClient",
														  "Rtp Client",
														  "RtpClient",
														  RTP_TYPE_CLIENT, 
														  G_PARAM_WRITABLE);

	g_object_class_install_properties(baseClass, N_PROPERTIES, properties);
}

static void upnpRenderingControlService_init(UpnpRenderingControlService *self) {
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(self);

	priv->mediaPlayer = NULL;
	priv->rtpClient = NULL;

	priv->LastChange = "";
	priv->PresetNameList = "";
//	priv->Brightness = 10;		/* 0(min) 10(max) 1(step) */
//	priv->Contrast = 78;		/* 0(min) 300(max) 1(step) */
//	priv->Sharpness = 3;		/* 0(min) 6(max) 1(step) */
//	priv->RedVideoGain = 0;		/* 0(min) 255(max) 1(step) */
//	priv->GreenVideoGain = 0;	/* 0(min) 255(max) 1(step) */
//	priv->BlueVideoGain = 0;	/* 0(min) 255(max) 1(step) */		
//	priv->RedVideoBlackLevel = 0;	/* 0(min) 255(max) 1(step) */
//	priv->GreenVideoBlackLevel = 0;	/* 0(min) 255(max) 1(step) */
//	priv->BlueVideoBlackLevel = 0;	/* 0(min) 255(max) 1(step) */
//	priv->ColorTemperature = 0;	/* 0(min) 10000(max) 1(step) */
//	priv->HorizontalKeystone = 0;
//	priv->VerticalKeystone = 0;	
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

	startIRMonitor(self);
}

void upnpRenderingControlServiceConnect(UpnpRenderingControlService *self) {
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(self);
	
	priv->service = gupnp_device_info_get_service(GUPNP_DEVICE_INFO(priv->device), "urn:schemas-upnp-org:service:RenderingControl:3");
	if(!priv->service) {
//		g_printerr("Cannot get RenderingControl service\n");
		
		exit(EXIT_FAILURE);
	}
	gdouble volume;
	volume = priv->Volume / 100.0;
	upnpMediaPlayerSetVolume(priv->mediaPlayer, volume);
/* ACTION CONNECT */
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::ListPresets", G_CALLBACK(listPresets), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SelectPreset", G_CALLBACK(selectPreset), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetBrightness", G_CALLBACK(getBrightness), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetBrightness", G_CALLBACK(setBrightness), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetContrast", G_CALLBACK(getContrast), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetContrast", G_CALLBACK(setContrast), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetSharpness", G_CALLBACK(getSharpness), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetSharpness", G_CALLBACK(setSharpness), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetRedVideoGain", G_CALLBACK(getRedVideoGain), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetRedVideoGain", G_CALLBACK(setRedVideoGain), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetGreenVideoGain", G_CALLBACK(getGreenVideoGain), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetGreenVideoGain", G_CALLBACK(setGreenVideoGain), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetBlueVideoGain", G_CALLBACK(getBlueVideoGain), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetBlueVideoGain", G_CALLBACK(setBlueVideoGain), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetRedVideoBlackLevel", G_CALLBACK(getRedVideoBlackLevel), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetRedVideoBlackLevel", G_CALLBACK(setRedVideoBlackLevel), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetGreenVideoBlackLevel", G_CALLBACK(getGreenVideoBlackLevel), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetGreenVideoBlackLevel", G_CALLBACK(setGreenVideoBlackLevel), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetBlueVideoBlackLevel", G_CALLBACK(getBlueVideoBlackLevel), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetBlueVideoBlackLevel", G_CALLBACK(setBlueVideoBlackLevel), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetColorTemperature", G_CALLBACK(getColorTemperature), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetColorTemperature", G_CALLBACK(setColorTemperature), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetHorizontalKeystone", G_CALLBACK(getHorizontalKeystone), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetHorizontalKeystone", G_CALLBACK(setHorizontalKeystone), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetVerticalKeystone", G_CALLBACK(getVerticalKeystone), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetVerticalKeystone", G_CALLBACK(setVerticalKeystone), self);
	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetMute", G_CALLBACK(getMute), self);
	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetMute", G_CALLBACK(setMute), self);
	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetVolume", G_CALLBACK(getVolume), self);
	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetVolume", G_CALLBACK(setVolume), self);
	/* self-def */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetSrcMode", G_CALLBACK(getSrcMode), self);
	/* self-def */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetSrcMode", G_CALLBACK(setSrcMode), self);
	/* self-def */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetPortNumber", G_CALLBACK(getPortNumber), self);
	/* self-def */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::Update", G_CALLBACK(update), self);
	/* self-def */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetVolumeTV", G_CALLBACK(setVolumeTV), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetVolumeDB", G_CALLBACK(getVolumeDB), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetVolumeDB", G_CALLBACK(setVolumeDB), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetVolumeDBRange", G_CALLBACK(getVolumeDBRange), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetLoudness", G_CALLBACK(getLoudness), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetLoudness", G_CALLBACK(setLoudness), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetStateVariables", G_CALLBACK(getStateVariables), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetStateVariables", G_CALLBACK(setStateVariables), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetAllowedTransforms", G_CALLBACK(getAllowedTransforms), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetTransforms", G_CALLBACK(setTransforms), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetTransforms", G_CALLBACK(getTransforms), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetAllAvailableTransforms", G_CALLBACK(getAllAvailableTransforms), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetAllowedDefaultTransforms", G_CALLBACK(getAllowedDefaultTransforms), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetDefaultTransforms", G_CALLBACK(getDefaultTransforms), self);
	///* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetDefaultTransforms", G_CALLBACK(setDefaultTransforms), self);

/* QUERY CONNECT */
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::LastChange", G_CALLBACK(queryLastChange), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::PresetNameList", G_CALLBACK(queryPresetNameList), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::Brightness", G_CALLBACK(queryBrightness), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::Contrast", G_CALLBACK(queryContrast), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::Sharpness", G_CALLBACK(querySharpness), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::RedVideoGain", G_CALLBACK(queryRedVideoGain), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::GreenVideoGain", G_CALLBACK(queryGreenVideoGain), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::BlueVideoGain", G_CALLBACK(queryBlueVideoGain), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::RedVideoBlackLevel", G_CALLBACK(queryRedVideoBlackLevel), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::GreenVideoBlackLevel", G_CALLBACK(queryGreenVideoBlackLevel), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::BlueVideoBlackLevel", G_CALLBACK(queryBlueVideoBlackLevel), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::ColorTemperature", G_CALLBACK(queryColorTemperature), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::HorizontalKeystone", G_CALLBACK(queryHorizontalKeystone), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::VerticalKeystone", G_CALLBACK(queryVerticalKeystone), self);
	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::Mute", G_CALLBACK(queryMute), self);
	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::Volume", G_CALLBACK(queryVolume), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::VolumeDB", G_CALLBACK(queryVolumeDB), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::Loudness", G_CALLBACK(queryLoudness), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::AllowedTransformSettings", G_CALLBACK(queryAllowedTransformSettings), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::TransformSettings", G_CALLBACK(queryTransformSettings), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::AllowedDefaultTransformSettings", G_CALLBACK(queryAllowedDefaultTransformSettings), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::DefaultTransformSettings", G_CALLBACK(queryDefaultTransformSettings), self);
	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_Channel", G_CALLBACK(queryChannel), self);
	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_InstanceID", G_CALLBACK(queryInstanceID), self);
	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_PresetName", G_CALLBACK(queryPresetName), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_DeviceUDN", G_CALLBACK(queryDeviceUDN), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_ServiceType", G_CALLBACK(queryServiceType), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_ServiceID", G_CALLBACK(queryServiceID), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_StateVariableValuePairs", G_CALLBACK(queryStateVariableValuePairs), self);
//	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_StateVariableList", G_CALLBACK(queryStateVariableList), self);
	/* self-def */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::SrcMode", G_CALLBACK(querySrcMode),self);
	/* self-def */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::rtpClientPort", G_CALLBACK(queryrtpClientPort), self);
	/* self-def */g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::volumeTV", G_CALLBACK(queryVolumeTV), self);
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

/* optional */ 
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

/* optional */ 
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

/* optional */ 
G_MODULE_EXPORT void 
setVolume(GUPnPService *service, GUPnPServiceAction *action, gpointer userData)
{
	guint InstanceID;
	gchar *Channel;
	guint Volume;
	gdouble volumeset;
	char *Value = "0";

	gupnp_service_action_get(action,
							 "InstanceID", G_TYPE_UINT, &InstanceID,
							 NULL);
	gupnp_service_action_get(action,
							 "Channel", G_TYPE_STRING, &Channel,
							 NULL);
	gupnp_service_action_get(action,
							 "DesiredVolume", G_TYPE_UINT, &Volume,
							 NULL);

	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(UPNP_RENDERING_CONTROL_SERVICE(userData));
	priv->InstanceID = InstanceID;
	priv->Channel = Channel;
	priv->Volume = Volume;

	if (0 == InstanceID)
	{
		volumeset = Volume * 1.0 / 100.0;
		Value=g_strdup_printf("%u", Volume);
		upnpMediaPlayerSetVolume(priv->mediaPlayer,volumeset);
	}
	create_notify(service, "Volume", Value, "Master");
	gupnp_service_action_return(action);	
}

/* self defined */ 
G_MODULE_EXPORT void 
setVolumeTV(GUPnPService *service, GUPnPServiceAction *action, gpointer userData) {
	guint volume;

	gupnp_service_action_get(action,
							 "DesiredVolume", G_TYPE_UINT, &volume,
							 NULL);
	
	gupnp_service_action_return(action);

	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(UPNP_RENDERING_CONTROL_SERVICE(userData));
	priv->volumeTV = volume;

	gupnp_service_notify(service, "volumeTV", G_TYPE_UINT, volume, NULL);

	rtpClientSetVolume(priv->rtpClient, volume * 1.0 / 100.0);
}

/*self define*/G_MODULE_EXPORT void 
getSrcMode(GUPnPService *service, GUPnPServiceAction *action, gpointer userData)
{
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(UPNP_RENDERING_CONTROL_SERVICE(userData));
	gchar *source = priv->SrcMode;
	
	gupnp_service_action_set(action,
							 "CurrentSourceMode", G_TYPE_STRING, source,
							 NULL);
	gupnp_service_action_return(action);
	
}


/*self define*/G_MODULE_EXPORT void 
setSrcMode(GUPnPService *service, GUPnPServiceAction *action, gpointer userData)
{
	guint InstanceID;
	gchar *Channel;
	gchar *source;

	gupnp_service_action_get(action,
							 "InstanceID", G_TYPE_UINT, &InstanceID,
							 NULL);
	gupnp_service_action_get(action,
							 "Channel", G_TYPE_STRING, &Channel,
							 NULL);
	gupnp_service_action_get(action,
							 "CurrentSourceMode", G_TYPE_STRING, &source,
							 NULL);

	UpnpRenderingControlService *self = UPNP_RENDERING_CONTROL_SERVICE(userData);
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(self);
	priv->InstanceID = InstanceID;
	priv->Channel = Channel;

	if(!strcmp(source, "Aux_in"))
	{
//		g_print("the src mode is Aux_in\n");
		priv->SrcMode = "Aux_in";
		rtpClientEnable(priv->rtpClient, FALSE);
		upnpMediaPlayerEnable(priv->mediaPlayer, FALSE);
		system("irsend SEND_ONCE NEC AUX");
	}
	else if(!strcmp(source, "Line_in"))
	{
//		g_print("the src mode is Line_in\n");
		priv->SrcMode = "Line_in";
		rtpClientEnable(priv->rtpClient, FALSE);
		upnpMediaPlayerEnable(priv->mediaPlayer, FALSE);
		system("irsend SEND_ONCE NEC LINE");
	}
	else if(!strcmp(source, "TV"))
	{
//		g_print("the src mode is TV\n");
		priv->SrcMode = "TV";
		rtpClientEnable(priv->rtpClient, TRUE);
		upnpMediaPlayerEnable(priv->mediaPlayer, FALSE);
		system("/etc/init.d/airplay stop");
		system("irsend SEND_ONCE NEC TV");
	}
	else if(!strcmp(source, "WIFI"))
	{
//		g_print("the src mode is WIFI\n");
		priv->SrcMode = "WIFI";
		rtpClientEnable(priv->rtpClient, FALSE);
		upnpMediaPlayerEnable(priv->mediaPlayer, TRUE);
		system("/etc/init.d/airplay start");
		system("irsend SEND_ONCE NEC WIFI");
	}
	create_notify(service, "SrcMode", source, "Master");
	gupnp_service_action_return(action);
}

/*self define*/G_MODULE_EXPORT void 
getPortNumber(GUPnPService *service, GUPnPServiceAction *action, gpointer userData)
{
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(UPNP_RENDERING_CONTROL_SERVICE(userData));
	guint port = priv->rtpClientPort;
	
	gupnp_service_action_set(action,
							 "Port_Number", G_TYPE_UINT, port,
							 NULL);
	gupnp_service_action_return(action);
	
}

/*self define*/G_MODULE_EXPORT void 
update(GUPnPService *service, GUPnPServiceAction *action, gpointer userData)
{
	gchar *url;
	gchar *md5;
	gchar *command;
	url = (gchar *)calloc(128, 1);
	md5 = (gchar *)calloc(34, 1);
	command = (gchar *)calloc(180, 1);

	gupnp_service_action_get(action,
							 "UpdateFile", G_TYPE_STRING, &url,
							 NULL);
	gupnp_service_action_get(action,
							 "Md5", G_TYPE_STRING, &md5,
							 NULL);
	gupnp_service_action_return(action);

	g_print("the url is :%s\n", url);
	g_print("the md5 is :%s\n", md5);
	
	strcat(command, "/etc/up.sh ");
	strcat(command, url);
	strcat(command, " ");
	strcat(command, md5);
	g_print("the command is:%s\n", command);
	system(command);
	
	free(md5);
	free(url);
	free(command);
	md5 = NULL;
	url = NULL;
	command = NULL;
}


/*STATE VARIABLES*/

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

/*A_ARG_TYPE_*/ 
G_MODULE_EXPORT void 
queryPresetName(GUPnPService *service, char *variable, GValue *value, gpointer userData)
{
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(UPNP_RENDERING_CONTROL_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->PresetName);
}

/* self-def */
G_MODULE_EXPORT void 
querySrcMode(GUPnPService *service, char *variable, GValue *value, gpointer userData)
{
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(UPNP_RENDERING_CONTROL_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->SrcMode);
}

/* self-def */
G_MODULE_EXPORT void 
queryrtpClientPort(GUPnPService *service, char *variable, GValue *value, gpointer userData)
{
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(UPNP_RENDERING_CONTROL_SERVICE(userData));

	g_value_init(value, G_TYPE_UINT);
	g_value_set_uint(value, priv->rtpClientPort);
}

void storecurrentvolume(gpointer *self)
{	
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(UPNP_RENDERING_CONTROL_SERVICE(self));
//	g_print("store the volume!\n");
	FILE *fp=fopen("/etc/webxml/mediaconfig","w");
	if(fp == NULL)
		{
//			g_print("store start volume error\n");
			return ;
		}
//	g_print("read is ok,volume:%d !\n", priv->Volume);
	fprintf(fp,"volume=%d",priv->Volume);
	fclose(fp);
//	g_print("the volume is %d\n",volume);
	return ;	
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

/* self-def */ 
G_MODULE_EXPORT void 
queryVolumeTV(GUPnPService *service, char *variable, GValue *value, gpointer userData) {
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(UPNP_RENDERING_CONTROL_SERVICE(userData));

	g_value_init(value, G_TYPE_UINT);
	g_value_set_uint(value, priv->volumeTV);
}

#define IR_READ			_IOW('i', 0x00000026, __u32)

gint irDev = -1;
static UpnpRenderingControlServicePrivate *gPriv = NULL;
void irReceived(int signum) {
	static unsigned int code[9];
	read(irDev, &code, sizeof(code));
	ioctl(irDev, IR_READ, code);

	if (code[8] > 0) {
		system("/etc/scan_wlan0 -ap");
		system("irsend SEND_ONCE NEC RESET");
	}

	if(code[4] == 0x3 && code[5] == 0xA) {
		gPriv->SrcMode = "Aux_in";
		rtpClientEnable(gPriv->rtpClient, FALSE);
		upnpMediaPlayerEnable(gPriv->mediaPlayer, FALSE);
	} else if(code[4] == 0xE && code[5] == 0x9) {
		gPriv->SrcMode = "Line_in";
		rtpClientEnable(gPriv->rtpClient, FALSE);
		upnpMediaPlayerEnable(gPriv->mediaPlayer, FALSE);
	} else if(code[4] == 0xE && code[5] == 0xD) {
		gPriv->SrcMode = "TV";
		rtpClientEnable(gPriv->rtpClient, TRUE);
		upnpMediaPlayerEnable(gPriv->mediaPlayer, FALSE);
	} else if(code[4] == 0xB && code[5] == 0x6) {
		gPriv->SrcMode = "WIFI";
		rtpClientEnable(gPriv->rtpClient, FALSE);
		upnpMediaPlayerEnable(gPriv->mediaPlayer, TRUE);
	} else if(code[4] == 0x1 && code[5] == 0x1) {
		system("uci set wireless.@wifi-iface[0].disable=1");
	} else if(code[4] == 0x2 && code[5] == 0x2) {
		system("uci set wireless.@wifi-iface[0].disable=0");
	}
}

static void startIRMonitor(UpnpRenderingControlService *self) {
	UpnpRenderingControlServicePrivate *priv = UPNP_RENDERING_CONTROL_SERVICE_GET_PRIVATE(self);
	gPriv = priv;

	system("irsend SEND_ONCE NEC CONNECT");

    signal(SIGIO, irReceived);
	irDev = open("/dev/lirc1", O_RDONLY);
	if (irDev < 0) {
		return;
	}
    fcntl(irDev, F_SETOWN, getpid());
    gint flags = fcntl(irDev, F_GETFL); 
    fcntl(irDev, F_SETFL, flags | FASYNC);
}
