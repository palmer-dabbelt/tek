
/*
 * Copyright (C) 2011 Palmer Dabbelt
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

#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#ifdef HAVE_TALLOC
#include <talloc.h>
#else
#include "extern/talloc.h"
#endif

static bool string_ends_with(const char *string, const char *end);
static int basename_len(const char *string);
static int string_index(const char *a, const char *b);
static char *make_path_to_dotdot(void *context, const char *input);

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

    if (p != NULL) {
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

    for (i = strlen(string) - 1; i >= 0; i--) {
        if (string[i] == '/')
            return i;
    }

    return 0;
}

int string_index(const char *a, const char *b)
{
    size_t i;

    for (i = 0; i < strlen(a); i++) {
        if (strncmp(a + i, b, strlen(b)) == 0)
            return i;
    }

    return -1;
}

char *make_path_to_dotdot(void *context, const char *input)
{
    size_t i;
    size_t seperators;

    seperators = 0;
    for (i = 0; i < strlen(input); ++i)
        if (input[i] == '/')
            seperators++;

    char *out = talloc_array(context, char, seperators * 3 + 1);
    memset(out, '\0', seperators * 3 + 1);

    for (i = 0; i < seperators; ++i)
        strcat(out, "../");

    return out;
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
    bool skip_recursive_deps;
    char *actual_cache_dir;

    /* By default we don't want to skip searching for recursive
     * dependencies. */
    skip_recursive_deps = false;

    /* We need access to the real structure, get it safely */
    p = talloc_get_type(p_uncast, struct processor_stex);

    /* Makes a new context */
    c = talloc_new(p);

    /* Finds the original filename */
    cachedir_index = string_index(filename, ".tek_cache/");
    if (cachedir_index == -1) {
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

    /* Check if there's an executable file that cooresponds to this
     * .tex file, which means that we should run this executable to
     * produce the text file. */
    {
        char *exefile;
        char *depfile;
        char *dotdotpath;
        char *dirname_exefile;

        exefile = talloc_asprintf(c, "%s.proc", infile);
        depfile = talloc_asprintf(c, "%s.deps", infile);
        if (access(exefile, X_OK) == 0) {
            char *outfile;
            ssize_t i;

            outfile = talloc_asprintf(c, "%s.out", filename);
            dotdotpath = make_path_to_dotdot(c, outfile);

            dirname_exefile = talloc_asprintf(c, "%s", exefile);
            for (i = strlen(exefile); i >= 0; --i) {
                if (dirname_exefile[i] != '/')
                    continue;

                dirname_exefile[i] = '\0';
                break;
            }
            if (i == 0)
                strcpy(dirname_exefile, ".");

            makefile_create_target(m, outfile);
            makefile_start_deps(m);
            makefile_add_dep(m, exefile);

            /* Checks to see if there are any additional dependencies
             * this s cript should use. */
            if (access(depfile, R_OK) == 0) {
                char buffer[1024];
                FILE *f = fopen(depfile, "r");
                while (fgets(buffer, 1024, f) != NULL) {
                    while (isspace(buffer[strlen(buffer)-1]))
                        buffer[strlen(buffer)-1] = '\0';
                    makefile_add_dep(m, "%s/%s",
                                     dirname_exefile, buffer);
                }
                fclose(f);
            }

            makefile_end_deps(m);

            makefile_start_cmds(m);
            makefile_nam_cmd(m, "echo -e \"RUN\\t%s\"", exefile);
            makefile_add_cmd(m, "mkdir -p \"%s\" >& /dev/null || true",
                             cachedir);
            makefile_add_cmd(m, "cd %s; ./`basename %s` > `dirname %s`/%s",
                             dirname_exefile, exefile, dotdotpath, outfile);
            makefile_end_cmds(m);

            infile = outfile;
            skip_recursive_deps = true;
        }
    }

    {
        size_t last_slash;
        size_t i;

        last_slash = 0;
        for (i = 0; i < strlen(filename); ++i)
            if (filename[i] == '/')
                last_slash = i;

        actual_cache_dir = talloc_array(c, char, last_slash + 30);
        actual_cache_dir[0] = '\0';
        strncat(actual_cache_dir,
                filename + strlen(".tek_cache/"),
                last_slash - strlen(".tek_cache"));
        strcat(actual_cache_dir, ".tek_cache/");
    }

    /* Creates the target to build the image */
    makefile_create_target(m, filename);
    makefile_start_deps(m);
    if (skip_recursive_deps == false)
        makefile_add_dep(m, "%s%s-stexdeps", actual_cache_dir, filename + basename_len(filename) + 1);
    makefile_add_dep(m, infile);
    makefile_end_deps(m);

    makefile_start_cmds(m);
    makefile_nam_cmd(m, "echo -e \"STRIP\\t%s\"", infile);
    makefile_add_cmd(m, "mkdir -p \"%s\" >& /dev/null || true", cachedir);
    makefile_add_cmd(m, "texstrip -i \"%s\" -o \"%s\"", infile, filename);
    makefile_end_cmds(m);

    /* We need to scan the .tex file for dependencies, this is kind of
     * just a hack to enable that. */
    if (skip_recursive_deps == false) {
        texfile = talloc_array(c, char, strlen(infile) + 10);
        texfile[0] = '\0';
        strcat(texfile, infile);
        texfile[strlen(texfile) - 4] = '\0';
        strcat(texfile, ".tex-nopdf");
        stack_push(s, texfile);
        talloc_unlink(texfile, c);
    }

    /* Cleans up all the memory allocated by this code. */
    TALLOC_FREE(c);
}
