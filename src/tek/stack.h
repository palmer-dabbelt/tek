
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

#ifndef STACK_H
#define STACK_H

#include "clopts.h"
#include "stringlist.h"

struct stack_node
{
    char *filename;
    struct stack_node *next;
};

struct stack
{
    struct stack_node *head;
    struct stringlist *processed;
};

extern struct stack *stack_new(struct clopts *o);

extern void stack_push(struct stack *s, char *filename);

extern char *stack_pop(struct stack *s, void *parent);

#endif
