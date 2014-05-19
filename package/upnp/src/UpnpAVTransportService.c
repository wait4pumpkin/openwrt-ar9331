#include "UpnpAVTransportService.h"

#include "UpnpMediaPlayer.h"

#include <asm-generic/ioctl.h>
#include <asm/types.h>
#include <signal.h>

extern gint irDev;

#define IO_LOW			_IOW('i', 0x00000027, __u32)
#define IO_HIGH			_IOW('i', 0x00000028, __u32)

G_DEFINE_TYPE(UpnpAVTransportService, upnpAVTransportService, G_TYPE_OBJECT);

void create_notify(GUPnPService *service, char* variable, const gchar* value, const gchar* channel );

static void upnpAVTransportServiceDispose(GObject *object);
static void upnpAVTransportServiceGetProperty(GObject *object, guint propertyID, GValue *value, GParamSpec *pspec);
static void upnpAVTransportServiceSetProperty(GObject *object, guint propertyID, const GValue *value, GParamSpec *pspec);


/* action function */
static void setAVTransportURI(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);

static void getMediaInfo(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
/* GetMediaInfo_Ext */static void getMediaInfoExt(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
static void getTransportInfo(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
static void getPositionInfo(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
static void getDeviceCapabilities(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
static void getTransportSettings(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
static void stop(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
static void play(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
/* optional */static void pauses(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);
static void seek(GUPnPService *service, GUPnPServiceAction *action, gpointer userData);

/* query function */
static void queryLastChange(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryTransportState(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryTransportStatus(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryCurrentMediaCategory(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryPlaybackStorageMedium(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryRecordStorageMedium(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryPossiblePlaybackStorageMedia(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryPossibleRecordStorageMedia(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryCurrentPlayMode(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryTransportPlaySpeed(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryRecordMediumWriteStatus(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryCurrentRecordQualityMode(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryPossibleRecordQualityModes(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryNumberOfTracks(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryCurrentTrack(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryCurrentTrackDuration(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryCurrentMediaDuration(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryCurrentTrackMetaData(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryCurrentTrackURI(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryAVTransportURI(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryAVTransportURIMetaData(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryNextAVTransportURI(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryNextAVTransportURIMetaData(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryRelativeTimePosition(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryAbsoluteTimePosition(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryRelativeCounterPosition(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryAbsoluteCounterPosition(GUPnPService *service, char *variable, GValue *value, gpointer userData);
//static void queryLastChange(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void querySeekMode(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void querySeekTarget(GUPnPService *service, char *variable, GValue *value, gpointer userData);
static void queryInstanceID(GUPnPService *service, char *variable, GValue *value, gpointer userData);

#define UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE((object), UPNP_TYPE_AVTRANSPORT_SERVICE, UpnpAVTransportServicePrivate))
 
enum PROPERTY_AVTRANSPORT_SERVICE {
	PROPERTY_BASE,
	PROPERTY_DEVICE,
	PROPERTY_MEDIA_PLAYER,
	N_PROPERTIES
};

typedef struct _UpnpAVTransportServicePrivate UpnpAVTransportServicePrivate;
struct _UpnpAVTransportServicePrivate {
	GUPnPRootDevice *device;
	GUPnPServiceInfo *service;

	UpnpMediaPlayer *mediaPlayer;

	gchar *LastChange;
	gchar *transportState;/* STOPPED PLAYING PAUSED */
	gchar *transportStatus; /* OK ERROR_OCCURRED */
	gchar *currentMediaCategory; /* NO_MEDIA TRACK_AWARE TRACK_UNAWARE */
	gchar *playbackStorageMedium;
	gchar *recordStorageMedium;
	gchar *possiblePlaybackStorageMedia;
	gchar *possibleRecordStorageMedia;
	gchar *currentPlayMode; /* NORMAL(Default) */
	gchar *transportPlaySpeed; /* 1 */
	gchar *recordMediumWriteStatus;
	gchar *currentRecordQualityMode;
	gchar *possibleRecordQualityModes;
	guint numberOfTracks; /* 0(min) */
	guint currentTrack; /* 0(min) 1(step) */
	gchar *currentTrackDuration;
	gchar *currentMediaDuration;
	gchar *currentTrackMetaData;
	gchar *currentTrackURI;
	gchar *avTransportURI;
	gchar *avTransportURIMetaData;
	gchar *nextAVTransportURI;
	gchar *nextAVTransportURIMetaData;
	gchar *relativeTimePosition;
	gchar *absoluteTimePosition;
	gint relativeCounterPosition ;
	guint absoluteCounterPosition;
//	/* optional */gchar *currentTransportActions;
//	gchar *lastChange;
//	/* optional */gchar *drmState; /* OK */ /* Default: UNKNOWN */
//	/* optional */gchar *syncOffset;
	/* A_ARG_TYPE_ */gchar *seekMode; /* TRACK_NR */
	/* A_ARG_TYPE_ */gchar *seekTarget;
	/* A_ARG_TYPE_ */guint instanceID;
//	/* A_ARG_TYPE_ */gchar *deviceUDN;
//	/* optional */ /* A_ARG_TYPE_ */gchar *serviceType;
//	/* optional */ /* A_ARG_TYPE_ */gchar *serviceID;
///	/* optional */ /* A_ARG_TYPE_ */gchar *stateVariableValuePairs;
//	/* optional */ /* A_ARG_TYPE_ */gchar *stateVariableList;
//	/* optional */ /* A_ARG_TYPE_ */gchar *syncOffsetAdj;
//	/* optional */ /* A_ARG_TYPE_ */gchar *presentationTime;
//	/* optional */ /* A_ARG_TYPE_ */gchar *clockId;
//	/* optional */ /* A_ARG_TYPE_ */gchar *playlistData;
//	/* optional */ /* A_ARG_TYPE_ */guint playlistOffset;
//	/* optional */ /* A_ARG_TYPE_ */guint playlistDataLength;
//	/* optional */ /* A_ARG_TYPE_ */guint playlistTotalLength;
//	/* optional */ /* A_ARG_TYPE_ */gchar *playlistMIMEType;
//	/* optional */ /* A_ARG_TYPE_ */gchar *playlistExtendedType;
//	/* optional */ /* A_ARG_TYPE_ */gchar *playlistStep; /* Initial Continue Stop Reset */
//	/* optional */ /* A_ARG_TYPE_ */gchar *playlistType; /* Static Streaming */
//	/* optional */ /* A_ARG_TYPE_ */gchar *playlistInfo;
//	/* optional */ /* A_ARG_TYPE_ */gchar *playlistStartObjID;
//	/* optional */ /* A_ARG_TYPE_ */gchar *playlistStartGroupID;
};

static void upnpAVTransportService_class_init(UpnpAVTransportServiceClass *klass) {
	g_type_class_add_private(klass, sizeof(UpnpAVTransportServicePrivate));

	GObjectClass *baseClass = G_OBJECT_CLASS(klass);
	baseClass->set_property = upnpAVTransportServiceSetProperty;
	baseClass->get_property = upnpAVTransportServiceGetProperty;
	baseClass->dispose=upnpAVTransportServiceDispose;

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

static void upnpAVTransportService_init(UpnpAVTransportService *self) {
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(self);

	priv->LastChange = "";
	priv->transportState = "STOPPED"; /* STOPPED PLAYING */
	priv->transportStatus = "OK"; /* OK ERROR_OCCURRED */
	priv->currentMediaCategory = "NO_MEDIA"; /* NO_MEDIA TRACK_AWARE TRACK_UNAWARE */
	priv->playbackStorageMedium = "NOT_IMPLEMENTED";
	priv->recordStorageMedium = "NOT_IMPLEMENTED";
	priv->possiblePlaybackStorageMedia = "NOT_IMPLEMENTED";
	priv->possibleRecordStorageMedia = "NOT_IMPLEMENTED";
	priv->currentPlayMode = "NORMAL"; /* NORMAL(Default) */
	priv->transportPlaySpeed = "1"; /* 1 */
	priv->recordMediumWriteStatus = "NOT_IMPLEMENTED";
	priv->currentRecordQualityMode = "NOT_IMPLEMENTED";
	priv->possibleRecordQualityModes = "NOT_IMPLEMENTED";
	priv->numberOfTracks = 0; /* 0(min) */
	priv->currentTrack = 0; /* 0(min) 1(step) */
	priv->currentTrackDuration = (gchar *)malloc(25);
	priv->currentMediaDuration = (gchar *)malloc(25);
	priv->currentTrackMetaData = "NOT_IMPLEMENTED";
	priv->currentTrackURI = "";
	priv->avTransportURI = "";
	priv->avTransportURIMetaData = "";
	priv->nextAVTransportURI = "NOT_IMPLEMENTED";
	priv->nextAVTransportURIMetaData = "NOT_IMPLEMENTED";
	priv->relativeTimePosition = (gchar *)malloc(25);
	priv->absoluteTimePosition = (gchar *)malloc(25);
	priv->relativeCounterPosition = G_MAXINT;
	priv->absoluteCounterPosition = G_MAXINT;
//	/* optional */priv->currentTransportActions = NULL;
//	priv->lastChange = "";
//	/* optional */priv->drmState = "UNKNOWN"; /* OK */ /* Default: UNKNOWN */
//	/* optional */priv->syncOffset = "";
	/* A_ARG_TYPE_ */priv->seekMode = "TRACK_NR"; /* TRACK_NR */
	/* A_ARG_TYPE_ */priv->seekTarget = 0;
	/* A_ARG_TYPE_ */priv->instanceID = 0;
//	/* A_ARG_TYPE_ */priv->deviceUDN = "";
//	/* optional */ /* A_ARG_TYPE_ */priv->serviceType = "";
//	/* optional */ /* A_ARG_TYPE_ */priv->serviceID = "";
//	/* optional */ /* A_ARG_TYPE_ */priv->stateVariableValuePairs = "";
//	/* optional */ /* A_ARG_TYPE_ */priv->stateVariableList = "";
//	/* optional */ /* A_ARG_TYPE_ */priv->syncOffsetAdj = "";
//	/* optional */ /* A_ARG_TYPE_ */priv->presentationTime = "";
//	/* optional */ /* A_ARG_TYPE_ */priv->clockId = "";
//	/* optional */ /* A_ARG_TYPE_ */priv->playlistData = "";
//	/* optional */ /* A_ARG_TYPE_ */priv->playlistOffset = 0;
//	/* optional */ /* A_ARG_TYPE_ */priv->playlistDataLength = 0;
//	/* optional */ /* A_ARG_TYPE_ */priv->playlistTotalLength = 0;
//	/* optional */ /* A_ARG_TYPE_ */priv->playlistMIMEType = "";
//	/* optional */ /* A_ARG_TYPE_ */priv->playlistExtendedType = "";
//	/* optional */ /* A_ARG_TYPE_ */priv->playlistStep = "Initial"; /* Initial Continue Stop Reset */
//	/* optional */ /* A_ARG_TYPE_ */priv->playlistType = "Static"; /* Static Streaming */
//	/* optional */ /* A_ARG_TYPE_ */priv->playlistInfo = "";
//	/* optional */ /* A_ARG_TYPE_ */priv->playlistStartObjID = "";
//	/* optional */ /* A_ARG_TYPE_ */priv->playlistStartGroupID = "";
}

void upnpAVTransportServiceConnect(UpnpAVTransportService *self) {
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(self);
	
	priv->service = gupnp_device_info_get_service(GUPNP_DEVICE_INFO(priv->device), "urn:schemas-upnp-org:service:AVTransport:3");
	if(!priv->service) {
//		g_printerr("Cannot get AVTransport service\n");
		
		exit(EXIT_FAILURE);
	}

	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::SetAVTransportURI", G_CALLBACK(setAVTransportURI), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetMediaInfo", G_CALLBACK(getMediaInfo), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetMediaInfo_Ext", G_CALLBACK(getMediaInfoExt), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetTransportInfo", G_CALLBACK(getTransportInfo), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetPositionInfo", G_CALLBACK(getPositionInfo), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetDeviceCapabilities", G_CALLBACK(getDeviceCapabilities), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::GetTransportSettings", G_CALLBACK(getTransportSettings), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::Stop", G_CALLBACK(stop), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::Play", G_CALLBACK(play), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::Seek", G_CALLBACK(seek), self);
	/* optional */g_signal_connect(GUPNP_SERVICE(priv->service), "action-invoked::Pause", G_CALLBACK(pauses), self);

	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::LastChange", G_CALLBACK(queryLastChange), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::TransportState", G_CALLBACK(queryTransportState), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::TransportStatus", G_CALLBACK(queryTransportStatus), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::CurrentMediaCategory", G_CALLBACK(queryCurrentMediaCategory), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::PlaybackStorageMedium", G_CALLBACK(queryPlaybackStorageMedium), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::RecordStorageMedium", G_CALLBACK(queryRecordStorageMedium), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::PossiblePlaybackStorageMedia", G_CALLBACK(queryPossiblePlaybackStorageMedia), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::PossibleRecordStorageMedia", G_CALLBACK(queryPossibleRecordStorageMedia), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::CurrentPlayMode", G_CALLBACK(queryCurrentPlayMode), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::TransportPlaySpeed", G_CALLBACK(queryTransportPlaySpeed), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::RecordMediumWriteStatus", G_CALLBACK(queryRecordMediumWriteStatus), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::CurrentRecordQualityMode", G_CALLBACK(queryCurrentRecordQualityMode), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::PossibleRecordQualityModes", G_CALLBACK(queryPossibleRecordQualityModes), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::NumberOfTracks", G_CALLBACK(queryNumberOfTracks), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::CurrentTrack", G_CALLBACK(queryCurrentTrack), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::CurrentTrackDuration", G_CALLBACK(queryCurrentTrackDuration), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::CurrentMediaDuration", G_CALLBACK(queryCurrentMediaDuration), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::CurrentTrackMetaData", G_CALLBACK(queryCurrentTrackMetaData), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::CurrentTrackURI", G_CALLBACK(queryCurrentTrackURI), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::AVTransportURI", G_CALLBACK(queryAVTransportURI), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::AVTransportURIMetaData", G_CALLBACK(queryAVTransportURIMetaData), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::NextAVTransportURI", G_CALLBACK(queryNextAVTransportURI), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::NextAVTransportURIMetaData", G_CALLBACK(queryNextAVTransportURIMetaData), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::RelativeTimePosition", G_CALLBACK(queryRelativeTimePosition), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::AbsoluteTimePosition", G_CALLBACK(queryAbsoluteTimePosition), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::RelativeCounterPosition", G_CALLBACK(queryRelativeCounterPosition), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::AbsoluteCounterPosition", G_CALLBACK(queryAbsoluteCounterPosition), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_SeekMode", G_CALLBACK(querySeekMode), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_SeekTarget", G_CALLBACK(querySeekTarget), self);
	g_signal_connect(GUPNP_SERVICE(priv->service), "query-variable::A_ARG_TYPE_InstanceID", G_CALLBACK(queryInstanceID), self);

}

static void upnpAVTransportServiceDispose(GObject *object) {
	UpnpAVTransportService *self = UPNP_AVTRANSPORT_SERVICE(object);
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(self);
	G_OBJECT_CLASS(upnpAVTransportService_parent_class)->dispose(object);
	free(priv->currentTrackDuration);
	free(priv->currentMediaDuration);
	free(priv->relativeTimePosition);
	free(priv->absoluteTimePosition);
}

static void upnpAVTransportServiceGetProperty(GObject *object, guint propertyID, GValue *value, GParamSpec *pspec) {
//	UpnpAVTransportService *self = UPNP_AVTRANSPORT_SERVICE(object);
//	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(self);

	switch(propertyID) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propertyID, pspec);
			break;
	}
}

static void upnpAVTransportServiceSetProperty(GObject *object, guint propertyID, const GValue *value, GParamSpec *pspec) {      
	UpnpAVTransportService *self = UPNP_AVTRANSPORT_SERVICE(object);
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(self);

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

G_MODULE_EXPORT void
setAVTransportURI(GUPnPService *service, GUPnPServiceAction *action, gpointer userData) {
	guint instanceID;
	gchar *currentURI;
	gchar *currentURIMetaData;
	gchar *lastchange;

	gupnp_service_action_get(action,
							 "InstanceID", G_TYPE_UINT, &instanceID,
							 NULL);
	gupnp_service_action_get(action,
							 "CurrentURI", G_TYPE_STRING, &currentURI,
							 NULL);
	gupnp_service_action_get(action,
							 "CurrentURIMetaData", G_TYPE_STRING, &currentURIMetaData,
							 NULL);

	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));
	priv->numberOfTracks = 1;
	priv->avTransportURI = currentURI;
	priv->currentTrackURI = currentURI;
	priv->avTransportURIMetaData = currentURIMetaData;

	upnpMediaPlayerSetURI(priv->mediaPlayer, priv->avTransportURI);
	if(GST_STATE_NULL != upnpMediagetstate(priv->mediaPlayer))
	{
		upnpMediaPlayerStop(priv->mediaPlayer);
		priv->transportState = "STOPPED";
	}
	lastchange = g_strdup_printf("<Event xmlns=\"urn:schemas-upnp-org:metadata-1-0/AVT/\"><InstanceID val=\"0\"><CurrentTrackURI val=\"%s\"/><TransportState val=\"%s\"/><AVTransportURI val=\"%s\"/><CurrentTrackMetadata val=\"%s\"/></InstanceID></Event>", currentURI, priv->transportState, currentURI, currentURIMetaData);
	gupnp_service_notify(service, "LastChange", G_TYPE_STRING, lastchange, NULL);
	free(lastchange);
	gupnp_service_action_return(action);
}

G_MODULE_EXPORT void 
getMediaInfo(GUPnPService *service, GUPnPServiceAction *action, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));
	guint NumberOfTracks;
	gchar *CurrentMediaDuration = (gchar *)malloc(25);
	gchar *AVTransportURI;
	gchar *AVTransportURIMetaData;
	gchar *NextAVTransportURI;
	gchar *NextAVTransportURIMetaData;
	gchar *PlaybackStorageMedium;
	gchar *RecordStorageMedium;
	gchar *RecordMediumWriteStatus;

	g_snprintf(CurrentMediaDuration, 24, "%u:%02u:%02u.%09u", GST_TIME_ARGS(upnpMediaPlayerQueryDuration(priv->mediaPlayer)));
	if(upnpMediagetstate(priv->mediaPlayer) == GST_STATE_NULL)
	{
		g_snprintf(CurrentMediaDuration, 24, "%u:%02u:%02u", 0,0,0);
	}

	NumberOfTracks = priv->numberOfTracks;
	AVTransportURI = priv->avTransportURI;
	AVTransportURIMetaData = priv->avTransportURIMetaData;
	NextAVTransportURI = priv->nextAVTransportURI;
	NextAVTransportURIMetaData = priv->nextAVTransportURIMetaData;
	PlaybackStorageMedium = priv->playbackStorageMedium;
	RecordStorageMedium = priv->recordStorageMedium;
	RecordMediumWriteStatus = priv->recordMediumWriteStatus;
	strcpy(priv->currentMediaDuration, CurrentMediaDuration);

	gupnp_service_action_set(action,
							 "NrTracks", G_TYPE_UINT, NumberOfTracks,
							 NULL);
	gupnp_service_action_set(action,
							 "MediaDuration", G_TYPE_STRING, CurrentMediaDuration,
							 NULL);
	gupnp_service_action_set(action,
							 "CurrentURI", G_TYPE_STRING, AVTransportURI,
							 NULL);
	gupnp_service_action_set(action,
							 "CurrentURIMetaData", G_TYPE_STRING, AVTransportURIMetaData,
							 NULL);
	gupnp_service_action_set(action,
							 "NextURI", G_TYPE_STRING, NextAVTransportURI,
							 NULL);
	gupnp_service_action_set(action,
							 "NextURIMetaData", G_TYPE_STRING, NextAVTransportURIMetaData,
							 NULL);
	gupnp_service_action_set(action,
							 "PlayMedium", G_TYPE_STRING, PlaybackStorageMedium,
							 NULL);
	gupnp_service_action_set(action,
							 "RecordMedium", G_TYPE_STRING, RecordStorageMedium,
							 NULL);
	gupnp_service_action_set(action,
							 "WriteStatus", G_TYPE_STRING, RecordMediumWriteStatus,
							 NULL);
	gupnp_service_action_return(action);
	
}

G_MODULE_EXPORT void 
getMediaInfoExt(GUPnPService *service, GUPnPServiceAction *action, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));
	
	gchar *CurrentMediaCategory;
	guint NumberOfTracks;
	gchar *CurrentMediaDuration = (gchar *)malloc(25);
	gchar *AVTransportURI;
	gchar *AVTransportURIMetaData;
	gchar *NextAVTransportURI;
	gchar *NextAVTransportURIMetaData;
	gchar *PlaybackStorageMedium;
	gchar *RecordStorageMedium;
	gchar *RecordMediumWriteStatus;
	g_snprintf(CurrentMediaDuration, 24, "%u:%02u:%02u.%09u", GST_TIME_ARGS(upnpMediaPlayerQueryDuration(priv->mediaPlayer)));
	if(upnpMediagetstate(priv->mediaPlayer) == GST_STATE_NULL)
	{
		g_snprintf(CurrentMediaDuration, 24, "%u:%02u:%02u", 0,0,0);
	}

	CurrentMediaCategory = priv->currentMediaCategory;
	NumberOfTracks = priv->numberOfTracks;
	AVTransportURI = priv->avTransportURI;
	AVTransportURIMetaData = priv->avTransportURIMetaData;
	NextAVTransportURI = priv->nextAVTransportURI;
	NextAVTransportURIMetaData = priv->nextAVTransportURIMetaData;
	PlaybackStorageMedium = priv->playbackStorageMedium;
	RecordStorageMedium = priv->recordStorageMedium;
	RecordMediumWriteStatus = priv->recordMediumWriteStatus;
	strcpy(priv->currentMediaDuration, CurrentMediaDuration);

	gupnp_service_action_set(action,
							 "CurrentType", G_TYPE_STRING, CurrentMediaCategory,
							 NULL);
	gupnp_service_action_set(action,
							 "NrTracks", G_TYPE_UINT, NumberOfTracks,
							 NULL);
	gupnp_service_action_set(action,
							 "MediaDuration", G_TYPE_STRING, CurrentMediaDuration,
							 NULL);
	gupnp_service_action_set(action,
							 "CurrentURI", G_TYPE_STRING, AVTransportURI,
							 NULL);
	gupnp_service_action_set(action,
							 "CurrentURIMetaData", G_TYPE_STRING, AVTransportURIMetaData,
							 NULL);
	gupnp_service_action_set(action,
							 "NextURI", G_TYPE_STRING, NextAVTransportURI,
							 NULL);
	gupnp_service_action_set(action,
							 "NextURIMetaData", G_TYPE_STRING, NextAVTransportURIMetaData,
							 NULL);
	gupnp_service_action_set(action,
							 "PlayMedium", G_TYPE_STRING, PlaybackStorageMedium,
							 NULL);
	gupnp_service_action_set(action,
							 "RecordMedium", G_TYPE_STRING, RecordStorageMedium,
							 NULL);
	gupnp_service_action_set(action,
							 "WriteStatus", G_TYPE_STRING, RecordMediumWriteStatus,
							 NULL);
	gupnp_service_action_return(action);
}

G_MODULE_EXPORT void 
getTransportInfo(GUPnPService *service, GUPnPServiceAction *action, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));
	
	GstState  state=upnpMediagetstate(priv->mediaPlayer);
	switch(state)
	{
		case GST_STATE_NULL:
			priv->transportState="STOPPED";
			break;
		case GST_STATE_READY:
			priv->transportState="TRANSITIONING";
			break;
		case GST_STATE_PAUSED:
			priv->transportState="PAUSED_PLAYBACK";
			break;
		case GST_STATE_PLAYING:
			priv->transportState="PLAYING";
			break;
		default:
			break;
	}
	
	gchar *TransportState = priv->transportState;
	gchar *TransportStatus = priv->transportStatus;
	gchar *TransportPlaySpeed = priv->transportPlaySpeed;

	gupnp_service_action_set(action,
							 "CurrentTransportState", G_TYPE_STRING, TransportState,
							 NULL);
	gupnp_service_action_set(action,
							 "CurrentTransportStatus", G_TYPE_STRING, TransportStatus,
							 NULL);
	gupnp_service_action_set(action,
							 "CurrentSpeed", G_TYPE_STRING, TransportPlaySpeed,
							 NULL);
	gupnp_service_action_return(action);	
}

G_MODULE_EXPORT void 
getPositionInfo(GUPnPService *service, GUPnPServiceAction *action, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));
	guint CurrentTrack;
	gchar *CurrentTrackDuration = (gchar *)malloc(25);
	gchar *CurrentTrackMetaData;
	gchar *CurrentTrackURI;
	gchar *RelativeTimePosition = (gchar *)malloc(25);
	gchar *AbsoluteTimePosition = (gchar *)malloc(25);
	gint  RelativeCounterPosition;
	guint AbsoluteCounterPosition;
	if(upnpMediagetstate(priv->mediaPlayer) == GST_STATE_NULL)
	{
		g_snprintf(CurrentTrackDuration, 24, "%u:%02u:%02u", 0,0,0);

		g_snprintf(RelativeTimePosition, 24, "%u:%02u:%02u", 0,0,0);

		g_snprintf(AbsoluteTimePosition, 24, "%u:%02u:%02u", 0,0,0);
	}
	else{
		g_snprintf(CurrentTrackDuration, 24, "%u:%02u:%02u.%09u", GST_TIME_ARGS(upnpMediaPlayerQueryDuration(priv->mediaPlayer)));

		g_snprintf(RelativeTimePosition, 24, "%u:%02u:%02u.%09u", GST_TIME_ARGS(upnpMediaPlayerQueryPosition(priv->mediaPlayer)));

		g_snprintf(AbsoluteTimePosition, 24, "%u:%02u:%02u%09u", GST_TIME_ARGS(upnpMediaPlayerQueryPosition(priv->mediaPlayer)));
	}

	CurrentTrack = priv->currentTrack;
	CurrentTrackMetaData = priv->currentTrackMetaData;
	CurrentTrackURI = priv->currentTrackURI;
	strcpy(priv->relativeTimePosition, RelativeTimePosition);
	strcpy(priv->absoluteTimePosition, AbsoluteTimePosition);
	RelativeCounterPosition = priv->relativeCounterPosition;
	AbsoluteCounterPosition = priv->absoluteCounterPosition;
	strcpy(priv->currentTrackDuration, CurrentTrackDuration);

	gupnp_service_action_set(action,
							 "Track", G_TYPE_UINT, CurrentTrack,
							 NULL);
	gupnp_service_action_set(action,
							 "TrackDuration", G_TYPE_STRING, CurrentTrackDuration,
							 NULL);
	gupnp_service_action_set(action,
							 "TrackMetaData", G_TYPE_STRING, CurrentTrackMetaData,
							 NULL);
	gupnp_service_action_set(action,
							 "TrackURI", G_TYPE_STRING, CurrentTrackURI ,
							 NULL);
	gupnp_service_action_set(action,
							 "RelTime", G_TYPE_STRING, RelativeTimePosition,
							 NULL);
	gupnp_service_action_set(action,
							 "AbsTime", G_TYPE_STRING, AbsoluteTimePosition,
							 NULL);
	gupnp_service_action_set(action,
							 "RelCount", G_TYPE_INT, RelativeCounterPosition,
							 NULL);
	gupnp_service_action_set(action,
							 "AbsCount", G_TYPE_UINT, AbsoluteCounterPosition,
							 NULL);
	gupnp_service_action_return(action);
	free(CurrentTrackDuration);
	free(RelativeTimePosition);
	free(AbsoluteTimePosition);
}

G_MODULE_EXPORT void 
getDeviceCapabilities(GUPnPService *service, GUPnPServiceAction *action, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));
	gchar *PossiblePlaybackStorageMedia = priv->possiblePlaybackStorageMedia;
	gchar *PossibleRecordStorageMedia = priv->possibleRecordStorageMedia;
	gchar *PossibleRecordQualityModes = priv->possibleRecordQualityModes;

	gupnp_service_action_set(action,
							 "PlayMedia", G_TYPE_STRING, PossiblePlaybackStorageMedia,
							 NULL);
	gupnp_service_action_set(action,
							 "RecMedia", G_TYPE_STRING, PossibleRecordStorageMedia,
							 NULL);
	gupnp_service_action_set(action,
							 "RecQualityModes", G_TYPE_STRING, PossibleRecordQualityModes,
							 NULL);
	gupnp_service_action_return(action);	
}

G_MODULE_EXPORT void 
getTransportSettings(GUPnPService *service, GUPnPServiceAction *action, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));
	gchar *CurrentPlayMode = priv->currentPlayMode;
	gchar *CurrentRecordQualityMode = priv->currentRecordQualityMode;

	gupnp_service_action_set(action,
							 "PlayMode", G_TYPE_STRING, CurrentPlayMode,
							 NULL);
	gupnp_service_action_set(action,
							 "RecQualityMode", G_TYPE_STRING, CurrentRecordQualityMode,
							 NULL);
	gupnp_service_action_return(action);	
}

G_MODULE_EXPORT void 
play(GUPnPService *service, GUPnPServiceAction *action, gpointer userData) {
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));
	guint instanceID;
	gchar *TransportPlaySpeed;
	gchar *lastchange;
	gchar *CurrentTrackDuration = (gchar *)malloc(25);

	ioctl(irDev, IO_LOW, NULL);

	gupnp_service_action_get(action,
							 "InstanceID", G_TYPE_UINT, &instanceID,
							 NULL);
	gupnp_service_action_get(action,
							 "Speed", G_TYPE_STRING, &TransportPlaySpeed,
							 NULL);
	if(strcmp(priv->transportState, "PLAYING"))	
	{
		upnpMediaPlayerPlay(priv->mediaPlayer);
		if(GST_STATE_PLAYING == upnpMediagetstate(priv->mediaPlayer))
		{
			priv->transportState = "PLAYING";
		}
	}	
	lastchange=g_strdup_printf("<Event xmlns=\"urn:schemas-upnp-org:metadata-1-0/AVT/\"><InstanceID val=\"0\"><TransportState val=\"%s\"/></InstanceID></Event>", priv->transportState);
	gupnp_service_notify(service, "LastChange", G_TYPE_STRING, lastchange, NULL);

	free(lastchange);
	gchar *lastchangetest;
	g_snprintf(CurrentTrackDuration, 24, "%u:%02u:%02u.%09u", GST_TIME_ARGS(upnpMediaPlayerQueryDuration(priv->mediaPlayer)));
	if(strcmp(priv->currentTrackDuration, CurrentTrackDuration))
	{
		strcpy(priv->currentTrackDuration, CurrentTrackDuration);
		strcpy(priv->currentMediaDuration, CurrentTrackDuration);
		lastchangetest=g_strdup_printf("<Event xmlns=\"urn:schemas-upnp-org:metadata-1-0/AVT/\"><InstanceID val=\"0\"><CurrentTrackDuration val=\"%s\"/><CurrentMediaDuration val=\"%s\"/></InstanceID></Event>", priv->currentTrackDuration, priv->currentMediaDuration);
		gupnp_service_notify(service, "LastChange", G_TYPE_STRING, lastchangetest, NULL);
		free(lastchangetest);
	}
	gupnp_service_action_return(action);
}

G_MODULE_EXPORT void 
stop(GUPnPService *service, GUPnPServiceAction *action, gpointer userData) {
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));
	guint instanceID;
	gchar *lastchange;

	ioctl(irDev, IO_HIGH, NULL);

	gupnp_service_action_get(action,
							 "InstanceID", G_TYPE_UINT, &instanceID,
							 NULL);
	if(strcmp(priv->transportState, "STOPPED"))	
	{
		upnpMediaPlayerStop(priv->mediaPlayer);
		if(GST_STATE_NULL == upnpMediagetstate(priv->mediaPlayer))
		{
			priv->transportState = "STOPPED";
		}
	}
	lastchange=g_strdup_printf("<Event xmlns=\"urn:schemas-upnp-org:metadata-1-0/AVT/\"><InstanceID val=\"0\"><TransportState val=\"%s\"/></InstanceID></Event>", priv->transportState);
	gupnp_service_notify(service, "LastChange", G_TYPE_STRING, lastchange, NULL);
	free(lastchange);
	gupnp_service_action_return(action);
}

G_MODULE_EXPORT void 
pauses(GUPnPService *service, GUPnPServiceAction *action, gpointer userData) {
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));
	guint instanceID;
	gchar *lastchange;

	ioctl(irDev, IO_HIGH, NULL);

	gupnp_service_action_get(action,
							 "InstanceID", G_TYPE_UINT, &instanceID,
							 NULL);
	if(strcmp(priv->transportState, "PAUSED_PLAYBACK"))	
	{
		upnpMediaPlayerPause(priv->mediaPlayer);
		if(GST_STATE_PAUSED == upnpMediagetstate(priv->mediaPlayer))
		{
			priv->transportState = "PAUSED_PLAYBACK";
		}
	}
	lastchange=g_strdup_printf("<Event xmlns=\"urn:schemas-upnp-org:metadata-1-0/AVT/\"><InstanceID val=\"0\"><TransportState val=\"%s\"/></InstanceID></Event>", priv->transportState);
	gupnp_service_notify(service, "LastChange", G_TYPE_STRING, lastchange, NULL);
	free(lastchange);		
	gupnp_service_action_return(action);
}

G_MODULE_EXPORT void
seek(GUPnPService *service, GUPnPServiceAction *action, gpointer userData) {
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	guint InstanceID;
 	gchar *SeekMode;
 	gchar *SeekTarget;
	gint64 seektime;

 	gupnp_service_action_get(action,
 							 "InstanceID", G_TYPE_UINT, &InstanceID,
 							 NULL);
 	gupnp_service_action_get(action,
 							 "Unit", G_TYPE_STRING, &SeekMode,
 							 NULL);
 	gupnp_service_action_get(action,
 							 "Target", G_TYPE_STRING, &SeekTarget,
 							 NULL);

 	priv->seekTarget = SeekTarget;
	printf("the target is:%s\n", SeekTarget);
	
	if((0 == strcmp(SeekMode, "REL_TIME"))||(0 == strcmp(SeekMode, "ABS_TIME")))
	{
		priv->seekMode = SeekMode;
		seektime = time_from_string(SeekTarget);
		upnpMediaPlayerSeek(priv->mediaPlayer, seektime);
 		gupnp_service_notify(service, "A_ARG_TYPE_SeekMode", G_TYPE_STRING, SeekMode, NULL);
		gupnp_service_notify(service, "A_ARG_TYPE_SeekTarget", G_TYPE_STRING, SeekTarget, NULL);
	}
	if(priv->instanceID != InstanceID)
	{
		priv->instanceID = InstanceID;
 		gupnp_service_notify(service, "A_ARG_TYPE_InstanceID", G_TYPE_UINT, InstanceID, NULL);
	}
	gupnp_service_action_return(action);
}


G_MODULE_EXPORT void 
queryLastChange(GUPnPService *service, char *variable, GValue *value, gpointer userData)
{
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->LastChange);
}

G_MODULE_EXPORT void
queryInstanceID(GUPnPService *service, char *variable, GValue *value, gpointer userData) {
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_UINT);
	g_value_set_uint(value, priv->instanceID);
}

G_MODULE_EXPORT void 
querySeekMode(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->seekMode);
}

G_MODULE_EXPORT void 
queryTransportState(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->transportState);
}

G_MODULE_EXPORT void 
queryTransportStatus(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->transportStatus);
}

G_MODULE_EXPORT void 
queryCurrentMediaCategory(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->currentMediaCategory);
}

G_MODULE_EXPORT void 
queryPlaybackStorageMedium(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->playbackStorageMedium);
}

G_MODULE_EXPORT void 
queryRecordStorageMedium(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->recordStorageMedium);
}

G_MODULE_EXPORT void 
queryPossiblePlaybackStorageMedia(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->possiblePlaybackStorageMedia);
}

G_MODULE_EXPORT void 
queryPossibleRecordStorageMedia(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->possibleRecordStorageMedia);
}

G_MODULE_EXPORT void 
queryCurrentPlayMode(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->currentPlayMode);
}

G_MODULE_EXPORT void 
queryTransportPlaySpeed(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->transportPlaySpeed);
}

G_MODULE_EXPORT void 
queryRecordMediumWriteStatus(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->recordMediumWriteStatus);
}

