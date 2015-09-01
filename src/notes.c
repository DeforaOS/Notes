/* $Id$ */
static char _copyright[] =
"Copyright © 2015 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop Notes */
static char const _license[] = "All rights reserved.\n"
"\n"
"Redistribution and use in source and binary forms, with or without\n"
"modification, are permitted provided that the following conditions are\n"
"met:\n"
"\n"
"1. Redistributions of source code must retain the above copyright\n"
"   notice, this list of conditions and the following disclaimer.\n"
"\n"
"2. Redistributions in binary form must reproduce the above copyright\n"
"   notice, this list of conditions and the following disclaimer in the\n"
"   documentation and/or other materials provided with the distribution.\n"
"\n"
"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS\n"
"IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED\n"
"TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A\n"
"PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT\n"
"HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\n"
"SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED\n"
"TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR\n"
"PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF\n"
"LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING\n"
"NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n"
"SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.";
/* TODO:
 * - add a clear/apply button (allocate a temporary object) */



#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <System.h>
#include <Desktop.h>
#include "noteedit.h"
#include "notes.h"
#include "../config.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* Notes */
/* private */
/* types */
typedef enum _NotesColumn
{
	ND_COL_NOTE, ND_COL_TITLE, ND_COL_CATEGORY
} NotesColumn;
#define ND_COL_LAST ND_COL_CATEGORY
#define ND_COL_COUNT (ND_COL_LAST + 1)

struct _Notes
{
	GtkWidget * window;
	GtkWidget * widget;
	GtkWidget * scrolled;
	GtkListStore * store;
	GtkListStore * priorities;
	GtkTreeModel * filter;
	GtkTreeModel * filter_sort;
	GtkWidget * view;
	GtkTreeViewColumn * columns[ND_COL_COUNT];
	GtkWidget * about;
};


/* prototypes */
static int _notes_confirm(GtkWidget * window, char const * message);
static gboolean _notes_get_iter(Notes * notes, GtkTreeIter * iter,
		GtkTreePath * path);
static char * _notes_note_get_directory(void);
static char * _notes_note_get_filename(char const * filename);
static char * _notes_note_get_new_filename(void);
static void _notes_note_save(Notes * notes, GtkTreeIter * iter);

/* callbacks */
/* toolbar */
static void _notes_on_new(gpointer data);
static void _notes_on_edit(gpointer data);
static void _notes_on_select_all(gpointer data);
static void _notes_on_delete(gpointer data);
#ifdef EMBEDDED
static void _notes_on_preferences(gpointer data);
#endif

/* view */
static void _notes_on_note_activated(gpointer data);
static void _notes_on_note_cursor_changed(gpointer data);
static void _notes_on_note_title_edited(GtkCellRendererText * renderer,
		gchar * path, gchar * title, gpointer data);


/* constants */
static const struct
{
	int col;
	char const * title;
	int sort;
	GCallback callback;
} _notes_columns[] =
{
	{ ND_COL_TITLE, N_("Title"), ND_COL_TITLE, G_CALLBACK(
			_notes_on_note_title_edited) },
	{ 0, NULL, 0, NULL }
};


static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};

/* toolbar */
static DesktopToolbar _toolbar[] =
{
	{ N_("New note"), G_CALLBACK(_notes_on_new), GTK_STOCK_NEW, 0, 0, NULL },
	{ N_("Edit note"), G_CALLBACK(_notes_on_edit), GTK_STOCK_EDIT, 0, 0,
		NULL },
	{ "", NULL, NULL, 0, 0, NULL },
#if GTK_CHECK_VERSION(2, 10, 0)
	{ N_("Select all"), G_CALLBACK(_notes_on_select_all),
		GTK_STOCK_SELECT_ALL, 0, 0, NULL },
#else
	{ N_("Select all"), G_CALLBACK(_notes_on_select_all), "edit-select-all",
		0, 0, NULL },
#endif
	{ N_("Delete note"), G_CALLBACK(_notes_on_delete), GTK_STOCK_DELETE, 0,
		0, NULL },
#ifdef EMBEDDED
	{ "", NULL, NULL, 0, 0, NULL },
	{ N_("Preferences"), G_CALLBACK(_notes_on_preferences),
		GTK_STOCK_PREFERENCES, 0, 0, NULL },
#endif
	{ "", NULL, NULL, 0, 0, NULL },
	{ NULL, NULL, NULL, 0, 0, NULL }
};


