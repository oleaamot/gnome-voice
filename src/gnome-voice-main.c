#include <config.h>
#include <gtk/gtk.h>
#include <gst/player/player.h>
#include <champlain/champlain.h>
#include <math.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include "gnome-voice-file.h"
#include "gnome-voice-vosc.h"
#define VOICE_MARKER_SIZE 10

static void
on_clicked (ClutterClickAction *action, ClutterActor *actor, gpointer user_data) {
  /* printf ("Clutter Station marker clicked\n"); */
        return;
}

#if 0
static void
voice_window_init (VoiceWindow *window)
{
	gtk_widget_init_template (GTK_WIDGET (window));
}

static void
voice_window_class_init (VoiceWindowClass *class)
{
	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class),
						     "/org/gtk/gnome-voice/window.ui");
}

static void
search_text_changed (GtkEntry *entry, VoiceWindow *window)
{
	VoiceWindow *priv;
	const gchar *text;
	GtkWidget *tab;
	GtkWidget *view;
	GtkTextBuffer *buffer;
	GtkTextIter start, match_start, match_end;
	text = gtk_entry_get_text (entry);
	if (text[0] == '\0')
		return;
	priv = voice_window_get_instance_private (window);
	tab = gtk_stack_get_visible_child (GTK_STACK (priv->stack));
	view = gtk_bin_get_child (GTK_BIN (tab));
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	/* Very simple-minded search implementation */
	gtk_text_buffer_get_start_iter (buffer, &start);
	if (gtk_text_iter_forward_search (&start, text, GTK_TEXT_SEARCH_CASE_INSENSITIVE,
					  &match_start, &match_end, NULL))
	{
		gtk_text_buffer_select_range (buffer, &match_start, &match_end);
		gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (view), &match_start,
					      0.0, FALSE, 0.0, 0.0);
	}
}

static void
init_voice_window (VoiceWindow *window) {
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), voice_search_changed);
}
#endif

/*
  gnome-voice draws the voice_marker wth Cairo composed of 1 static
  filled circle and 1 stroked circle animated as echo.
 */
static ClutterActor *
create_voice_marker (void)
{
	ClutterActor *voice_marker;
	ClutterActor *bg;
	ClutterTimeline *timeline;
	cairo_t *cr;
	ClutterAction *action;
	/* Create the marker */
	voice_marker = champlain_custom_marker_new ();
	action = clutter_click_action_new ();
	/* Static filled circle ------------------------------------------ */
	bg = clutter_cairo_texture_new (VOICE_MARKER_SIZE, VOICE_MARKER_SIZE);
	cr = clutter_cairo_texture_create (CLUTTER_CAIRO_TEXTURE (bg));
	cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cr);
	cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
	/* Draw the circle */
	cairo_set_source_rgb (cr, 0, 0, 0);
	cairo_arc (cr, VOICE_MARKER_SIZE / 2.0,
		   VOICE_MARKER_SIZE / 2.0,
		   VOICE_MARKER_SIZE / 2.0, 0, 2 * M_PI);
	cairo_close_path (cr);
	/* Fill the circle */
	cairo_set_source_rgba (cr, 0.1, 0.9, 0.1, 1.0);
	cairo_fill (cr);
	cairo_destroy (cr);
	/* Add the circle to the voice_marker */
	clutter_container_add_actor (CLUTTER_CONTAINER (voice_marker), bg);
	clutter_actor_set_anchor_point_from_gravity (bg, CLUTTER_GRAVITY_CENTER);
	clutter_actor_set_position (bg, 0, 0);
	/* Echo circle ----------------------------------------------- */
	bg = clutter_cairo_texture_new (2 * VOICE_MARKER_SIZE,
					2 * VOICE_MARKER_SIZE);
	cr = clutter_cairo_texture_create (CLUTTER_CAIRO_TEXTURE (bg));
	/* Draw the circle */
	cairo_set_source_rgb (cr, 0, 0, 0);
	cairo_arc (cr, VOICE_MARKER_SIZE, VOICE_MARKER_SIZE,
		   0.9 * VOICE_MARKER_SIZE, 0, 2 * M_PI);
	cairo_close_path (cr);
	/* Stroke the circle */
	cairo_set_line_width (cr, 2.0);
	cairo_set_source_rgba (cr, 0.1, 0.7, 0.1, 1.0);
	cairo_stroke (cr);
	cairo_destroy (cr);
	/* Add the circle to the voice_marker */
	clutter_container_add_actor (CLUTTER_CONTAINER (voice_marker), bg);
	clutter_actor_lower_bottom (bg); /* Ensure it is under the previous circle */
	clutter_actor_set_position (bg, 0, 0);
	clutter_actor_set_anchor_point_from_gravity (bg,
						     CLUTTER_GRAVITY_CENTER);
	/* Animate the echo circle */
	timeline = clutter_timeline_new (1000);
	clutter_timeline_set_loop (timeline, TRUE);
	clutter_actor_set_opacity (CLUTTER_ACTOR (bg), 255);
	clutter_actor_set_scale (CLUTTER_ACTOR (bg), 0.5, 0.5);
	clutter_actor_animate_with_timeline (CLUTTER_ACTOR (bg),
					     CLUTTER_EASE_OUT_SINE,
					     timeline,
					     "opacity", 0,
					     "scale-x", 2.0,
					     "scale-y", 2.0,
					     NULL);
	clutter_actor_add_action (CLUTTER_ACTOR (voice_marker), CLUTTER_ACTION (action));
	g_signal_connect (CLUTTER_ACTION (action), "clicked", G_CALLBACK (on_clicked), NULL);
	clutter_timeline_start (timeline);
	return voice_marker;
}