G_MODULE_EXPORT void 
queryCurrentRecordQualityMode(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->currentRecordQualityMode);
}

G_MODULE_EXPORT void 
queryPossibleRecordQualityModes(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->possibleRecordQualityModes);
}

G_MODULE_EXPORT void 
queryCurrentTrack(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_UINT);
	g_value_set_uint(value, priv->currentTrack);
}

G_MODULE_EXPORT void 
queryCurrentTrackDuration(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->currentTrackDuration);
}

G_MODULE_EXPORT void 
queryCurrentMediaDuration(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->currentMediaDuration);
}

G_MODULE_EXPORT void 
queryCurrentTrackURI(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->currentTrackURI);
}

G_MODULE_EXPORT void 
queryNextAVTransportURI(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->nextAVTransportURI);
}

G_MODULE_EXPORT void 
queryNextAVTransportURIMetaData(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->nextAVTransportURIMetaData);
}

G_MODULE_EXPORT void 
queryRelativeTimePosition(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->relativeTimePosition);
}

G_MODULE_EXPORT void 
queryAbsoluteTimePosition(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->absoluteTimePosition);
}

G_MODULE_EXPORT void 
queryRelativeCounterPosition(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_INT);
	g_value_set_int(value, priv->relativeCounterPosition);
}

G_MODULE_EXPORT void 
queryAbsoluteCounterPosition(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_UINT);
	g_value_set_uint(value, priv->absoluteCounterPosition);
}

