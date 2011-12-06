
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

#include "makefile.h"
#include <talloc.h>
#include <assert.h>
#include <stdarg.h>

static int mf_destructor(struct makefile *m);

struct makefile *makefile_new(struct clopts *o)
{
    struct makefile *m;

    m = talloc(o, struct makefile);
    talloc_set_destructor(m, &mf_destructor);
    m->file = fopen("Makefile", "w");
    m->targets_all = stringlist_new(m);
    m->targets_clean = stringlist_new(m);
    m->targets_cleancache = stringlist_new(m);
    m->targets_distclean = stringlist_new(m);
    m->state = MAKEFILE_STATE_NONE;

    fprintf(m->file, "SHELL=/bin/bash\n");
    fprintf(m->file, ".PHONY: all tek__all clean cleancache distclean\n");
    fprintf(m->file, ".SUFFIXES:\n");
    fprintf(m->file, "all: tek__all\n");
    fprintf(m->file, "\n");

    return m;
}

void makefile_create_target(struct makefile *m, const char *name)
{
    assert(m->state == MAKEFILE_STATE_NONE);
    m->state = MAKEFILE_STATE_TARGET;

    fprintf(m->file, "%s: Makefile", name);
}

void makefile_add_all(struct makefile *m, const char *name)
{
    stringlist_add(m->targets_all, name);
}

void makefile_add_clean(struct makefile *m, const char *name)
{
    stringlist_add(m->targets_clean, name);
}

void makefile_add_cleancache(struct makefile *m, const char *name)
{
    stringlist_add(m->targets_cleancache, name);
}

void makefile_add_distclean(struct makefile *m, const char *name)
{
    stringlist_add(m->targets_distclean, name);
}

void makefile_start_deps(struct makefile *m)
{
    assert(m->state == MAKEFILE_STATE_TARGET);
    m->state = MAKEFILE_STATE_DEPS;
}

void makefile_add_dep(struct makefile *m, const char *format, ...)
{
    va_list args;

    assert(m->state == MAKEFILE_STATE_DEPS);
    m->state = MAKEFILE_STATE_DEPS;

    fprintf(m->file, " ");

    va_start(args, NULL);
    vfprintf(m->file, format, args);
    va_end(args);
}

void makefile_end_deps(struct makefile *m)
{
    assert(m->state == MAKEFILE_STATE_DEPS);
    m->state = MAKEFILE_STATE_DEPS_DONE;

    fprintf(m->file, "\n");
}

void makefile_start_cmds(struct makefile *m)
{
    assert(m->state == MAKEFILE_STATE_DEPS_DONE);
    m->state = MAKEFILE_STATE_CMDS;
}

void makefile_nam_cmd(struct makefile *m, const char *format, ...)
{
#ifndef DEBUG
    va_list args;

    assert(m->state == MAKEFILE_STATE_CMDS);
    m->state = MAKEFILE_STATE_CMDS;

    fprintf(m->file, "\t@");

    va_start(args, NULL);
    vfprintf(m->file, format, args);
    va_end(args);

    fprintf(m->file, "\n");
#endif
}

void makefile_add_cmd(struct makefile *m, const char *format, ...)
{
    va_list args;

    assert(m->state == MAKEFILE_STATE_CMDS);
    m->state = MAKEFILE_STATE_CMDS;

#ifdef DEBUG
    fprintf(m->file, "\t");
#else
    fprintf(m->file, "\t@");
#endif

    va_start(args, NULL);
    vfprintf(m->file, format, args);
    va_end(args);

    fprintf(m->file, "\n");
}

void makefile_end_cmds(struct makefile *m)
{
    assert(m->state == MAKEFILE_STATE_CMDS);
    m->state = MAKEFILE_STATE_NONE;

    fprintf(m->file, "\n");
}

int mf_destructor(struct makefile *m)
{
    struct stringlist_node *cur;

    /* Creates the fake all target. */
    makefile_create_target(m, "tek__all");
    makefile_start_deps(m);
    cur = stringlist_start(m->targets_all);
    while (stringlist_notend(cur))
    {
        makefile_add_dep(m, stringlist_data(cur));
        cur = stringlist_next(cur);
    }
    makefile_end_deps(m);
    makefile_start_cmds(m);
    makefile_end_cmds(m);

    /* Cleans the cache */
    makefile_create_target(m, "cleancache");
    makefile_start_deps(m);
    makefile_end_deps(m);

    makefile_start_cmds(m);
    cur = stringlist_start(m->targets_cleancache);
    while (stringlist_notend(cur))
    {
        makefile_add_cmd(m, "rm -rf \"%s\" >& /dev/null || true",
                         stringlist_data(cur));
        cur = stringlist_next(cur);
    }
    makefile_end_cmds(m);

    /* Creates the fake clean target. */
    makefile_create_target(m, "clean");
    makefile_start_deps(m);
    makefile_add_dep(m, "cleancache");
    makefile_end_deps(m);

    makefile_start_cmds(m);
    cur = stringlist_start(m->targets_clean);
    while (stringlist_notend(cur))
    {
        makefile_add_cmd(m, "rm \"%s\" >& /dev/null || true",
                         stringlist_data(cur));
        cur = stringlist_next(cur);
    }
    makefile_end_cmds(m);

    /* The disclean target has its own magic. */
    makefile_create_target(m, "distclean");
    makefile_start_deps(m);
    makefile_add_dep(m, "clean");
    makefile_end_deps(m);
    makefile_start_cmds(m);
    cur = stringlist_start(m->targets_distclean);
    while (stringlist_notend(cur))
    {
        makefile_add_cmd(m, "rm \"%s\" >& /dev/null || true",
                         stringlist_data(cur));
        cur = stringlist_next(cur);
    }
    makefile_end_cmds(m);

    fclose(m->file);

    return 0;
}