/* public */
/* functions */
/* notes_new */
static void _new_view(Notes * notes);
static gboolean _new_idle(gpointer data);

Notes * notes_new(GtkWidget * window, GtkAccelGroup * group)
{
	Notes * notes;
	GtkWidget * vbox;
	GtkWidget * widget;

	if((notes = object_new(sizeof(*notes))) == NULL)
		return NULL;
	/* main window */
	notes->window = window;
	vbox = gtk_vbox_new(FALSE, 0);
	notes->widget = vbox;
	/* toolbar */
	widget = desktop_toolbar_create(_toolbar, notes, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	/* view */
	notes->scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(notes->scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	_new_view(notes);
	gtk_box_pack_start(GTK_BOX(vbox), notes->scrolled, TRUE, TRUE, 0);
	notes->about = NULL;
	g_idle_add(_new_idle, notes);
	return notes;
}

static void _new_view(Notes * notes)
{
	size_t i;
	GtkTreeSelection * sel;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;

	notes->store = gtk_list_store_new(ND_COL_COUNT,
			G_TYPE_POINTER, /* note */
			G_TYPE_STRING,	/* title */
			G_TYPE_STRING);	/* category */
	/* XXX get rid of filter? */
	notes->filter = gtk_tree_model_filter_new(GTK_TREE_MODEL(notes->store),
			NULL);
	notes->filter_sort = gtk_tree_model_sort_new_with_model(notes->filter);
	notes->view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				notes->filter_sort));
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(notes->view), TRUE);
	if((sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(notes->view)))
			!= NULL)
		gtk_tree_selection_set_mode(sel, GTK_SELECTION_MULTIPLE);
	g_signal_connect_swapped(notes->view, "cursor-changed", G_CALLBACK(
				_notes_on_note_cursor_changed), notes);
	g_signal_connect_swapped(notes->view, "row-activated", G_CALLBACK(
				_notes_on_note_activated), notes);
	/* columns */
	memset(&notes->columns, 0, sizeof(notes->columns));
	for(i = 0; _notes_columns[i].title != NULL; i++)
	{
		renderer = gtk_cell_renderer_text_new();
		if(_notes_columns[i].callback != NULL)
		{
			g_object_set(G_OBJECT(renderer), "editable", TRUE,
					"ellipsize", PANGO_ELLIPSIZE_END, NULL);
			g_signal_connect(renderer, "edited", G_CALLBACK(
						_notes_columns[i].callback),
					notes);
		}
		column = gtk_tree_view_column_new_with_attributes(
				_(_notes_columns[i].title), renderer, "text",
				_notes_columns[i].col, NULL);
		notes->columns[_notes_columns[i].col] = column;
#if GTK_CHECK_VERSION(2, 4, 0)
		gtk_tree_view_column_set_expand(column, TRUE);
#endif
		gtk_tree_view_column_set_resizable(column, TRUE);
		gtk_tree_view_column_set_sort_column_id(column,
				_notes_columns[i].sort);
		gtk_tree_view_append_column(GTK_TREE_VIEW(notes->view), column);
	}
	gtk_tree_view_column_set_sort_column_id(column, ND_COL_TITLE);
	gtk_container_add(GTK_CONTAINER(notes->scrolled), notes->view);
}

static gboolean _new_idle(gpointer data)
{
	Notes * notes = data;

	notes_note_reload_all(notes);
	return FALSE;
}


/* notes_delete */
void notes_delete(Notes * notes)
{
	notes_note_save_all(notes);
	notes_note_remove_all(notes);
	free(notes);
	object_delete(notes);
}


/* accessors */
/* notes_get_widget */
GtkWidget * notes_get_widget(Notes * notes)
{
	return notes->widget;
}


/* useful */
/* notes_about */
static gboolean _about_on_closex(gpointer data);

void notes_about(Notes * notes)
{
	if(notes->about != NULL)
	{
		gtk_widget_show(notes->about);
		return;
	}
	notes->about = desktop_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(notes->about),
			GTK_WINDOW(notes->window));
	desktop_about_dialog_set_authors(notes->about, _authors);
	desktop_about_dialog_set_comments(notes->about,
			_("Notes for the DeforaOS desktop"));
	desktop_about_dialog_set_copyright(notes->about, _copyright);
	desktop_about_dialog_set_logo_icon_name(notes->about, "notes");
	desktop_about_dialog_set_license(notes->about, _license);
	desktop_about_dialog_set_program_name(notes->about, PACKAGE);
	desktop_about_dialog_set_translator_credits(notes->about,
			_("translator-credits"));
	desktop_about_dialog_set_version(notes->about, VERSION);
	desktop_about_dialog_set_website(notes->about, "http://www.defora.org/");
	g_signal_connect_swapped(notes->about, "delete-event", G_CALLBACK(
				_about_on_closex), notes);
	gtk_widget_show(notes->about);
}

