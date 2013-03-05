
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

#include "stack.h"

#include <assert.h>

#ifdef HAVE_TALLOC
#include <talloc.h>
#else
#include "extern/talloc.h"
#endif

struct stack *stack_new(struct clopts *o)
{
    struct stack *s;

    s = talloc(o, struct stack);
    s->head = NULL;
    s->processed = stringlist_new(s);

    return s;
}

void stack_push(struct stack *s, char *filename)
{
    struct stack_node *n;

    assert(s != NULL);
    assert(filename != NULL);

    if (stringlist_include(s->processed, filename))
        return;

    stringlist_add(s->processed, filename);

    n = talloc(s, struct stack_node);
    assert(n != NULL);
    n->filename = talloc_reference(n, filename);
    assert(n->filename != NULL);
    n->next = s->head;
    s->head = n;
}

char *stack_pop(struct stack *s, void *parent)
{
    char *out;
    struct stack_node *old;

    assert(s != NULL);

    if (s->head == NULL)
        return NULL;

    out = talloc_reparent(s->head, parent, s->head->filename);
    old = s->head;
    s->head = s->head->next;
    TALLOC_FREE(old);

    return out;
}
