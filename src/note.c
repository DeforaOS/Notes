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



#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <System.h>
#include "note.h"


/* Note */
/* private */
/* types */
struct _Note
{
	Config * config;

	/* internal */
	char * filename;
	String * description;
};


/* public */
/* functions */
/* note_new */
Note * note_new(void)
{
	Note * note;

	if((note = object_new(sizeof(*note))) == NULL)
		return NULL;
	note->config = config_new();
	note->filename = NULL;
	note->description = NULL;
	if(note->config == NULL)
	{
		note_delete(note);
		return NULL;
	}
	return note;
}


/* note_new_from_file */
Note * note_new_from_file(char const * filename)
{
	Note * note;

	if((note = note_new()) == NULL)
		return NULL;
	if(note_set_filename(note, filename) != 0
			|| note_load(note) != 0)
	{
		note_delete(note);
		return NULL;
	}
	return note;
}


/* note_delete */
void note_delete(Note * note)
{
	string_delete(note->description);
	free(note->filename);
	if(note->config != NULL)
		config_delete(note->config);
	object_delete(note);
}


/* accessors */
/* note_get_description */
char const * note_get_description(Note * note)
{
	String const * p;
	String * q;

	if(note->description != NULL)
		return note->description;
	if((p = config_get(note->config, NULL, "description")) == NULL)
		return "";
	if((q = string_new_replace(p, "\\n", "\n")) == NULL
			|| string_replace(&q, "\\\\", "\\") != 0)
		return NULL;
	note->description = q;
	return note->description;
}


/* note_get_filename */
char const * note_get_filename(Note * note)
{
	return note->filename;
}


/* note_get_title */
char const * note_get_title(Note * note)
{
	char const * ret;

	if((ret = config_get(note->config, NULL, "title")) == NULL)
		return "";
	return ret;
}


/* note_set_description */
int note_set_description(Note * note, char const * description)
{
	String * d;

	if((d = string_new_replace(description, "\\", "\\\\")) == NULL)
		return -1;
	if(string_replace(&d, "\n", "\\n") != 0
			|| config_set(note->config, NULL, "description", d)
			!= 0)
	{
		string_delete(d);
		return -1;
	}
	string_delete(note->description);
	note->description = d;
	return 0;
}


/* note_set_filename */
int note_set_filename(Note * note, char const * filename)
{
	char * p;

	if((p = strdup(filename)) == NULL)
		return -1; /* XXX set error */
	free(note->filename);
	note->filename = p;
	return 0;
}


/* note_set_title */
int note_set_title(Note * note, char const * title)
{
	return config_set(note->config, NULL, "title", title);
}


/* useful */
/* note_load */
int note_load(Note * note)
{
	config_reset(note->config);
	return config_load(note->config, note->filename);
}


/* note_save */
int note_save(Note * note)
{
	if(note->filename == NULL)
		return -1; /* XXX set error */
	return config_save(note->config, note->filename);
}


/* note_unlink */
int note_unlink(Note * note)
{
	if(note->filename == NULL)
		return -1; /* XXX set error */
	return unlink(note->filename);
}
