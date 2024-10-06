/* $Id$ */
/* Copyright (c) 2015-2024 Pierre Pronchery <khorben@defora.org> */
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



#ifndef DESKTOP_NOTES_H
# define DESKTOP_NOTES_H

# include <gtk/gtk.h>


/* Notes */
/* types */
typedef struct _Notes Notes;

typedef enum _NotesColumn
{
	ND_COL_NOTE = 0,
	ND_COL_TITLE,
	ND_COL_CATEGORY
} NotesColumn;
# define ND_COL_LAST ND_COL_CATEGORY
# define ND_COL_COUNT (ND_COL_LAST + 1)


/* functions */
Notes * notes_new(GtkWidget * window, GtkAccelGroup * group);
void notes_delete(Notes * notes);

/* accessors */
GtkWidget * notes_get_view(Notes * notes);
GtkTreeViewColumn * notes_get_view_column(Notes * notes, unsigned int i);
GtkWidget * notes_get_widget(Notes * notes);

/* useful */
void notes_about(Notes * notes);
int notes_error(Notes * notes, char const * message, int ret);

void notes_show_preferences(Notes * notes, gboolean show);

#endif /* !DESKTOP_NOTES_H */
