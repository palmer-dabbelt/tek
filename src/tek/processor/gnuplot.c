
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

#include "gnuplot.h"
#include "../stringlist.h"

#include <string.h>
#include <stdbool.h>
#include <talloc.h>
#include <unistd.h>

static bool string_ends_with(const char *string, const char *end);
static int basename_len(const char *string);
static const char *restname(const char *string);
static int string_index(const char *a, const char *b);

static void process(struct processor *p_uncast, const char *filename,
                    struct stack *s, struct makefile *m);

static char *p_name;

void processor_gnuplot_boot(void *context)
{
    p_name = talloc_strdup(context, "LATEX");
}

struct processor *processor_gnuplot_search(void *context,
                                           const char *filename)
{
    struct processor_gnuplot *p;

    p = NULL;

    if (string_ends_with(filename, ".gnuplot.pdf"))
        p = talloc(context, struct processor_gnuplot);

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

const char *restname(const char *string)
{
    if (basename_len(string) == 0)
        return string;

    return string + basename_len(string) + 1;
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
    void *c;
    struct processor_gnuplot *p;
    int cachedir_index;
    char *cachedir;
    char *infile;
    char *pp_file;
    FILE *inf;
    char *buf;
    int buf_size;
    struct stringlist *deps, *deps_short;
    struct stringlist_node *cur, *cur_short;

    /* We need access to the real structure, get it safely */
    p = talloc_get_type(p_uncast, struct processor_gnuplot);

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
    infile[strlen(infile) - 4] = '\0';

    TALLOC_FREE(cachedir);
    cachedir = talloc_strndup(c, filename, basename_len(filename));

    /* The pre-processed file */
    pp_file = talloc_strdup(c, filename);
    pp_file[strlen(pp_file) - 4] = '\0';

    /* First, we preprocess the .gnuplot file */
    makefile_create_target(m, pp_file);
    makefile_start_deps(m);
    makefile_add_dep(m, infile);
    makefile_end_deps(m);

    makefile_start_cmds(m);
    makefile_nam_cmd(m, "echo -e \"GPLOTPP\\t%s\"", infile);
    makefile_add_cmd(m, "mkdir -p \"%s\" >& /dev/null || true", cachedir);
    makefile_add_cmd(m, "gnuplotpp -i \"%s\" -o \"%s\"", infile, pp_file);
    makefile_end_cmds(m);

    /* Generates a PDF, checking for additional dependencies */
    makefile_create_target(m, filename);
    makefile_start_deps(m);
    makefile_add_dep(m, pp_file);
    deps = stringlist_new(c);
    deps_short = stringlist_new(c);

    buf_size = 10240;
    buf = talloc_array(c, char, buf_size);
    inf = fopen(infile, "r");
    while (fgets(buf, buf_size, inf) != NULL)
    {
        if (string_index(buf, ".dat\"") != -1)
        {
            int index;
            char *included_name;
            char *full_path;
            int full_path_size;
            char *cache_path;
            int cache_path_size;

            /* Removes all the optional agruments */
            index = string_index(buf, ".dat\"");
            while ((buf[index] != '"') && (buf[index] != '\0'))
                index--;

            /* Ensures that the code is properly formed */
            if (buf[index] == '\0')
            {
                fprintf(stderr, "Bad dat file\n");
                continue;
            }

            /* Stores the buffer and gets the filename */
            included_name = buf + index + 1;
            index++;
            while ((buf[index] != '"') && (buf[index] != '\0'))
                index++;
            buf[index] = '\0';

            /* Creates the full path */
            full_path_size =
                strlen(included_name) + basename_len(filename) + 3;
            full_path = talloc_array(c, char, full_path_size);
            full_path[0] = '\0';
            if (basename_len(infile) != 0)
            {
                strncat(full_path, infile, basename_len(infile));
                strcat(full_path, "/");
            }
            strcat(full_path, included_name);

            /* We need to convert to PDF so latex will input the file */
            cache_path_size = strlen(cachedir) + strlen(included_name) + 10;
            cache_path = talloc_array(c, char, cache_path_size);
            cache_path[0] = '\0';
            strcat(cache_path, cachedir);
            strcat(cache_path, "/");
            strcat(cache_path, included_name);
            makefile_add_dep(m, cache_path);

            /* List the extra dependencies */
            stringlist_add(deps, cache_path);
            stringlist_add(deps_short, full_path);

            /* This full path is a dependency */
            talloc_unlink(full_path, c);
        }
    }

    TALLOC_FREE(buf);

    makefile_end_deps(m);

    makefile_start_cmds(m);
    makefile_nam_cmd(m, "echo -e \"GNUPLOT\\t%s\"", infile);
    makefile_add_cmd(m, "mkdir -p \"%s\" >& /dev/null || true", cachedir);
    makefile_add_cmd(m, "cd \"%s\" ; gnuplot < \"%s\" > \"%s\".ps",
                     cachedir, restname(pp_file), restname(pp_file));
    makefile_add_cmd(m, "ps2pdf \"%s\".ps \"%s\"", pp_file, filename);
    makefile_add_cmd(m, "rm \"%s\".ps", pp_file);
    makefile_end_cmds(m);

    /* There were (potentially) some dependencies */
    cur = stringlist_start(deps);
    cur_short = stringlist_start(deps_short);
    while (stringlist_notend(cur) && stringlist_notend(cur_short))
    {
        char *buf;
        int buf_size;

        buf_size = strlen(stringlist_data(cur_short)) + strlen(".proc") + 1;
        buf = talloc_array(c, char, buf_size);
        buf[0] = '\0';
        strcat(buf, stringlist_data(cur_short));
        strcat(buf, ".proc");

        /* Checks if there is a preprocessor step */
        if (access(buf, X_OK) == 0)
        {
            makefile_create_target(m, stringlist_data(cur));
            makefile_start_deps(m);
            makefile_add_dep(m, buf);

            buf[0] = '\0';
            strcat(buf, stringlist_data(cur_short));
            strcat(buf, ".in");

            if (access(buf, R_OK))
                makefile_add_dep(m, buf);

            makefile_end_deps(m);

            makefile_start_cmds(m);

            makefile_nam_cmd(m, "echo -e \"GPLDAT\\t%s\"",
                             stringlist_data(cur_short));
            makefile_add_cmd(m, "mkdir -p \"%s\" >& /dev/null || true",
                             cachedir);

            if (access(buf, R_OK))
            {
                makefile_add_cmd(m, "\"./%s.proc\" > \"%s\"",
                                 stringlist_data(cur_short),
                                 stringlist_data(cur));
            }
            else
            {
                makefile_add_cmd(m, "\"./%s.proc\" < \"%s.in\" > \"%s\"",
                                 stringlist_data(cur_short),
                                 stringlist_data(cur_short),
                                 stringlist_data(cur));
            }

            makefile_end_cmds(m);
        }
        else
        {
            makefile_create_target(m, stringlist_data(cur));

            makefile_start_deps(m);
            makefile_add_dep(m, stringlist_data(cur_short));
            makefile_end_deps(m);

            makefile_start_cmds(m);
            makefile_nam_cmd(m, "echo -e \"GPLDAT\\t%s\"",
                             stringlist_data(cur_short));
            makefile_add_cmd(m, "mkdir -p \"%s\" >& /dev/null || true",
                             cachedir);
            makefile_add_cmd(m, "cp \"%s\" \"%s\"",
                             stringlist_data(cur_short),
                             stringlist_data(cur));
            makefile_end_cmds(m);
        }

        cur = stringlist_next(cur);
        cur_short = stringlist_next(cur_short);
    }

    /* Some temproary files were created, make sure to nuke them all */
    makefile_add_cleancache(m, cachedir);

    /* Cleans up all the memory allocated by this code. */
    TALLOC_FREE(c);
}