static gboolean _about_on_closex(gpointer data)
{
	Notes * notes = data;

	gtk_widget_hide(notes->about);
	return TRUE;
}


/* notes_error */
static int _error_text(char const * message, int ret);

int notes_error(Notes * notes, char const * message, int ret)
{
	GtkWidget * dialog;

	if(notes == NULL)
		return _error_text(message, ret);
	dialog = gtk_message_dialog_new(GTK_WINDOW(notes->window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s",
#if GTK_CHECK_VERSION(2, 8, 0)
			_("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s",
#endif
			message);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret;
}

static int _error_text(char const * message, int ret)
{
	fputs(PACKAGE ": ", stderr);
	fputs(message, stderr);
	fputc('\n', stderr);
	return ret;
}


/* notes_show_preferences */
void notes_show_preferences(Notes * notes, gboolean show)
{
	/* FIXME implement */
}


/* notes */
/* notes_note_add */
Note * notes_note_add(Notes * notes, Note * note)
{
	GtkTreeIter iter;
	char * filename;

	if(note == NULL)
	{
		if((note = note_new()) == NULL)
			return NULL;
		if((filename = _notes_note_get_new_filename()) == NULL)
		{
			notes_error(notes, error_get(), 0);
			note_delete(note);
			return NULL;
		}
		note_set_filename(note, filename);
		free(filename);
		note_set_title(note, _("New note"));
		note_save(note);
	}
	gtk_list_store_insert(notes->store, &iter, 0);
	gtk_list_store_set(notes->store, &iter, ND_COL_NOTE, note,
			ND_COL_TITLE, note_get_title(note), -1);
	return note;
}


/* notes_note_delete_selected */
static void _note_delete_selected_foreach(GtkTreeRowReference * reference,
		Notes * notes);

void notes_note_delete_selected(Notes * notes)
{
	GtkTreeSelection * treesel;
	GList * selected;
	GtkTreeModel * model = GTK_TREE_MODEL(notes->store);
	GtkTreeRowReference * reference;
	GList * s;
	GtkTreePath * path;

	if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(notes->view)))
			== NULL)
		return;
	if((selected = gtk_tree_selection_get_selected_rows(treesel, NULL))
			== NULL)
		return;
	if(_notes_confirm(notes->window, _("Are you sure you want to delete the"
					" selected note(s)?")) != 0)
		return;
	for(s = g_list_first(selected); s != NULL; s = g_list_next(s))
	{
		if((path = s->data) == NULL)
			continue;
		reference = gtk_tree_row_reference_new(model, path);
		s->data = reference;
		gtk_tree_path_free(path);
	}
	g_list_foreach(selected, (GFunc)_note_delete_selected_foreach, notes);
	g_list_free(selected);
}

static void _note_delete_selected_foreach(GtkTreeRowReference * reference,
		Notes * notes)
{
	GtkTreeModel * model = GTK_TREE_MODEL(notes->store);
	GtkTreePath * path;
	GtkTreeIter iter;
	Note * note;

	if(reference == NULL)
		return;
	if((path = gtk_tree_row_reference_get_path(reference)) == NULL)
		return;
	if(_notes_get_iter(notes, &iter, path) == TRUE)
	{
		gtk_tree_model_get(model, &iter, ND_COL_NOTE, &note, -1);
		note_unlink(note);
		note_delete(note);
	}
	gtk_list_store_remove(notes->store, &iter);
	gtk_tree_path_free(path);
}


/* notes_note_cursor_changed */
void notes_note_cursor_changed(Notes * notes)
{
	GtkTreeModel * model = GTK_TREE_MODEL(notes->store);
	GtkTreePath * path = NULL;
	GtkTreeViewColumn * column = NULL;
	GtkTreeIter iter;
	Note * note = NULL;
	gint id = -1;

	gtk_tree_view_get_cursor(GTK_TREE_VIEW(notes->view), &path, &column);
	if(path == NULL)
		return;
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, ND_COL_NOTE, &note, -1);
	if(column != NULL)
		id = gtk_tree_view_column_get_sort_column_id(column);
	gtk_tree_path_free(path);
}


