
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

#include "clopts.h"

#include <talloc.h>

struct clopts *clopts_new(int argc, char **argv)
{
    struct clopts *o;

    o = talloc(NULL, struct clopts);
    o->verbose = false;

    return o;
}
