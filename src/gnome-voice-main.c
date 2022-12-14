#include <config.h>
#include <gtk/gtk.h>
#include <gst/player/player.h>
#include <champlain/champlain.h>
#include <math.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>
#include <string.h>
#include <geoclue.h>
#include <champlain-gtk/champlain-gtk.h>
#include <clutter-gtk/clutter-gtk.h>

#include "gnome-voice-file.h"
#include "gnome-voice-vosc.h"
#include "gnome-voice-main.h"

#define VOICE_MARKER_SIZE 10

GClueSimple *simple = NULL;
GClueClient *client = NULL;
GMainLoop *main_loop;
GClueLocation *location = NULL;
GtkWidget *title_entry;
GtkWidget *title_label;
GtkWidget *filename_entry;
GtkWidget *filename_label;
GtkWidget *summary_entry;
GtkWidget *summary_label;

#define N_COLS 2
#define COL_ID 0
#define COL_NAME 1

static gboolean
on_location_timeout (gpointer user_data)
{
        g_clear_object (&client);
        g_clear_object (&simple);
        g_main_loop_quit (main_loop);

        return FALSE;
}

static void
on_client_active_notify (GClueClient *client,
                         GParamSpec *pspec,
                         gpointer    user_data)
{
        if (gclue_client_get_active (client))
                return;

        g_print ("Geolocation disabled. Quitting..\n");
        on_location_timeout (NULL);
}

static void
on_simple_ready (GObject      *source_object,
                 GAsyncResult *res,
                 gpointer      user_data)
{
        GError *error = NULL;
	GClueSimple *simple = NULL;
        simple = gclue_simple_new_finish (res, &error);
        if (error != NULL) {
            g_critical ("Failed to connect to GeoClue2 service: %s", error->message);

            exit (-1);
        }
        client = gclue_simple_get_client (simple);
        if (client) {
                g_object_ref (client);
                g_print ("Client object: %s\n",
                         g_dbus_proxy_get_object_path (G_DBUS_PROXY (client)));

		g_signal_connect (client,
				  "notify::active",
				  G_CALLBACK (on_client_active_notify),
				  NULL);
        }
        g_signal_connect (simple,
                          "notify::location",
                          G_CALLBACK (gps_callback),
                          user_data);
}

static void
on_clicked (ClutterClickAction *action, ClutterActor *actor, gpointer user_data) {

        printf ("Clutter Voice marker clicked\n");
        return;
}


static void
map_source_changed (GtkWidget *widget,
		    ChamplainView *view)
{
	gchar *id;
	ChamplainMapSource *source;
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget), &iter))
		return;

	model = gtk_combo_box_get_model (GTK_COMBO_BOX (widget));

	gtk_tree_model_get (model, &iter, COL_ID, &id, -1);

	ChamplainMapSourceFactory *factory = champlain_map_source_factory_dup_default ();
	source = champlain_map_source_factory_create_cached_source (factory, id);
	g_object_set (G_OBJECT (view), "map-source", source, NULL);
	g_object_unref (factory);
}

static void
zoom_changed (GtkSpinButton *spinbutton,
	      ChamplainView *view)
{
	gint zoom = gtk_spin_button_get_value_as_int (spinbutton);

	g_object_set (G_OBJECT (view), "zoom-level", zoom, NULL);
}


/* Commandline options */
static gint timeout = 3600; /* seconds */
static GClueAccuracyLevel accuracy_level = GCLUE_ACCURACY_LEVEL_EXACT;
static gint time_threshold;
GMainLoop *main_loops;

static GOptionEntry entries[] =
{
        { "timeout",
          't',
          0,
          G_OPTION_ARG_INT,
          &timeout,
          N_("Exit after T seconds. Default: 3600"),
          "T" },
        { "time-threshold",
          'i',
          0,
          G_OPTION_ARG_INT,
          &time_threshold,
          N_("Only report location update after T seconds. "
             "Default: 0 (report new location without any delay)"),
          "T" },
        { "accuracy-level",
          'a',
          0,
          G_OPTION_ARG_INT,
          &accuracy_level,
          N_("Request accuracy level A. "
             "Country = 1, "
             "City = 4, "
             "Neighborhood = 5, "
             "Street = 6, "
             "Exact = 8."),
          "A" },
        { NULL }
};

