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
#include <gdk/gdkkeysyms.h>
#include <Desktop.h>
#include "notes.h"
#include "window.h"
#include "../config.h"
#define _(string) gettext(string)
#define N_(string) (string)

#ifndef PROGNAME
# define PROGNAME	"notes"
#endif


/* NotesWindow */
/* private */
/* types */
struct _NotesWindow
{
	Notes * notes;

	/* widgets */
	GtkWidget * window;
	GtkWidget * statusbar;
};


/* prototypes */
/* callbacks */
static void _noteswindow_on_close(gpointer data);
static gboolean _noteswindow_on_closex(gpointer data);
static void _noteswindow_on_edit(gpointer data);
static void _noteswindow_on_new(gpointer data);
static void _noteswindow_on_preferences(gpointer data);

#ifndef EMBEDDED
/* menus */
/* file menu */
static void _noteswindow_on_file_new(gpointer data);
static void _noteswindow_on_file_edit(gpointer data);
static void _noteswindow_on_file_close(gpointer data);

/* edit menu */
static void _noteswindow_on_edit_select_all(gpointer data);
static void _noteswindow_on_edit_delete(gpointer data);
static void _noteswindow_on_edit_preferences(gpointer data);

/* help menu */
static void _noteswindow_on_help_about(gpointer data);
static void _noteswindow_on_help_contents(gpointer data);
#endif

/* constants */
/* accelerators */
static const DesktopAccel _notes_accel[] =
{
#ifdef EMBEDDED
	{ G_CALLBACK(_noteswindow_on_close), GDK_CONTROL_MASK, GDK_KEY_W },
	{ G_CALLBACK(_noteswindow_on_edit), GDK_CONTROL_MASK, GDK_KEY_E },
	{ G_CALLBACK(_noteswindow_on_new), GDK_CONTROL_MASK, GDK_KEY_N },
	{ G_CALLBACK(_noteswindow_on_preferences), GDK_CONTROL_MASK, GDK_KEY_P },
#endif
	{ NULL, 0, 0 }
};

#ifndef EMBEDDED
/* menubar */
static const DesktopMenu _file_menu[] =
{
	{ N_("_New"), G_CALLBACK(_noteswindow_on_file_new), GTK_STOCK_NEW,
		GDK_CONTROL_MASK, GDK_KEY_N },
	{ N_("_Edit"), G_CALLBACK(_noteswindow_on_file_edit), GTK_STOCK_EDIT,
		GDK_CONTROL_MASK, GDK_KEY_E },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Close"), G_CALLBACK(_noteswindow_on_file_close), GTK_STOCK_CLOSE,
		GDK_CONTROL_MASK, GDK_KEY_W },
	{ NULL, NULL, NULL, 0, 0 }
};
static const DesktopMenu _edit_menu[] =
{
	{ N_("Select _All"), G_CALLBACK(_noteswindow_on_edit_select_all),
#if GTK_CHECK_VERSION(2, 10, 0)
		GTK_STOCK_SELECT_ALL,
#else
		"edit-select-all",
#endif
		GDK_CONTROL_MASK, GDK_KEY_A },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Delete"), G_CALLBACK(_noteswindow_on_edit_delete),
		GTK_STOCK_DELETE, 0, 0 },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Preferences"), G_CALLBACK(_noteswindow_on_edit_preferences),
		GTK_STOCK_PREFERENCES, GDK_CONTROL_MASK, GDK_KEY_P },
	{ NULL, NULL, NULL, 0, 0 }
};
static const DesktopMenu _help_menu[] =
{
	{ N_("_Contents"), G_CALLBACK(_noteswindow_on_help_contents),
		"help-contents", 0, GDK_KEY_F1 },
	{ N_("_About"), G_CALLBACK(_noteswindow_on_help_about),
#if GTK_CHECK_VERSION(2, 6, 0)
		GTK_STOCK_ABOUT, 0, 0 },
#else
		NULL, 0, 0 },
#endif
	{ NULL, NULL, NULL, 0, 0 }
};
static const DesktopMenubar _menubar[] =
{
	{ N_("_File"), _file_menu },
	{ N_("_Edit"), _edit_menu },
	{ N_("_Help"), _help_menu },
	{ NULL, NULL },
};
#endif


