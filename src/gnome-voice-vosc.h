#ifndef GNOME_VOICE_VOSC_H
#define GNOME_VOICE_VOSC_H 1

#include <glib.h>
#include <gtk/gtk.h>
#include <gst/player/player.h>
#include <champlain/champlain.h>
#define VOICE_VISUAL_HEIGHT 300
#define VOICE_VISUAL_LENGTH 300

typedef struct _VOSCWindow VOSCWindow;

struct _VOSCWindow {
        GtkWidget *window;
        GtkEntryBuffer *streaminghistory;
        GtkEntry *streaminggram;
        GtkLabel *streaminglabel;
        GtkEntryBuffer *recordinghistory;
        GtkEntry *recordinggram;
        GtkLabel *recordinglabel;
        GtkVBox *vbox;
        GtkStack *stack;
        struct timeval *tv;
	struct timezone *tz;
        ClutterActor *actor;
        GstPlayer *player;
};

ClutterActor *gnome_voice_idea (VOSCWindow *window, GstPlayer *player, ClutterActor *oscilloscope);
VOSCWindow *gnome_voice_real (GstPlayer *player, ClutterActor *oscilloscope);
ClutterActor *gnome_voice_plot (GFunc *vosc, GFunc *real);
void gnome_voice_free (VOSCWindow *vosc);

#endif /* GNOME_VOICE_VOSC_H */
