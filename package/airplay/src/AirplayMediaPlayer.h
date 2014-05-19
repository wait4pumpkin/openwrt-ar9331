#ifndef _AIRPLAY_MEDIA_PLAYER_H
#define _AIRPLAY_MEDIA_PLAYER_H

#include <gst/gst.h>

#define AIRPLAY_TYPE_MEDIA_PLAYER (airplayMediaPlayer_get_type())
#define AIRPLAY_MEDIA_PLAYER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), AIRPLAY_TYPE_MEDIA_PLAYER, AirplayMediaPlayer))
#define AIRPLAY_IS_MEDIA_PLAYER(obj) G_TYPE_CHECK_INSTANCE_TYPE((obj), AIRPLAY_TYPE_MEDIA_PLAYER))
#define AIRPLAY_MEDIA_PLAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), AIRPLAY_TYPE_MEDIA_PLAYER, AirplayMediaPlayerClass))
#define AIRPLAY_IS_MEDIA_PLAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), AIRPLAY_MEDIA_PLAYER))
#define AIRPLAY_MEDIA_PLAYER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), AIRPLAY_TYPE_MEDIA_PLAYER, AirplayMediaPlayerClass))

typedef struct _AirplayMediaPlayer AirplayMediaPlayer;
struct _AirplayMediaPlayer {
	GObject parent;
};
 
typedef struct _AirplayMediaPlayerClass AirplayMediaPlayerClass;
struct _AirplayMediaPlayerClass {
	GObjectClass parentClass;
};

GType airplayMediaPlayer_get_type(void);
void airplayMediaPlayerConnect(AirplayMediaPlayer *self, gchar *signal, GCallback callback, gpointer userData);
void airplayMediaPlayerSetURI(AirplayMediaPlayer *self, gchar *uri);
void airplayMediaPlayerPlay(AirplayMediaPlayer *self);
void airplayMediaPlayerPause(AirplayMediaPlayer *self);
void airplayMediaPlayerStop(AirplayMediaPlayer *self);
void airplayMediaPlayerMute(AirplayMediaPlayer *self, gboolean mute);
void airplayMediaPlayerSetVolume(AirplayMediaPlayer *self, gdouble volume);

#endif