double lat = 21.293352;
double lon = -157.839583;

typedef struct
{
	ChamplainView *view;
	ChamplainMarker *voice_marker;
} GpsCallbackData;

typedef struct
{
        VoiceOscilloscope *oscilloscope_visual;
} OscilloscopeCallbackData;

static gboolean
gps_callback (GpsCallbackData *data)
{
	champlain_view_center_on (data->view, lat, lon);
	champlain_location_set_location (CHAMPLAIN_LOCATION (data->voice_marker), lat, lon);
	return TRUE;
}

gint
main (gint argc, gchar **argv)
{
	GstPlayer *player;
	GtkWidget *window;
	ChamplainView *view;
	ClutterActor *actor, *voice_oscilloscope, *voice_marker, *oscilloscope_visual, *stage;
	ChamplainMarkerLayer *layer;
	VoiceInfo *voiceinfo;	
	GpsCallbackData callback_data;
	GstElement *src, *conv, *enc, *muxer, *sink, *pipeline;
	/* OscilloscopeCallbackData oscilloscope_data; */
	VOSCWindow *vosc;
	GMainLoop *main_loops;
	gchar *filename;

	gst_init(NULL, NULL);
	pipeline = gst_pipeline_new("record_pipe");

	src = gst_element_factory_make("autoaudiosrc", "auto_source");
	conv = gst_element_factory_make("audioconvert", "convert");
	enc = gst_element_factory_make("vorbisenc", "vorbis_enc");
	muxer = gst_element_factory_make("oggmux", "oggmux");
	sink = gst_element_factory_make("filesink", "sink");
	filename = g_strconcat("file://", g_get_host_name(), g_get_user_special_dir(G_USER_DIRECTORY_MUSIC), "/GNOME.ogg", NULL);
	g_object_set(G_OBJECT(sink), "location",
		     g_strconcat(g_get_user_special_dir(G_USER_DIRECTORY_MUSIC), "/GNOME.ogg", NULL));
	/* g_object_set(G_OBJECT(enc), "quality", 1.0); */
	gst_bin_add_many(GST_BIN(pipeline), src, conv, enc, muxer, sink, NULL);
	gst_element_link_many(src, conv, enc, muxer, sink, NULL);

	gst_element_set_state(pipeline, GST_STATE_PLAYING);
	gtk_init(&argc, &argv);
	main_loops = g_main_loop_new(NULL, TRUE);
  
	if (gtk_clutter_init (&argc, &argv) != CLUTTER_INIT_SUCCESS)
		return 1;
	vosc = (VOSCWindow *)g_new0(VOSCWindow, 1);
        /* Visual Oscillator */
        vosc->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        vosc->vbox = gtk_vbox_new (TRUE, 0);
        vosc->streaminghistory = gtk_entry_buffer_new ("http://api.perceptron.stream:8000/56.ogg", 8196);
        vosc->streaminggram = gtk_entry_new_with_buffer (vosc->streaminghistory);
        vosc->streaminglabel = gtk_label_new ("Voicegram Streaming URL:");
        vosc->recordinghistory = gtk_entry_buffer_new (filename, 8196);
        vosc->recordinggram = gtk_entry_new_with_buffer (vosc->recordinghistory);
        vosc->recordinglabel = gtk_label_new ("Voicegram Recording URL:");
        gtk_window_set_title(GTK_WINDOW (vosc->window), "Voicegram");
	gtk_window_set_default_size(GTK_WINDOW (vosc->window), 800, 20);
	gtk_window_set_keep_above(GTK_WINDOW(vosc->window), TRUE);
        gtk_container_add(GTK_CONTAINER (vosc->vbox), GTK_ENTRY(vosc->streaminglabel));
        gtk_container_add(GTK_CONTAINER (vosc->vbox), GTK_ENTRY(vosc->streaminggram));
        gtk_container_add(GTK_CONTAINER (vosc->vbox), GTK_ENTRY(vosc->recordinglabel));
        gtk_container_add(GTK_CONTAINER (vosc->vbox), GTK_ENTRY(vosc->recordinggram));
        gtk_container_add(GTK_CONTAINER (vosc->window), GTK_VBOX(vosc->vbox));
        // voice_window_init (GTK_WINDOW (vosc->window));
        gtk_widget_show_all (vosc->window);
	stage = clutter_stage_new ();
	clutter_stage_set_title (stage, g_strconcat(PACKAGE, " ", VERSION, " - ", "http://www.gnomevoice.org/", " - ", "https://wiki.gnome.org/Apps/Voice", NULL));
	clutter_actor_set_size (stage, 800, 600);
	g_signal_connect (stage, "destroy", G_CALLBACK (clutter_main_quit), NULL);
	/* Create the map view */
	actor = champlain_view_new ();
	clutter_actor_set_size (CLUTTER_ACTOR (actor), 800, 600);
	clutter_container_add_actor (CLUTTER_CONTAINER (stage), actor);
	/* Create the voice_marker layer */
	layer = champlain_marker_layer_new_full (CHAMPLAIN_SELECTION_SINGLE);
	clutter_actor_show (CLUTTER_ACTOR (layer));
	champlain_view_add_layer (CHAMPLAIN_VIEW (actor), CHAMPLAIN_LAYER (layer));
	/* Create a voice_marker */
	voice_marker = create_voice_marker ();
	champlain_marker_layer_add_marker (layer, CHAMPLAIN_MARKER (voice_marker));
	/* Create a oscilloscope_visual */
	/* oscilloscope_visual = create_oscilloscope_visual (); */
        /* gnome_voice_add_visual_oscilloscope (layer, GNOME_VOICE_MARKER (oscilloscope_visual)); */
	/* Finish initialising the map view */
	g_object_set (G_OBJECT (actor), "zoom-level", 1,
		      "kinetic-mode", TRUE, NULL);
	champlain_view_center_on (CHAMPLAIN_VIEW (actor), lat, lon);
	/* Create callback that updates the map periodically */
	callback_data.view = CHAMPLAIN_VIEW (actor);
	callback_data.voice_marker = CHAMPLAIN_MARKER (voice_marker);
	/* oscilloscope_data.view = GNOME_VOICE_VIEW (voice_oscilloscope); */
        /* oscilloscope_data.oscilloscope_visual = GNOME_VOICE_MARKER (oscilloscope_visual); */
	/* Create the voice player */
	player = gst_player_new (NULL, gst_player_g_main_context_signal_dispatcher_new(NULL));
	/* gnome_voice_file_loader (voiceinfo, "gnome-voice.xml"); */
	gst_player_set_uri (GST_PLAYER (player), gtk_entry_get_text(vosc->streaminggram));
	gst_player_stop (GST_PLAYER (player));
	/* Visual Oscillator */
        /* vosc->window = gtk_window_new (GTK_WINDOW_TOPLEVEL); */
        /* gtk_widget_show_all (vosc->window); */
        /* gnome_voice_real(GST_PLAYER (player), CLUTTER_ACTOR (voice_oscilloscope)); */
        /* clutter_container_add_actor (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (voice_oscilloscope)); */
	gst_player_play(GST_PLAYER (player));
	g_timeout_add (1000, (GSourceFunc) gps_callback, &callback_data);
	/* g_timeout_add (1000, (GSourceFunc) gnome_voice_real, &oscilloscope_data); */
	clutter_actor_show (stage);
        /* clutter_actor_show (voice_oscilloscope); */
	clutter_main ();
	g_main_loop_run(main_loops);

	return (0);
}
