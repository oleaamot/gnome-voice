#ifndef GNOME_VOICE_MAIN_H
#define GNOME_VOICE_MAIN_H 1

#include <geoclue.h>

double lat = 21.293352;
double lon = -157.839583;

double lat_gps = 60.293352;
double lon_gps = 10.839583;

typedef struct
{
	ChamplainView *view;
	ChamplainMarker *voice_marker;
} GpsCallbackData;

typedef struct
{
	ChamplainView *view;
	ChamplainMarker *voicegram;
} GetVoicegramData;

typedef struct
{
        VoiceOscilloscope *oscilloscope_visual;
} OscilloscopeCallbackData;

typedef struct
{
        gboolean gnome_voice_real_name_privacy;
} GVoiceCfg;

gboolean gps_callback (GClueSimple *simple, GpsCallbackData *data);

static void gv_wizard_entry_changed(GtkEditable *, GtkAssistant *, GstElement *);
static void gv_wizard_button_toggled(GtkCheckButton *, GtkAssistant *);
static void gv_wizard_button_clicked(GtkButton *, GtkAssistant *);
static void gv_wizard_cancel (GtkAssistant *, gpointer);
static void gv_wizard_close (GtkAssistant *, gpointer);
static void gv_wizard_apply (GtkAssistant *, gpointer);

#endif /* GNOME_VOICE_MAIN_H */
