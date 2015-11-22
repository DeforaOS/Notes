/* $Id$ */
/* Copyright (c) 2015 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Notes */
/* All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */



#include <stdlib.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include "noteedit.h"
#define _(string) gettext(string)


/* NoteEdit */
/* private */
/* types */
struct _NoteEdit
{
	Notes * notes;
	Note * note;

	/* widgets */
	GtkWidget * window;
	GtkWidget * title;
	GtkWidget * description;
};


/* public */
/* functions */
/* note_new */
static void _on_noteedit_cancel(gpointer data);
static void _on_noteedit_ok(gpointer data);

NoteEdit * noteedit_new(Notes * notes, Note * note)
{
	NoteEdit * noteedit;
	char buf[80];
	GtkSizeGroup * group;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	GtkWidget * bbox;
	GtkWidget * scrolled;
	char const * description;

	if((noteedit = malloc(sizeof(*noteedit))) == NULL)
		return NULL;
	noteedit->notes = notes;
	noteedit->note = note;
	noteedit->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	snprintf(buf, sizeof(buf), "%s%s", _("Edit note: "), note_get_title(
				note));
	gtk_window_set_default_size(GTK_WINDOW(noteedit->window), 300, 400);
	gtk_window_set_title(GTK_WINDOW(noteedit->window), buf);
	g_signal_connect_swapped(noteedit->window, "delete-event", G_CALLBACK(
				_on_noteedit_cancel), noteedit);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
	/* title */
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
	widget = gtk_label_new(_("Title:"));
#if GTK_CHECK_VERSION(3, 0, 0)
	g_object_set(widget, "halign", GTK_ALIGN_START, NULL);
#else
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
#endif
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	noteedit->title = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(noteedit->title), note_get_title(note));
	gtk_box_pack_start(GTK_BOX(hbox), noteedit->title, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* description */
	widget = gtk_label_new(_("Description:"));
#if GTK_CHECK_VERSION(3, 0, 0)
	g_object_set(widget, "halign", GTK_ALIGN_START, NULL);
#else
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
#endif
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled),
			GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	noteedit->description = gtk_text_view_new();
	if((description = note_get_description(note)) != NULL)
		gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(
						noteedit->description)),
				description, -1);
	gtk_container_add(GTK_CONTAINER(scrolled), noteedit->description);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
	bbox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
	gtk_box_set_spacing(GTK_BOX(bbox), 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				_on_noteedit_cancel), noteedit);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	widget = gtk_button_new_from_stock(GTK_STOCK_OK);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(_on_noteedit_ok),
			noteedit);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	gtk_box_pack_end(GTK_BOX(vbox), bbox, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(noteedit->window), 4);
	gtk_container_add(GTK_CONTAINER(noteedit->window), vbox);
	gtk_widget_show_all(noteedit->window);
	return noteedit;
}

static void _on_noteedit_cancel(gpointer data)
{
	NoteEdit * noteedit = data;

	noteedit_delete(noteedit);
}

static void _on_noteedit_ok(gpointer data)
{
	NoteEdit * noteedit = data;
	GtkTextBuffer * tbuf;
	GtkTextIter start;
	GtkTextIter end;
	gchar * description;
	
	note_set_title(noteedit->note, gtk_entry_get_text(GTK_ENTRY(
					noteedit->title)));
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(noteedit->description));
	gtk_text_buffer_get_start_iter(tbuf, &start);
	gtk_text_buffer_get_end_iter(tbuf, &end);
	description = gtk_text_buffer_get_text(tbuf, &start, &end, FALSE);
	note_set_description(noteedit->note, description);
	g_free(description);
	note_save(noteedit->note);
	notes_note_reload_all(noteedit->notes); /* XXX violent solution */
	_on_noteedit_cancel(noteedit);
}


/* noteedit_delete */
void noteedit_delete(NoteEdit * noteedit)
{
	gtk_widget_destroy(noteedit->window);
	free(noteedit);
}
