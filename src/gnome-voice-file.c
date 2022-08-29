#include <libxml/parser.h>
#include <libxml/tree.h>
#include "gnome-voice-file.h"

GList *voice_stations;

static void
gnome_voice_file_parser (VoiceInfo *info,
			 xmlDocPtr doc,
			 xmlNodePtr cur)
{
	xmlNodePtr sub;
	g_return_if_fail (info != NULL);
	g_return_if_fail (doc != NULL);
	g_return_if_fail (cur != NULL);
	info->uri = (gchar *) xmlGetProp (cur, (const xmlChar *)"uri");
	sub = cur->xmlChildrenNode;
	while (sub != NULL) {
		if ((!strcasecmp (sub->name, (const xmlChar *) "uri"))) {
			info->uri = (gchar *) xmlNodeListGetString (doc, sub->xmlChildrenNode, 1);
			g_print ("station:%s\n", info->uri);
		}
		if ((!strcasecmp (sub->name, (const xmlChar *) "location"))) {
			LocationInfo *location = g_new0 (LocationInfo, 1);
			info->location = location;
			info->location->city = (gchar *) xmlNodeListGetString (doc, sub->xmlChildrenNode, 1);
			info->location->lat = (gchar *) xmlGetProp (doc, sub->xmlChildrenNode);
			info->location->lon = (gchar *) xmlGetProp (doc, sub->xmlChildrenNode);			
			g_print ("location:city:%s\n", info->location->city);
			g_print ("location:lat:%d\n", info->location->lat);
			g_print ("location:lon:%d\n", info->location->lon);
		}
		if ((!strcasecmp (sub->name, (const xmlChar *) "stream"))) {
			StreamInfo *stream = g_new0 (StreamInfo, 1);
			info->stream = stream;
			info->stream->uri = (gchar *) xmlGetProp (doc, sub->xmlChildrenNode);
		}
		sub = sub->next;
	}
	return;
}

VoiceInfo *
gnome_voice_file_loader (VoiceInfo *head,
			 char *filename)
{
	xmlDocPtr doc = NULL;
	xmlNodePtr cur = NULL;
	VoiceInfo *curr = NULL;
	VoiceInfo *list = NULL;
	g_return_val_if_fail (filename != NULL, NULL);
	doc = xmlParseFile (filename);
	if (doc == NULL) {
		perror ("xmlParseFile");
		xmlFree (doc);
		return NULL;
	}
	cur = xmlDocGetRootElement (doc);
	if (cur == NULL) {
		fprintf (stderr, "Empty document\n");
		xmlFree (doc);
		return NULL;
	}
	if (strcasecmp (cur->name, (const xmlChar *) "voice")) {
		fprintf (stderr,
			 "Document of wrong type, root node != voice\n");
		xmlFree (doc);
		return NULL;
	}
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!strcasecmp (cur->name, (const xmlChar *) "station"))) {
			curr = g_new0(VoiceInfo, 1);
			gnome_voice_file_parser (curr, doc, cur);
			curr->next = head;
			head = curr;
			list = head;
			voice_stations = g_list_append (voice_stations, (VoiceInfo *)list);
			g_free (curr);
		}
		curr = curr->next;
	}
	xmlFree (doc);
	return curr;
}
