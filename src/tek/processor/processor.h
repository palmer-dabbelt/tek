
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

#ifndef PROCESSOR_H
#define PROCESSOR_H

#ifndef CACHE_DIR
#define CACHE_DIR ".tek_cache"
#endif

#include "../stack.h"
#include "../makefile.h"

struct processor
{
    char *name;
    void (*process) (struct processor *, const char *,
                     struct stack *, struct makefile *);
};

extern void processor_process(struct processor *p, char *filename,
                              struct stack *s, struct makefile *mf);

#endif
