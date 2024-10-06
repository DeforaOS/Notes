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



#ifndef NOTES_NOTES_H
# define NOTES_NOTES_H

# include "../include/Notes.h"
# include "note.h"


/* Notes */
/* functions */
/* note */
Note * notes_note_add(Notes * notes, Note * note);
void notes_note_delete_selected(Notes * notes);
void notes_note_remove_all(Notes * notes);

/* accessors */
void notes_note_set_title(Notes * notes, GtkTreePath * path,
		char const * title);

void notes_note_cursor_changed(Notes * notes);
void notes_note_edit(Notes * notes);
int notes_note_reload_all(Notes * notes);
void notes_note_save_all(Notes * notes);
void notes_note_select_all(Notes * notes);

#endif /* !NOTES_NOTES_H */
