
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

#ifndef STRING_LIST_H
#define STRING_LIST_H

#include "error.h"
#include <stdio.h>
#include <stdbool.h>

struct stringlist_node
{
    struct stringlist_node *next;
    const char *data;
};

struct stringlist
{
    struct stringlist_node *head;
};

/* Allocates a new stringlist, passed the parent context */
extern struct stringlist *stringlist_new(void *context);

/* Adds an entry to the given stringlist */
extern void stringlist_add(struct stringlist *l, const char *to_add);

/* Removes a string from the given list */
extern void stringlist_del(struct stringlist *l, const char *to_del);

/* Checks if the given string is in the given string list */
extern bool stringlist_include(struct stringlist *l, const char *s);

static inline struct stringlist_node *stringlist_start(struct stringlist *l)
{
    return l->head;
}

static inline bool stringlist_notend(struct stringlist_node *c)
{
    return c != NULL;
}

static inline const char *stringlist_data(struct stringlist_node *c)
{
    return c->data;
}

static inline struct stringlist_node *stringlist_next(struct stringlist_node
                                                      *c)
{
    return c->next;
}

#endif
