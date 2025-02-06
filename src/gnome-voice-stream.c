#include <string.h>
#include <gtk/gtk.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libsoup/soup.h>
#include <gst/gst.h>
#include <stdlib.h>
#include <string.h>

#define OUTPUT_OGG_FILE "GNOME.ogg"
#define OUTPUT_XML_FILE "GNOME.voice"

// Global variables for GTK widgets
GtkWidget *window;
GtkWidget *record_button;
GtkWidget *stop_button;
GtkWidget *status_label;

GstElement *pipeline;

// Stream structure
typedef struct {
    char *url;
    char *location;
    double latitude;
    double longitude;
    GtkWidget *play_button;
    GtkWidget *stop_button;
} Stream;

GList *streams = NULL;
GtkWidget *main_box;
GtkWidget *url_entry;

// Callback to start recording
static void start_recording(GtkButton *button, gpointer user_data) {
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    gtk_label_set_text(GTK_LABEL(status_label), "Recording...");
}

// Callback to stop recording
static void stop_recording(GtkButton *button, gpointer user_data) {
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gtk_label_set_text(GTK_LABEL(status_label), "Recording stopped.");

    // Save XML metadata
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "voice");
    xmlDocSetRootElement(doc, root_node);

    xmlNewProp(root_node, BAD_CAST "version", BAD_CAST "1.2.0");

    xmlNodePtr station_node = xmlNewChild(root_node, NULL, BAD_CAST "station", NULL);
    xmlNewProp(station_node, BAD_CAST "name", BAD_CAST g_get_user_name());
    xmlNewProp(station_node, BAD_CAST "uri", BAD_CAST "http://www.gnomevoice.org/");

    xmlNewChild(station_node, NULL, BAD_CAST "stream", BAD_CAST g_strconcat("http://www.gnomevoice.org/stream/", g_get_user_name(), ".voice", NULL));

    xmlSaveFormatFileEnc(OUTPUT_XML_FILE, doc, "UTF-8", 1);
    xmlFreeDoc(doc);

    gtk_label_set_text(GTK_LABEL(status_label), "Recording and metadata saved.");
}

// Initialize GStreamer pipeline
static void initialize_pipeline() {
    gst_init(NULL, NULL);

    pipeline = gst_parse_launch(
        "autoaudiosrc ! audioconvert ! vorbisenc ! oggmux ! filesink location=" OUTPUT_OGG_FILE,
        NULL);
}

// Function to save streams to an XML file
void save_streams_to_xml(const char *filename) {
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "streams");
    xmlDocSetRootElement(doc, root_node);

    for (GList *l = streams; l != NULL; l = l->next) {
        Stream *stream = (Stream *)l->data;
        xmlNodePtr station_node = xmlNewChild(root_node, NULL, BAD_CAST "station", NULL);
        xmlNewProp(station_node, BAD_CAST "uri", BAD_CAST stream->url);

        xmlNodePtr location_node = xmlNewChild(station_node, NULL, BAD_CAST "location", BAD_CAST stream->location);
        xmlNewProp(location_node, BAD_CAST "lat", BAD_CAST g_strdup_printf("%f", stream->latitude));
        xmlNewProp(location_node, BAD_CAST "lon", BAD_CAST g_strdup_printf("%f", stream->longitude));
    }

    if (xmlSaveFormatFileEnc(filename, doc, "UTF-8", 1) == -1) {
        g_print("Failed to save the XML file: %s\n", filename);
    } else {
        g_print("XML file saved successfully: %s\n", filename);
    }

    xmlFreeDoc(doc);
}

