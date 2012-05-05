
/*
 * Copyright (C) 2011 Daniel Dabbelt
 *   <palmer@dabbelt.com>
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

#include "stex.h"

#include <string.h>
#include <stdbool.h>
#include <talloc.h>

static bool string_ends_with(const char *string, const char *end);
static int basename_len(const char *string);
static int string_index(const char *a, const char *b);

static void process(struct processor *p_uncast, const char *filename,
                    struct stack *s, struct makefile *m);

static char *p_name;

void processor_stex_boot(void *context)
{
    p_name = talloc_strdup(context, "STRIP");
}

struct processor *processor_stex_search(void *context, const char *filename)
{
    struct processor_stex *p;

    p = NULL;

    if (string_ends_with(filename, ".stex"))
        p = talloc(context, struct processor_stex);

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

    for (i = strlen(string) - 1; i >= 0; i--)
    {
        if (string[i] == '/')
            return i;
    }

    return 0;
}

int string_index(const char *a, const char *b)
{
    int i;

    for (i = 0; i < strlen(a); i++)
    {
        if (strncmp(a + i, b, strlen(b)) == 0)
            return i;
    }

    return -1;
}

void process(struct processor *p_uncast, const char *filename,
             struct stack *s, struct makefile *m)
{
    struct processor_stex *p;
    void *c;
    int cachedir_index;
    char *cachedir;
    char *infile;
    char *texfile;

    /* We need access to the real structure, get it safely */
    p = talloc_get_type(p_uncast, struct processor_stex);

    /* Makes a new context */
    c = talloc_new(p);

    /* Finds the original filename */
    cachedir_index = string_index(filename, ".tek_cache/");
    if (cachedir_index == -1)
    {
        fprintf(stderr, "Bad cachedir for image\n");
        return;
    }

    cachedir = talloc_strdup(c, filename);
    cachedir[cachedir_index + strlen(".tex_cache/")] = '\0';

    infile = talloc_strdup(c, filename);
    infile[cachedir_index] = '\0';
    strcat(infile, filename + strlen(cachedir));
    infile[strlen(infile) - 5] = '\0';
    strcat(infile, ".tex");

    TALLOC_FREE(cachedir);
    cachedir = talloc_strndup(c, filename, basename_len(filename));

    /* Creates the target to build the image */
    makefile_create_target(m, filename);
    makefile_start_deps(m);
    makefile_add_dep(m, "%s-stexdeps", filename);
    makefile_add_dep(m, infile);
    makefile_end_deps(m);

    makefile_start_cmds(m);
    makefile_nam_cmd(m, "echo -e \"STRIP\\t%s\"", infile);
    makefile_add_cmd(m, "mkdir -p \"%s\" >& /dev/null || true", cachedir);
    makefile_add_cmd(m, "texstrip -i \"%s\" -o \"%s\"", infile, filename);
    makefile_end_cmds(m);

    /* We need to scan the .tex file for dependencies, this is kind of
     * just a hack to enable that. */
    texfile = talloc_array(c, char, strlen(infile) + 10);
    texfile[0] = '\0';
    strcat(texfile, infile);
    texfile[strlen(texfile) - 4] = '\0';
    strcat(texfile, ".tex-nopdf");
    stack_push(s, texfile);
    talloc_unlink(texfile, c);

    /* Cleans up all the memory allocated by this code. */
    TALLOC_FREE(c);
}