/* notes_note_edit */
void notes_note_edit(Notes * notes)
{
	GtkTreeSelection * treesel;
	GList * selected;
	GtkTreeModel * model = GTK_TREE_MODEL(notes->store);
	GList * s;
	GtkTreePath * path;
	GtkTreeIter iter;
	Note * note;

	if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(notes->view)))
			== NULL)
		return;
	if((selected = gtk_tree_selection_get_selected_rows(treesel, NULL))
			== NULL)
		return;
	for(s = g_list_first(selected); s != NULL; s = g_list_next(s))
	{
		if((path = s->data) == NULL)
			continue;
		if(_notes_get_iter(notes, &iter, path) != TRUE)
			continue;
		gtk_tree_model_get(model, &iter, ND_COL_NOTE, &note, -1);
		if(note != NULL)
			noteedit_new(notes, note);
	}
	g_list_free(selected);
}


/* notes_note_reload_all */
int notes_note_reload_all(Notes * notes)
{
	int ret = 0;
	char * filename;
	DIR * dir;
	struct dirent * de;
	Note * note;

	if((filename = _notes_note_get_directory()) == NULL)
		return notes_error(notes, error_get(), 1);
	if((dir = opendir(filename)) == NULL)
	{
		if(errno != ENOENT)
		{
			error_set("%s: %s", filename, strerror(errno));
			ret = notes_error(notes, error_get(), 1);
		}
	}
	else
	{
		notes_note_remove_all(notes);
		while((de = readdir(dir)) != NULL)
		{
			if(strncmp(de->d_name, "note.", 5) != 0)
				continue;
			free(filename);
			if((filename = _notes_note_get_filename(de->d_name))
					== NULL)
				continue; /* XXX report error */
			if((note = note_new_from_file(filename)) == NULL)
			{
				notes_error(NULL, error_get(), 1);
				continue;
			}
			if(notes_note_add(notes, note) == NULL)
			{
				note_delete(note);
				continue; /* XXX report error */
			}
		}
	}
	free(filename);
	return ret;
}


/* notes_note_remove_all */
void notes_note_remove_all(Notes * notes)
{
	GtkTreeModel * model = GTK_TREE_MODEL(notes->store);
	GtkTreeIter iter;
	gboolean valid;
	Note * note;

	valid = gtk_tree_model_get_iter_first(model, &iter);
	for(; valid == TRUE; valid = gtk_tree_model_iter_next(model, &iter))
	{
		gtk_tree_model_get(model, &iter, ND_COL_NOTE, &note, -1);
		note_delete(note);
	}
	gtk_list_store_clear(notes->store);
}


/* notes_note_save_all */
void notes_note_save_all(Notes * notes)
{
	GtkTreeModel * model = GTK_TREE_MODEL(notes->store);
	GtkTreeIter iter;
	gboolean valid;

	valid = gtk_tree_model_get_iter_first(model, &iter);
	for(; valid == TRUE; valid = gtk_tree_model_iter_next(model, &iter))
		_notes_note_save(notes, &iter);
}


/* notes_note_select_all */
void notes_note_select_all(Notes * notes)
{
	GtkTreeSelection * sel;

	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(notes->view));
	gtk_tree_selection_select_all(sel);
}


/* notes_note_set_title */
void notes_note_set_title(Notes * notes, GtkTreePath * path, char const * title)
{
	GtkTreeModel * model = GTK_TREE_MODEL(notes->store);
	GtkTreeIter iter;
	Note * note;

	_notes_get_iter(notes, &iter, path);
	gtk_tree_model_get(model, &iter, ND_COL_NOTE, &note, -1);
	note_set_title(note, title);
	gtk_list_store_set(notes->store, &iter, ND_COL_TITLE, title, -1);
	note_save(note);
}


