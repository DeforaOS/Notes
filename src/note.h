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



#ifndef NOTES_NOTE_H
# define NOTES_NOTE_H


/* Note */
/* types */
typedef struct _Note Note;


/* functions */
Note * note_new(void);
Note * note_new_from_file(char const * filename);
void note_delete(Note * note);


/* accessors */
char const * note_get_description(Note * note);
char const * note_get_filename(Note * note);
char const * note_get_title(Note * note);

int note_set_description(Note * note, char const * description);
int note_set_filename(Note * note, char const * filename);
int note_set_title(Note * note, char const * title);


/* useful */
int note_load(Note * note);
int note_save(Note * note);
int note_unlink(Note * note);

#endif /* !NOTES_NOTE_H */