// Function to load streams from an XML file
void load_streams_from_xml(const char *filename) {
    xmlDocPtr doc;
    xmlNodePtr root_element, station_node, location_node;
    char *url, *location;
    double lat, lon;

    doc = xmlReadFile(filename, NULL, 0);
    if (doc == NULL) {
        g_print("Failed to parse the XML file: %s\n", filename);
        return;
    }

    root_element = xmlDocGetRootElement(doc);
    station_node = root_element->children;

    for (; station_node != NULL; station_node = station_node->next) {
        if (station_node->type == XML_ELEMENT_NODE && xmlStrcmp(station_node->name, BAD_CAST "station") == 0) {
            url = (char *)xmlGetProp(station_node, BAD_CAST "uri");
            location_node = station_node->children;
            location = (char *)xmlNodeGetContent(location_node);

            lat = atof((char *)xmlGetProp(location_node, BAD_CAST "lat"));
            lon = atof((char *)xmlGetProp(location_node, BAD_CAST "lon"));

            Stream *new_stream = g_new(Stream, 1);
            new_stream->url = g_strdup(url);
            new_stream->location = g_strdup(location);
            new_stream->latitude = lat;
            new_stream->longitude = lon;

            new_stream->play_button = gtk_button_new_with_label("Play");
            new_stream->stop_button = gtk_button_new_with_label("Stop");

            streams = g_list_append(streams, new_stream);

            GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
            gtk_box_pack_start(GTK_BOX(hbox), new_stream->play_button, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), new_stream->stop_button, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(main_box), hbox, FALSE, FALSE, 5);
        }
    }

    xmlFreeDoc(doc);
}

// Callback for adding a new stream
void add_stream(GtkButton *button, gpointer user_data) {
    const char *url = gtk_entry_get_text(GTK_ENTRY(url_entry));
    if (strlen(url) == 0) {
        g_print("URL is empty!\n");
        return;
    }

    Stream *new_stream = g_new(Stream, 1);
    new_stream->url = g_strdup(url);
    new_stream->location = g_strdup("Unknown");
    new_stream->latitude = 0.0;
    new_stream->longitude = 0.0;

    new_stream->play_button = gtk_button_new_with_label("Play");
    new_stream->stop_button = gtk_button_new_with_label("Stop");

    streams = g_list_append(streams, new_stream);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(hbox), new_stream->play_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), new_stream->stop_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), hbox, FALSE, FALSE, 5);

    gtk_widget_show_all(main_box);

    save_streams_to_xml(OUTPUT_XML_FILE);
}

// GUI setup
static void activate(GtkApplication *app, gpointer user_data) {
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Voicegram Recorder");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(box), 10);
    gtk_container_add(GTK_CONTAINER(window), box);

    record_button = gtk_button_new_with_label("Start Recording");
    g_signal_connect(record_button, "clicked", G_CALLBACK(start_recording), NULL);
    gtk_box_pack_start(GTK_BOX(box), record_button, TRUE, TRUE, 0);

    stop_button = gtk_button_new_with_label("Stop Recording");
    g_signal_connect(stop_button, "clicked", G_CALLBACK(stop_recording), NULL);
    gtk_box_pack_start(GTK_BOX(box), stop_button, TRUE, TRUE, 0);

    status_label = gtk_label_new("Ready to record.");
    gtk_box_pack_start(GTK_BOX(box), status_label, TRUE, TRUE, 0);

    GtkWidget *window_app = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_app), "GNOME Voice");
    gtk_window_set_default_size(GTK_WINDOW(window_app), 800, 600);
    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window_app), vbox);

    GtkWidget *url_label = gtk_label_new("Enter Stream URL:");
    gtk_box_pack_start(GTK_BOX(vbox), url_label, FALSE, FALSE, 0);

    url_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), url_entry, FALSE, FALSE, 0);

    GtkWidget *add_button = gtk_button_new_with_label("Add Stream");
    g_signal_connect(add_button, "clicked", G_CALLBACK(add_stream), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), add_button, FALSE, FALSE, 0);

    main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), main_box, TRUE, TRUE, 0);

    g_signal_connect(window_app, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window_app);

    gtk_widget_show_all(window);
}

int main(int argc, char *argv[]) {

  GtkApplication *app;
    int status;

    app = gtk_application_new("org.gnomevoice.api.voicegram", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    initialize_pipeline();

    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    if (pipeline) {
        gst_object_unref(pipeline);
    }

    gtk_init(&argc, &argv);

    const char *filename = OUTPUT_XML_FILE;
    if (argc > 1) {
        if (strcmp(argv[1], "--filename") == 0 && argc > 2) {
            filename = argv[2];
        } else {
            g_print("Usage: %s [--filename <filename>]\n", argv[0]);
            return 1;
        }
    }

    load_streams_from_xml(filename);
    
    save_streams_to_xml(filename);

    gtk_main();
    
    return status;
}