/* public */
/* functions */
/* noteswindow_new */
NotesWindow * noteswindow_new(void)
{
	NotesWindow * notes;
	GtkAccelGroup * group;
	GtkWidget * vbox;
	GtkWidget * widget;

	if((notes = malloc(sizeof(*notes))) == NULL)
		return NULL;
	notes->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	group = gtk_accel_group_new();
	notes->notes = notes_new(notes->window, group);
	/* check for errors */
	if(notes->notes == NULL)
	{
		noteswindow_delete(notes);
		g_object_unref(group);
		return NULL;
	}
	desktop_accel_create(_notes_accel, notes, group);
	gtk_window_add_accel_group(GTK_WINDOW(notes->window), group);
	g_object_unref(group);
	gtk_window_set_default_size(GTK_WINDOW(notes->window), 640, 480);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(notes->window), "notes");
#endif
	gtk_window_set_title(GTK_WINDOW(notes->window), _("Notes"));
	g_signal_connect_swapped(notes->window, "delete-event", G_CALLBACK(
				_noteswindow_on_closex), notes);
	vbox = gtk_vbox_new(FALSE, 0);
#ifndef EMBEDDED
	/* menubar */
	widget = desktop_menubar_create(_menubar, notes, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
#endif
	widget = notes_get_widget(notes->notes);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	/* statusbar */
	notes->statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox), notes->statusbar, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(notes->window), vbox);
	gtk_widget_show_all(notes->window);
	return notes;
}


/* noteswindow_delete */
void noteswindow_delete(NotesWindow * notes)
{
	if(notes->notes != NULL)
		notes_delete(notes->notes);
	gtk_widget_destroy(notes->window);
	free(notes);
}


/* private */
/* functions */
/* callbacks */
/* noteswindow_on_close */
static void _noteswindow_on_close(gpointer data)
{
	NotesWindow * notes = data;

	_noteswindow_on_closex(notes);
}


/* noteswindow_on_closex */
static gboolean _noteswindow_on_closex(gpointer data)
{
	NotesWindow * notes = data;

	gtk_widget_hide(notes->window);
	gtk_main_quit();
	return TRUE;
}


/* noteswindow_on_edit */
static void _noteswindow_on_edit(gpointer data)
{
	NotesWindow * notes = data;

	notes_note_edit(notes->notes);
}


/* noteswindow_on_new */
static void _noteswindow_on_new(gpointer data)
{
	NotesWindow * notes = data;

	notes_note_add(notes->notes, NULL);
}


/* noteswindow_on_preferences */
static void _noteswindow_on_preferences(gpointer data)
{
	NotesWindow * notes = data;

	notes_show_preferences(notes->notes, TRUE);
}


#ifndef EMBEDDED
/* file menu */
/* noteswindow_on_file_close */
static void _noteswindow_on_file_close(gpointer data)
{
	NotesWindow * notes = data;

	_noteswindow_on_close(notes);
}


/* noteswindow_on_file_edit */
static void _noteswindow_on_file_edit(gpointer data)
{
	NotesWindow * notes = data;

	_noteswindow_on_edit(notes);
}


/* noteswindow_on_file_new */
static void _noteswindow_on_file_new(gpointer data)
{
	NotesWindow * notes = data;

	_noteswindow_on_new(notes);
}


/* edit menu */
/* noteswindow_on_edit_delete */
static void _noteswindow_on_edit_delete(gpointer data)
{
	NotesWindow * notes = data;

	notes_note_delete_selected(notes->notes);
}


/* noteswindow_on_edit_preferences */
static void _noteswindow_on_edit_preferences(gpointer data)
{
	NotesWindow * notes = data;

	_noteswindow_on_preferences(notes);
}


/* noteswindow_on_edit_select_all */
static void _noteswindow_on_edit_select_all(gpointer data)
{
	NotesWindow * notes = data;

	notes_note_select_all(notes->notes);
}


/* help menu */
/* noteswindow_on_help_about */
static void _noteswindow_on_help_about(gpointer data)
{
	NotesWindow * notes = data;

	notes_about(notes->notes);
}


/* noteswindow_on_help_contents */
static void _noteswindow_on_help_contents(gpointer data)
{
	desktop_help_contents(PACKAGE, PROGNAME);
}
#endif
