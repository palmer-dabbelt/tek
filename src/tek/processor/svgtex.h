
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

#ifndef PROCESSOR_SVGTEX_H
#define PROCESSOR_SVGTEX_H

#include "processor.h"
#include <stdbool.h>

struct processor_svgtex
{
    struct processor p;

    /* Set to true when the image should be cropped after conversion
     * to PDF. */
    bool crop;

    /* Set to true when this is an image that should be converted
     * using inkscape (as opposed to using ImageMagick). */
    bool inkscape;
};

extern void processor_svgtex_boot(void *context);

extern struct processor *processor_svgtex_search(void *context,
                                                      const char *filename);

#endif
