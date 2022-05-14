#include <config.h>
#include <math.h> /* sin, cos, tan, csin, ccos, ctan */
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/player/player.h>
#include <math.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <sys/time.h>
#include "gnome-voice-vosc.h"

/* Ideal Oscillator */

ClutterActor *
gnome_voice_new (void)
{
	/* return g_object_new (GNOME_RADIO_TYPE_VIEW, NULL); */
}

VOSCWindow *
gnome_voice_vosc (GtkWidget *window, struct timeval *tv, struct timezone *tz, ClutterActor *actor, GstPlayer *player)
{
	VOSCWindow *vosc;
}

ClutterActor *
gnome_voice_idea (VOSCWindow *window, GstPlayer *player, ClutterActor *oscilloscope)
{
	return window->actor;
}

/* Real-Time Oscillator */

VOSCWindow *
gnome_voice_real (GstPlayer *player, ClutterActor *oscilloscope)
{
	VOSCWindow *vosc;
	return vosc;
}

ClutterActor *
gnome_voice_plot (GFunc *vosc, GFunc *real)
{	
	ClutterActor *actor;
	return (actor);
}

void
gnome_voice_free (VOSCWindow *vosc)
{
	g_free (vosc);
}
