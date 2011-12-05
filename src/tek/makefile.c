
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

static int mf_destructor(struct makefile *m);

struct makefile *makefile_new(struct clopts *o)
{
    struct makefile *m;

    m = talloc(o, struct makefile);
    talloc_set_destructor(m, &mf_destructor);
    m->file = fopen("Makefile", "w");
    m->targets_all = stringlist_new(m);
    m->build_list  = stringlist_new(m);

    fprintf(m->file, "SHELL=/bin/bash\n");
    fprintf(m->file, ".PHONY: all tek__all clean\n");
    fprintf(m->file, ".SUFFIXES:\n");
    fprintf(m->file, "all: tek__all\n");
    fprintf(m->file, "\n");

    return m;
}



int mf_destructor(struct makefile *m)
{
    fprintf(m->file, "#Closed\n");

    fclose(m->file);

    return 0;
}

