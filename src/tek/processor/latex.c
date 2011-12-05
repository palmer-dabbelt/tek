
/*
 * Copyright (C) 2011 Daniel Dabbelt
 *   <palmem@comcast.net>
 *
 * This file is part of tek.
 * 
 * tek is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * tek is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with tek.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "latex.h"

#include <string.h>
#include <stdbool.h>
#include <talloc.h>

static bool string_ends_with(const char *string, const char *end);
static int basename_len(const char *string);
static const char *restname(const char *string);

static void process(struct processor *p_uncast, const char *filename,
		    struct stack *s, struct makefile *m);

static char *p_name;

void processor_latex_boot(void *context)
{
    p_name = talloc_strdup(context, "LATEX");
}

struct processor *processor_latex_search(void *context, const char *filename)
{
    struct processor_latex *p;

    p = NULL;

    if (string_ends_with(filename, ".tex"))
        p = talloc(context, struct processor_latex);

    if (p != NULL)
    {
        p->p.name = talloc_reference(p, p_name);
	p->p.process = &process;
    }

    return (struct processor *)p;
}

bool string_ends_with(const char *string, const char *end)
{
    return strcmp(string + strlen(string) - strlen(end), end) == 0;
}

int basename_len(const char *string)
{
    int i;
    
    for (i = strlen(string)-1; i >= 0; i--)
    {
	if (string[i] == '/')
	    return i;
    }

    return 0;
}

const char *restname(const char *string)
{
    return string + basename_len(string);
}

void process(struct processor *p_uncast, const char *filename, struct stack *s, 
	     struct makefile *m)
{
    void *c;
    int cache_dir_size;
    char *cache_dir;
    char *pp_file;
    char *pdf_file;
    char *out_file;
    struct processor_latex *p;
    
    /* We need access to the real structure, get it safely */
    p = talloc_get_type(p_uncast, struct processor_latex);

    /* Makes a new context */
    c = talloc_new(p);

    /* The cache dir */
    cache_dir_size = basename_len(filename) + strlen(CACHE_DIR) + 2;
    cache_dir = talloc_array(c, char, cache_dir_size);
    cache_dir[0] = '\0';
    if (basename_len(filename) != 0)
    {
	strncat(cache_dir, filename, basename_len(filename));
	strcat(cache_dir, "/");
    }
    strcat(cache_dir, CACHE_DIR);

    /* The preprocessed file */
    pp_file = talloc_array(c, char,
			   cache_dir_size + strlen(restname(filename)) + 2);
    pp_file[0] = '\0';
    strcat(pp_file, cache_dir);
    strcat(pp_file, "/");
    strcat(pp_file, restname(filename));

    /* The output file from latex */
    pdf_file = talloc_strdup(c, pp_file);
    pdf_file[strlen(pdf_file) - 3] = '\0';
    strcat(pdf_file, "pdf");

    /* The actual output file the user wants */
    out_file = talloc_strdup(c, filename);
    out_file[strlen(out_file) - 3] = '\0';
    strcat(out_file, "pdf");

    /* Generates the targets for this file. */
    

#ifdef DEBUG
    fprintf(stderr, "cache_dir: '%s'\n", cache_dir);
    fprintf(stderr, "pp_file:   '%s'\n", pp_file);
    fprintf(stderr, "pdf_file:  '%s'\n", pdf_file);
    fprintf(stderr, "out_file:  '%s'\n", out_file);
#endif

    /* Cleans up all the memory allocated by this code. */
    TALLOC_FREE(c);
}