G_MODULE_EXPORT void 
querySeekTarget(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->seekTarget);
}

G_MODULE_EXPORT void 
queryNumberOfTracks(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_UINT);
	g_value_set_uint(value, priv->numberOfTracks);
}

G_MODULE_EXPORT void 
queryCurrentTrackMetaData(GUPnPService *service, char *variable, GValue *value, gpointer userData){
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->currentTrackMetaData);
}


G_MODULE_EXPORT void 
queryAVTransportURI(GUPnPService *service, char *variable, GValue *value, gpointer userData) {
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->avTransportURI);
}

G_MODULE_EXPORT void 
queryAVTransportURIMetaData(GUPnPService *service, char *variable, GValue *value, gpointer userData) {
	UpnpAVTransportServicePrivate *priv = UPNP_AVTRANSPORT_SERVICE_GET_PRIVATE(UPNP_AVTRANSPORT_SERVICE(userData));

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, priv->avTransportURIMetaData);
}

gint64 time_from_string (gchar* str) {
	gint64 result = 0LL;
	guint64 hours = 0ULL;
	guint64 minutes = 0ULL;
	guint64 seconds = 0ULL;
	const gchar* _tmp0_;
	g_return_val_if_fail (str != NULL, 0LL);
	_tmp0_ = str;
	sscanf (_tmp0_, "%llu:%2llu:%2llu%*s", &hours, &minutes, &seconds);
	result = ((gint64) (((hours * 3600) + (minutes * 60)) + seconds)) * GST_SECOND;
	return result;
}