static void
on_clicked_voicegram (ClutterClickAction *action, ClutterActor *actor, gpointer user_data) {
        printf ("Clutter Voicegram clicked\n");
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
	text = gtk_entry_get_text (filename_entry);
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

/*
  gnome-voice draws the voice_marker wth Cairo composed of 1 static
  filled circle and 1 stroked circle animated as echo.
 */
static ClutterActor *
create_voicegram (void)
{
	ClutterActor *voicegram;
	ClutterActor *bg;
	ClutterTimeline *timeline;
	cairo_t *cr;
	ClutterAction *action;
	/* Create the marker */
	voicegram = champlain_custom_marker_new ();
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
	cairo_set_source_rgba (cr, 0.9, 0.1, 0.1, 1.0);
	cairo_fill (cr);
	cairo_destroy (cr);
	/* Add the circle to the voicegram */
	clutter_container_add_actor (CLUTTER_CONTAINER (voicegram), bg);
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
	cairo_set_source_rgba (cr, 0.7, 0.1, 0.1, 1.0);
	cairo_stroke (cr);
	cairo_destroy (cr);
	/* Add the circle to the voice_marker */
	clutter_container_add_actor (CLUTTER_CONTAINER (voicegram), bg);
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
	clutter_actor_add_action (CLUTTER_ACTOR (voicegram), CLUTTER_ACTION (action));
	g_signal_connect (CLUTTER_ACTION (action), "clicked", G_CALLBACK (on_clicked_voicegram), NULL);
	clutter_timeline_start (timeline);
	return voicegram;
}

gboolean
gps_callback (GClueSimple *simple, GpsCallbackData *data)
{

        GError **error = NULL;
	gdouble lat, lon;
	ClutterColor city_color = { 0x9a, 0x9b, 0x9c, 0x9d };
	ClutterColor text_color = { 0xff, 0xff, 0xff, 0xff };
	const char *name, *name_city, *name_country;
	/* GeocodeForward *fwd; */
	/* GList *list; */

        gdouble altitude, speed, heading;
        GVariant *timestamp;
        GTimeVal tv = { 0 };
        const char *desc;

	lat_gps = gclue_location_get_latitude (location);
	lon_gps = gclue_location_get_longitude (location);

	champlain_view_center_on (CHAMPLAIN_VIEW (data->view), lat_gps, lon_gps);
	champlain_location_set_location (CHAMPLAIN_LOCATION (data->voice_marker), lat_gps, lon_gps);

	return TRUE;
}

typedef struct {
	GtkWidget *widget;
	gint index;
	const gchar *title;
	GtkAssistantPageType type;
	gboolean complete;
} PageInfo;

static void gv_wizard_entry_changed(GtkEditable * editable,
				       GtkAssistant * assistant,
				       GstElement * pipeline)
{
	return;
}

static void gv_wizard_button_toggled(GtkCheckButton * checkbutton,
					GtkAssistant * assistant)
{
	return;
}

static void gv_wizard_button_clicked(GtkButton * button,
					GtkAssistant * assistant)
{
	GstElement *src, *conv, *enc, *muxer, *sink, *recorder;
	gchar *filename = NULL;
	GDateTime *datestamp = g_date_time_new_now_utc ();
	GstElementFactory *factory;
	GTimeVal *timeval;
	gst_element_send_event(recorder, gst_event_new_eos());
	recorder = gst_pipeline_new("record_pipe");
	/*
	  FIXME: Line #59 from https://github.com/GStreamer/gst-plugins-base/blob/master/tools/gst-device-monitor.c
	  element = gst_device_create_element (device, NULL);
	  if (!element)
	  return NULL;
	  factory = gst_element_get_factory (element);
	  if (!factory) {
	  gst_object_unref (element);
	  return NULL;
	  }
	  src = gst_element_factory_create(factory, NULL);
	*/
	src = gst_element_factory_make("autoaudiosrc", "auto_source");
	conv = gst_element_factory_make("audioconvert", "convert");
	enc = gst_element_factory_make("vorbisenc", "vorbis_enc");
	muxer = gst_element_factory_make("oggmux", "oggmux");
	sink = gst_element_factory_make("filesink", "sink");
	filename = g_strconcat(g_get_user_special_dir(G_USER_DIRECTORY_MUSIC), "/",
			       "GNOME.ogg", NULL);
	g_object_set(G_OBJECT(sink), "location",
		     g_strconcat(g_get_user_special_dir(G_USER_DIRECTORY_MUSIC), "/",
				 "GNOME.ogg", NULL), NULL);
	g_object_set(G_OBJECT(enc), "quality", 1.0);
	gst_bin_add_many(GST_BIN(recorder), src, conv, enc, muxer, sink, NULL);
	gst_element_link_many(src, conv, enc, muxer, sink, NULL);
	gst_element_set_state(recorder, GST_STATE_PLAYING);
	datestamp = g_date_time_new_now_utc ();
        gst_tag_setter_add_tags (GST_TAG_SETTER (enc),
                                 GST_TAG_MERGE_APPEND,
                                 GST_TAG_TITLE, gtk_entry_get_text(GTK_ENTRY(title_entry)),
                                 GST_TAG_ARTIST, g_get_real_name(),
                                 GST_TAG_ALBUM, "Voicegrams",
                                 GST_TAG_COMMENT, "GNOME 43",
                                 GST_TAG_DATE, g_date_time_format_iso8601 (datestamp),
                                 NULL);
	g_date_time_unref (datestamp);
	main_loops = g_main_loop_new(NULL, TRUE);
	g_main_loop_run(main_loops);
	gst_element_set_state(recorder, GST_STATE_NULL);
	g_main_loop_unref(main_loops);
	gst_object_unref(GST_OBJECT(recorder));
	g_date_time_unref (datestamp);
}

static void gv_wizard_cancel(GtkAssistant * assistant, gpointer data)
{
	if (!main_loops) {
		g_error("Quit more loops than there are.");
	} else {
		GMainLoop *loop = main_loops;
		g_main_loop_quit(loop);
		gtk_main_quit();
	}
}

static void gv_wizard_close(GtkAssistant * assistant, gpointer data)
{
	FILE *voice_pointer = NULL;
	VoiceInfo *voiceinfo = (VoiceInfo *) data;
	GDateTime *datestamp = g_date_time_new_now_utc ();
	gchar *filename_voice =
	    g_strconcat(g_get_user_special_dir(G_USER_DIRECTORY_MUSIC), "/",
			"GNOME.voice", NULL);
	voice_pointer = fopen(filename_voice, "w");
	fprintf(voice_pointer, "<voice version='%s'>\n", VERSION);
	fprintf(voice_pointer, "  <station name='%s' uri='http://%s/GNOME.ogg'>\n",
		g_get_real_name(),
		g_get_host_name());
#if 0
	fprintf(voice_pointer, "    <location lat='%s' lon='%s'>%s</location>\n",
		voiceinfo->location->lat,
		voiceinfo->location->lon,
		voiceinfo->location->city);
#endif
	fprintf(voice_pointer, "    <stream>http://%s%s</stream>\n", g_get_host_name(), "/GNOME.ogg");
	fprintf(voice_pointer, "  </station>\n");
	fprintf(voice_pointer, "</voice>\n");
	fclose(voice_pointer);
	g_date_time_unref (datestamp);
	gst_element_send_event(data, gst_event_new_eos());
}

static void gv_wizard_apply(GtkAssistant * assistant, gpointer data)
{
        GVoiceCfg *config;
        GtkWindow *window;
        /* gtk_init (&argc, &argv); */
        /* config = main_config (GTK_WIDGET(window), gtk_entry_get_text(GTK_ENTRY(title_entry))); */
        /* window = main (config); */
        /* gtk_widget_show_all (window); */
        /* gst_init(&argc, &argc); */
        /* gtk_main(); */
	/* gst_element_send_event(data, gst_event_new_eos()); */
}

GtkAssistantPageFunc gv_wizard_cb(GtkAssistant * assistant,
				  GDateTime * datestamp)
{
	/* gtk_assistant_next_page(assistant); */
}


gint
main (gint argc, gchar **argv)
{
	GstPlayer *player;
	GtkWidget *window;
	GVoiceCfg *config;
	ChamplainView *view;
	ClutterActor *actor, *second, *voice_oscilloscope, *voice_marker, *voicegram, *oscilloscope_visual, *stage, *wizard;
	ChamplainMarkerLayer *layer;
	ChamplainMarkerLayer *world;
	VoiceInfo *voiceinfo;
	GpsCallbackData callback_data;
	GetVoicegramData voicegram_data;
	GstElement *src, *conv, *enc, *muxer, *sink, *pipeline;
	/* OscilloscopeCallbackData oscilloscope_data; */
	/* VOSCWindow *vosc; */
	GMainLoop *main_loops;
	gchar *filename;
	GTimeVal *timeval;
	GDateTime *datestamp;
	guint context_id;
	/* GClueLocation *location; */
        gdouble altitude, speed, heading;
        GVariant *timestamp;
        GTimeVal tv = { 0 };
        const char *desc;
	gchar *voice_xml;
	GtkWidget *introduction;
	int i = 0;
	PageInfo page[5] = {
		{NULL, -1, "Voice 0.2.0", GTK_ASSISTANT_PAGE_INTRO, TRUE},
		{NULL, -1, "Title", GTK_ASSISTANT_PAGE_CONTENT, TRUE},
		{NULL, -1, "Filename", GTK_ASSISTANT_PAGE_CONTENT, TRUE},
		{NULL, -1, "Summary", GTK_ASSISTANT_PAGE_SUMMARY, TRUE},		
		{NULL, -1, "Complete", GTK_ASSISTANT_PAGE_CONFIRM, TRUE},		
	};
	// gtk_init(&argc, &argv);
	gtk_clutter_init(&argc, &argv);
	window = gtk_clutter_window_new ();
	introduction = gtk_assistant_new();
	gtk_container_add (GTK_WINDOW (window), introduction);
	gtk_widget_set_size_request(GTK_WIDGET(introduction), 640, 480);
	gtk_window_set_title(GTK_WINDOW(introduction), "Voice 0.2.0");
	g_signal_connect(G_OBJECT(introduction), "destroy",
			 G_CALLBACK(gtk_main_quit), NULL);
	page[0].widget = gtk_label_new(_("Welcome to Voice 0.2.0!\n\nRecord respectfully around others.\n\nClick Next to setup a voice recording session!\n\nClick Cancel to stop the voice recording session.\n\nClick Cancel twice to exit GNOME Voice."));
	page[1].widget = gtk_box_new(FALSE, 5);
	title_label = gtk_label_new(_("Title:"));
	title_entry = gtk_entry_new();
	if (g_strcmp0(title_entry, NULL)!=0) gtk_entry_set_text(GTK_ENTRY(title_entry), g_get_real_name()); else gtk_entry_set_text(GTK_ENTRY(title_entry), gtk_entry_get_text(GTK_ENTRY(title_entry)));
	gtk_box_pack_start(GTK_BOX(page[1].widget), GTK_WIDGET(title_label),
			   FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(page[1].widget), GTK_WIDGET(title_entry),
			   FALSE, FALSE, 5);
	page[2].widget = gtk_box_new(FALSE, 5);
	filename_label = gtk_label_new(_("Filename:"));
	filename_entry = gtk_entry_new();
	if (g_strcmp0(filename_entry, NULL)!=0) gtk_entry_set_text(GTK_ENTRY(filename_entry), g_strconcat ("GNOME.ogg", NULL)); else gtk_entry_set_text(GTK_ENTRY(filename_entry), gtk_entry_get_text(GTK_ENTRY(filename_entry)));
	gtk_box_pack_start(GTK_BOX(page[2].widget), GTK_WIDGET(filename_label),
			   FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(page[2].widget), GTK_WIDGET(filename_entry),
			   FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(filename_label), "clicked",
			 G_CALLBACK(gv_wizard_apply),
			 gtk_entry_get_text(GTK_ENTRY(filename_entry)));
	page[3].widget = gtk_box_new(FALSE, 5);
	summary_label = gtk_label_new(_("Recording"));
	summary_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(summary_entry), "Recording");
	gtk_box_pack_start(GTK_BOX(page[4].widget), GTK_WIDGET(summary_label),
			   FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(page[4].widget), GTK_WIDGET(summary_entry),
			   FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(summary_label), "clicked",
			 G_CALLBACK(gv_wizard_apply),
			 gtk_entry_get_text(GTK_ENTRY(summary_entry)));
	for (i = 0; i < 5; i++) {
	        page[i].index = gtk_assistant_append_page(GTK_ASSISTANT(introduction),
					      GTK_WIDGET(page[i].widget));
		gtk_assistant_set_page_title(GTK_ASSISTANT(introduction),
					     GTK_WIDGET(page[i].widget),
					     page[i].title);
		gtk_assistant_set_page_type(GTK_ASSISTANT(introduction),
					    GTK_WIDGET(page[i].widget),
					    page[i].type);
		gtk_assistant_set_page_complete(GTK_ASSISTANT(introduction),
						GTK_WIDGET(page[i].widget),
						page[i].complete);
	}
	voice_xml = g_strconcat (GNOME_VOICE_DATADIR, "/gnome-voice.xml", NULL);
	voiceinfo = (VoiceInfo *)g_new0 (VoiceInfo, 1);
#if 0	
	gnome_voice_file_loader (voiceinfo, voice_xml);
#endif
	g_signal_connect(G_OBJECT(filename_entry), "changed",
			 G_CALLBACK(gv_wizard_entry_changed), pipeline);
	g_signal_connect(G_OBJECT(introduction), "cancel",
			 G_CALLBACK(gv_wizard_cancel), main_loops);
	g_signal_connect(G_OBJECT(introduction), "close",
			 G_CALLBACK(gv_wizard_close), voiceinfo);
	g_signal_connect(G_OBJECT(introduction), "apply",
			 G_CALLBACK(gv_wizard_close), pipeline);
	gtk_widget_show_all (GTK_WIDGET (introduction));
	gst_init(&argc, &argv);
	gst_init(NULL, NULL);
	pipeline = gst_pipeline_new("record_pipe");

	src = gst_element_factory_make("autoaudiosrc", "auto_source");
	conv = gst_element_factory_make("audioconvert", "convert");
	enc = gst_element_factory_make("vorbisenc", "vorbis_enc");
	muxer = gst_element_factory_make("oggmux", "oggmux");
	sink = gst_element_factory_make("filesink", "sink");
	filename = g_strconcat("file://", g_get_host_name(), g_get_user_special_dir(G_USER_DIRECTORY_MUSIC), gtk_entry_get_text(filename_entry), NULL);
	g_object_set(G_OBJECT(sink), "location",
		     g_strconcat(g_get_user_special_dir(G_USER_DIRECTORY_MUSIC), "/", gtk_entry_get_text(filename_entry), NULL));
	gst_bin_add_many(GST_BIN(pipeline), src, conv, enc, muxer, sink, NULL);
	gst_element_link_many(src, conv, enc, muxer, sink, NULL);

	gst_element_set_state(pipeline, GST_STATE_PLAYING);
        datestamp = g_date_time_new_now_utc ();
	gst_tag_setter_add_tags (GST_TAG_SETTER (enc),
				 GST_TAG_MERGE_APPEND,
				 GST_TAG_TITLE, g_get_real_name(),
				 GST_TAG_ARTIST, g_get_real_name(),
				 GST_TAG_ALBUM, gtk_entry_get_text(filename_entry),
				 GST_TAG_COMMENT, "Voice 0.2.0",
				 GST_TAG_DATE, g_date_time_format_iso8601 (datestamp),
				 NULL);
	g_date_time_unref (datestamp);
	main_loops = g_main_loop_new(NULL, TRUE);
  
	if (clutter_init (&argc, &argv) != CLUTTER_INIT_SUCCESS)
		return 1;
	/* vosc = (VOSCWindow *)g_new0(VOSCWindow, 1); */
	stage = clutter_stage_new ();

	wizard = gtk_clutter_window_get_stage (GTK_WINDOW (window));
	clutter_stage_set_title (stage, g_strconcat(PACKAGE, " ", VERSION, " - ", "http://www.gnomevoice.org/", " - ", "https://wiki.gnome.org/Apps/Voice", NULL));
	clutter_actor_set_size (stage, 800, 600);
	g_signal_connect (stage, "destroy", G_CALLBACK (clutter_main_quit), NULL);
	/* Create the map view */
	actor = champlain_view_new ();
	clutter_actor_set_size (CLUTTER_ACTOR (actor), 800, 600);
	clutter_container_add_actor (CLUTTER_CONTAINER (stage), actor);
	second = champlain_view_new ();
	clutter_actor_set_size (CLUTTER_ACTOR (second), 80, 60);
	clutter_container_add_actor (CLUTTER_CONTAINER (stage), second);
	/* Create the voice_marker layer */
	layer = champlain_marker_layer_new_full (CHAMPLAIN_SELECTION_SINGLE);
	world = champlain_marker_layer_new_full (CHAMPLAIN_SELECTION_SINGLE);
	clutter_actor_show (CLUTTER_ACTOR (layer));
	clutter_actor_show (CLUTTER_ACTOR (world));
	champlain_view_add_layer (CHAMPLAIN_VIEW (actor), CHAMPLAIN_LAYER (layer));
	champlain_view_add_layer (CHAMPLAIN_VIEW (second), CHAMPLAIN_LAYER (world));
	/* Create a voice_marker */
	voice_marker = create_voice_marker ();
	champlain_marker_layer_add_marker (layer, CHAMPLAIN_MARKER (voice_marker));
	/* Create a voicegram */
	voicegram = create_voicegram ();
	champlain_marker_layer_add_marker (world, CHAMPLAIN_MARKER (voicegram));
#if 0
	/* Locate a voicegram */
#endif
	/* Create a oscilloscope_visual */
	/* oscilloscope_visual = create_oscilloscope_visual (); */
        /* gnome_voice_add_visual_oscilloscope (layer, GNOME_VOICE_MARKER (oscilloscope_visual)); */

#if 0
	gclue_simple_new ("gnome-voice",
			  accuracy_level,
			  time_threshold,
			  on_simple_ready,
			  CHAMPLAIN_VIEW (view));
	
	location = gclue_simple_get_location (GCLUE_SIMPLE(simple));
#endif
	/* Finish initialising the map view */
	g_object_set (G_OBJECT (actor), "zoom-level", 1,
		      "kinetic-mode", TRUE, NULL);
#if 0
	champlain_view_center_on (CHAMPLAIN_VIEW (actor), (gdouble)*voiceinfo->location->lat, (gdouble)*voiceinfo->location->lon);
#endif
	champlain_view_center_on (CHAMPLAIN_VIEW (actor), lat_gps, lon_gps);
	g_object_set (G_OBJECT (second), "zoom-level", 6,
		      "kinetic-mode", TRUE, NULL);
#if 0
	champlain_view_center_on (CHAMPLAIN_VIEW (second), (gdouble)*voiceinfo->location->lat, (gdouble)*voiceinfo->location->lon);
#endif
	champlain_view_center_on (CHAMPLAIN_VIEW (second), lat_gps, lon_gps);
	/* Create callback that updates the map periodically */
	callback_data.view = CHAMPLAIN_VIEW (actor);
	callback_data.voice_marker = CHAMPLAIN_MARKER (voice_marker);
	voicegram_data.view = CHAMPLAIN_VIEW (second);
	voicegram_data.voicegram = CHAMPLAIN_MARKER (voicegram);
	/* oscilloscope_data.view = GNOME_VOICE_VIEW (voice_oscilloscope); */
        /* oscilloscope_data.oscilloscope_visual = GNOME_VOICE_MARKER (oscilloscope_visual); */
	/* Create the voice player */
	player = gst_player_new (NULL, gst_player_g_main_context_signal_dispatcher_new(NULL));
#if 0
	gst_player_set_uri (GST_PLAYER (player), (gchar*)voiceinfo->stream->uri);
#endif
	gst_player_set_uri (GST_PLAYER (player), "http://api.perceptron.stream:8000/56.ogg");	
	gst_player_stop (GST_PLAYER (player));
	gst_player_play(GST_PLAYER (player));
#if 0
        g_timeout_add (3600000, (GSourceFunc) on_simple_ready, &callback_data);
	g_timeout_add (3600000, (GSourceFunc) on_simple_ready, &voicegram_data);
#endif
	clutter_container_add_actor (CLUTTER_CONTAINER (voicegram), wizard);
	clutter_actor_show (stage);	
	clutter_actor_show (voicegram);
	clutter_main ();

	g_main_loop_run(main_loops);

	return (0);
}
