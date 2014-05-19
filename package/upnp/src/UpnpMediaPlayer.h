#ifndef _UPNP_MEDIA_PLAYER_H
#define _UPNP_MEDIA_PLAYER_H

#include <libgupnp/gupnp.h>
#include <gst/gst.h>

#define UPNP_TYPE_MEDIA_PLAYER (upnpMediaPlayer_get_type())
#define UPNP_MEDIA_PLAYER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), UPNP_TYPE_MEDIA_PLAYER, UpnpMediaPlayer))
#define UPNP_IS_MEDIA_PLAYER(obj) G_TYPE_CHECK_INSTANCE_TYPE((obj), UPNP_TYPE_MEDIA_PLAYER))
#define UPNP_MEDIA_PLAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), UPNP_TYPE_MEDIA_PLAYER, UpnpMediaPlayerClass))
#define UPNP_IS_MEDIA_PLAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), UPNP_MEDIA_PLAYER))
#define UPNP_MEDIA_PLAYER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), UPNP_TYPE_MEDIA_PLAYER, UpnpMediaPlayerClass))

typedef struct _UpnpMediaPlayer UpnpMediaPlayer;
struct _UpnpMediaPlayer {
	GObject parent;
};
 
typedef struct _UpnpMediaPlayerClass UpnpMediaPlayerClass;
struct _UpnpMediaPlayerClass {
	GObjectClass parentClass;
};

GType upnpMediaPlayer_get_type(void);
void upnpMediaPlayerSetURI(UpnpMediaPlayer *self, gchar *uri);
void upnpMediaPlayerPlay(UpnpMediaPlayer *self);
void upnpMediaPlayerStop(UpnpMediaPlayer *self);
void upnpMediaPlayerPause(UpnpMediaPlayer *self);
GstState upnpMediagetstate(UpnpMediaPlayer *self) ;
void upnpMediaPlayerSetMute(UpnpMediaPlayer *self, gboolean mute);
void upnpMediaPlayerSetVolume(UpnpMediaPlayer *self, gdouble volume);
void upnpMediaPlayerSetLoudness(UpnpMediaPlayer *self, gboolean loudness);
gint64 upnpMediaPlayerQueryDuration(UpnpMediaPlayer *self);
gint64 upnpMediaPlayerQueryPosition(UpnpMediaPlayer *self);
void upnpMediaPlayerSeek(UpnpMediaPlayer *self, gint64 time);
void upnpMediaPlayerEnable(UpnpMediaPlayer *self, gboolean enable);

#endif
