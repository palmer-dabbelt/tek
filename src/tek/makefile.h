
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

#ifndef MAKEFILE_H
#define MAKEFILE_H

#include <stdio.h>

#include "clopts.h"
#include "stringlist.h"

enum makefile_state
{
    MAKEFILE_STATE_NONE = 0,
    MAKEFILE_STATE_TARGET,
    MAKEFILE_STATE_DEPS,
    MAKEFILE_STATE_DEPS_DONE,
    MAKEFILE_STATE_CMDS
};

struct makefile
{
    /* The current state of the makefile */
    enum makefile_state state;

    /* The file pointer used to write to this makefile.  Don't use this
     * directly, but instead use the methods below */
    FILE *file;

    /* The list of targets to be build when one types "make all" */
    struct stringlist *targets_all;

    /* The list of things that will be "rm"d on a "make clean" */
    struct stringlist *targets_clean;

    /* The list of things that will be "rm -rf"d on a "make cleancache"
     * (and also on a "make clean", and "make distclean") */
    struct stringlist *targets_cleancache;

    /* These targets will only be removed on a distclean */
    struct stringlist *targets_distclean;
};

/* Creates a new makefile, allocating it as a child of "o" */
extern struct makefile *makefile_new(struct clopts *o);

/* Creates a new target */
extern void makefile_create_target(struct makefile *m, const char *name);

/* Adds an existing target to the named command */
extern void makefile_add_all(struct makefile *m, const char *name);
extern void makefile_add_clean(struct makefile *m, const char *name);
extern void makefile_add_cleancache(struct makefile *m, const char *name);
extern void makefile_add_distclean(struct makefile *m, const char *name);

/* Adds a new dependency to a target (_start just changes state, for use with
   raw fd writes) */
extern void makefile_start_deps(struct makefile *m);
extern void makefile_add_dep(struct makefile *m, const char *format, ...);
extern void makefile_end_deps(struct makefile *m);

/* Adds a new comand for building a target (_start as above) */
extern void makefile_start_cmds(struct makefile *m);
extern void makefile_nam_cmd(struct makefile *m, const char *format, ...);
extern void makefile_add_cmd(struct makefile *m, const char *format, ...);
extern void makefile_end_cmds(struct makefile *m);

#endif
