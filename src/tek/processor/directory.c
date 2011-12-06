
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

#include "directory.h"

#include <string.h>
#include <stdbool.h>
#include <talloc.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>

static bool string_ends_with(const char *string, const char *end);

static void process(struct processor *p_uncast, const char *filename,
                    struct stack *s, struct makefile *m);

static char *p_name;

void processor_directory_boot(void *context)
{
    p_name = talloc_strdup(context, "DIR");
}

struct processor *processor_directory_search(void *context,
                                             const char *filename)
{
    struct processor_directory *p;
    struct stat statbuf;

    p = NULL;

    if (stat(filename, &statbuf) == 0)
        if (S_ISDIR(statbuf.st_mode))
            p = talloc(context, struct processor_directory);

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

void process(struct processor *p_uncast, const char *filename,
             struct stack *s, struct makefile *m)
{
    void *c;
    struct processor_directory *p;
    DIR *dip;
    struct dirent *dit;
    struct stringlist *l;
    char *exclude_filename;
    int exclude_filename_size;
    FILE *exclude_file;
    char *distclean_filename;
    int distclean_filename_size;

    /* We need access to the real structure, get it safely */
    p = talloc_get_type(p_uncast, struct processor_directory);

    /* Makes a new context */
    c = talloc_new(p);

    /* Make sure to exclude some files */
    l = stringlist_new(c);
    exclude_filename_size = strlen(filename) + strlen(".tekignore") + 10;
    exclude_filename = talloc_array(c, char, exclude_filename_size);
    exclude_filename[0] = '\0';
    strcat(exclude_filename, filename);
    strcat(exclude_filename, "/");
    strcat(exclude_filename, ".tekignore");

    exclude_file = NULL;
    exclude_file = fopen(exclude_filename, "r");
    if (exclude_file != NULL)
    {
        char *buf;
        int buf_size;

        buf_size = 10240;
        buf = talloc_array(c, char, buf_size);

        while (fgets(buf, buf_size, exclude_file) != NULL)
        {
            while ((strlen(buf) > 0) && (isspace(buf[strlen(buf) - 1])))
                buf[strlen(buf) - 1] = '\0';

            stringlist_add(l, talloc_strdup(c, buf));
        }

        TALLOC_FREE(buf);
        fclose(exclude_file);
    }

    TALLOC_FREE(exclude_filename);

    /* This directory is a target for distcleaning */
    distclean_filename_size = strlen(filename) + strlen("Makefile") + 10;
    distclean_filename = talloc_array(c, char, distclean_filename_size);
    distclean_filename[0] = '\0';
    strcat(distclean_filename, filename);
    strcat(distclean_filename, "/");
    strcat(distclean_filename, "Makefile");
    makefile_add_distclean(m, distclean_filename);
    talloc_unlink(c, distclean_filename);

    /* Checks this entire directory for files/directories */
    dip = opendir(filename);
    while ((dit = readdir(dip)) != NULL)
    {
        struct stat statbuf;
        char *longname;
        int longname_size;

        if (dit->d_name[0] == '.')
            continue;

        if (stringlist_include(l, dit->d_name) == true)
            continue;

        longname_size = strlen(filename) + strlen(dit->d_name) + 3;
        longname = talloc_array(c, char, longname_size);
        longname[0] = '\0';
        strcat(longname, filename);
        strcat(longname, "/");
        strcat(longname, dit->d_name);

        stat(longname, &statbuf);

        if (string_ends_with(longname, ".tex"))
            stack_push(s, longname);

        if (S_ISDIR(statbuf.st_mode))
            stack_push(s, longname);

        talloc_unlink(c, longname);
    }
    closedir(dip);

    TALLOC_FREE(c);
}
