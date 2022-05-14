#ifndef GNOME_VOICE_FILE_H
#define GNOME_VOICE_FILE_H 1

#include <glib.h>
#include <gtk/gtk.h>
#include <gst/player/player.h>

typedef struct _LocationInfo LocationInfo;
typedef struct _VoiceInfo VoiceInfo;
typedef struct _StreamInfo StreamInfo;
typedef struct _VoiceWindow VoiceWindow;
typedef struct _VoiceOscilloscope VoiceOscilloscope;

struct _VoiceOscilloscope {
	GtkWidget *window;
        struct timeval *tv;
        struct timezone *tz;
        GstPlayer *player;
	gchar *window_title;
	gchar *text;
	gchar *uri;
};

struct _VoiceWindow {
	GtkWidget *window;
	gchar *window_title;
	gchar *text;
	gchar *uri;
};

struct _LocationInfo {
	double *lat;
	double *lon;
	gchar *city;
};

struct _VoiceInfo {
	gchar *uri;
	LocationInfo *location;
	StreamInfo *stream;
	VoiceInfo *next;
	VoiceInfo *prev;
};

struct _StreamInfo {
	gchar *uri;
	StreamInfo *next;
	StreamInfo *prev;	
};

#endif /* GNOME_VOICE_FILE_H */