/* private */
/* functions */
/* notes_confirm */
static int _notes_confirm(GtkWidget * window, char const * message)
{
	GtkWidget * dialog;
	int res;

	dialog = gtk_message_dialog_new(GTK_WINDOW(window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "%s",
#if GTK_CHECK_VERSION(2, 8, 0)
			_("Question"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s",
#endif
			message);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Question"));
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	if(res == GTK_RESPONSE_YES)
		return 0;
	return 1;
}


/* notes_get_iter */
static gboolean _notes_get_iter(Notes * notes, GtkTreeIter * iter,
		GtkTreePath * path)
{
	GtkTreeIter p;

	if(gtk_tree_model_get_iter(GTK_TREE_MODEL(notes->filter_sort), iter,
				path) == FALSE)
		return FALSE;
	gtk_tree_model_sort_convert_iter_to_child_iter(GTK_TREE_MODEL_SORT(
				notes->filter_sort), &p, iter);
	gtk_tree_model_filter_convert_iter_to_child_iter(GTK_TREE_MODEL_FILTER(
				notes->filter), iter, &p);
	return TRUE;
}


/* notes_note_get_directory */
static char * _notes_note_get_directory(void)
{
	char const * homedir;
	size_t len;
	char const directory[] = ".notes";
	char * filename;

	if((homedir = getenv("HOME")) == NULL)
		homedir = g_get_home_dir();
	len = strlen(homedir) + 1 + sizeof(directory);
	if((filename = malloc(len)) == NULL)
		return NULL;
	snprintf(filename, len, "%s/%s", homedir, directory);
	return filename;
}


/* notes_note_get_filename */
static char * _notes_note_get_filename(char const * filenam)
{
	char const * homedir;
	int len;
	char const directory[] = ".notes";
	char * pathname;

	if((homedir = getenv("HOME")) == NULL)
		homedir = g_get_home_dir();
	len = strlen(homedir) + 1 + sizeof(directory) + 1 + strlen(filenam) + 1;
	if((pathname = malloc(len)) == NULL)
		return NULL;
	snprintf(pathname, len, "%s/%s/%s", homedir, directory, filenam);
	return pathname;
}


/* notes_note_get_new_filename */
static char * _notes_note_get_new_filename(void)
{
	char const * homedir;
	int len;
	char const directory[] = ".notes";
	char template[] = "note.XXXXXX";
	char * filename;
	int fd;

	if((homedir = getenv("HOME")) == NULL)
		homedir = g_get_home_dir();
	len = strlen(homedir) + 1 + sizeof(directory) + 1 + sizeof(template);
	if((filename = malloc(len)) == NULL)
		return NULL;
	snprintf(filename, len, "%s/%s", homedir, directory);
	if((mkdir(filename, 0777) != 0 && errno != EEXIST)
			|| snprintf(filename, len, "%s/%s/%s", homedir,
				directory, template) >= len
			|| (fd = mkstemp(filename)) < 0)
	{
		error_set("%s: %s", filename, strerror(errno));
		free(filename);
		return NULL;
	}
	close(fd);
	return filename;
}


/* notes_note_save */
static void _notes_note_save(Notes * notes, GtkTreeIter * iter)
{
	GtkTreeModel * model = GTK_TREE_MODEL(notes->store);
	Note * note;

	gtk_tree_model_get(model, iter, ND_COL_NOTE, &note, -1);
	note_save(note);
}


/* callbacks */
/* toolbar */
/* notes_on_delete */
static void _notes_on_delete(gpointer data)
{
	Notes * notes = data;

	notes_note_delete_selected(notes);
}


/* notes_on_edit */
static void _notes_on_edit(gpointer data)
{
	Notes * notes = data;

	notes_note_edit(notes);
}


/* notes_on_new */
static void _notes_on_new(gpointer data)
{
	Notes * notes = data;

	notes_note_add(notes, NULL);
}


#ifdef EMBEDDED
/* notes_on_preferences */
static void _notes_on_preferences(gpointer data)
{
	Notes * notes = data;

	notes_show_preferences(notes, TRUE);
}
#endif


/* notes_on_select_all */
static void _notes_on_select_all(gpointer data)
{
	Notes * notes = data;

	notes_note_select_all(notes);
}


/* view */
/* notes_on_note_activated */
static void _notes_on_note_activated(gpointer data)
{
	Notes * notes = data;

	notes_note_edit(notes);
}


/* notes_on_note_cursor_changed */
static void _notes_on_note_cursor_changed(gpointer data)
{
	Notes * notes = data;

	notes_note_cursor_changed(notes);
}


/* notes_on_note_title_edited */
static void _notes_on_note_title_edited(GtkCellRendererText * renderer,
		gchar * path, gchar * title, gpointer data)
{
	Notes * notes = data;
	GtkTreePath * treepath;

	treepath = gtk_tree_path_new_from_string(path);
	notes_note_set_title(notes, treepath, title);
	gtk_tree_path_free(treepath);
}
