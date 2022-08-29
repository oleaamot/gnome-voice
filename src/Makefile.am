AM_CPPFLAGS = $(GNOME_VOICE_CFLAGS) \
	-DGNOME_VOICE_DATADIR="\"$(pkgdatadir)\"" \
	-DGNOME_VOICE_LOCALEDIR="\"$(localedir)\""

AM_CFLAGS = $(GNOME_VOICE_CFLAGS) \
	-DGNOME_VOICE_DATADIR="\"$(pkgdatadir)\"" \
	-DGNOME_VOICE_LOCALEDIR="\"$(localedir)\""
	$(NULL)

AM_LDFLAGS= $(GNOME_RADIO_LIBS)

bin_PROGRAMS = gnome-voice

gnome_voice_SOURCES = gnome-voice-file.c gnome-voice-file.h gnome-voice-main.c gnome-voice-main.h gnome-voice-vosc.c gnome-voice-vosc.h

gnome_voice_CFLAGS = $(GNOME_VOICE_CFLAGS) \
	-DGNOME_VOICE_DATADIR=\"$(datadir)/gnome-voice\" \
	-DDATADIR=\"$(datadir)\" \
	-DGNOME_VOICE_DEBUG=1 \
	-DGNOME_VOICE_CFG \
	-DGNOME_VOICE_LOCALEDIR="\"$(localedir)\""

gnome_voice_LDADD = $(GNOME_VOICE_LIBS)

LDADD = $(GNOME_VOICE_LIBS)
