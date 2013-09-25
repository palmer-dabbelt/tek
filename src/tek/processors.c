
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

#include "processor/processor.h"
#include "processor/latex.h"
#include "processor/directory.h"
#include "processor/gnuplot.h"
#include "processor/imagemagick.h"
#include "processor/pdfcrop.h"
#include "processor/stex.h"
#include "processor/source.h"
#include "processor/xfig.h"
#include "processor/pdfcopy.h"
#include "processor/svgtex.h"

#include <stdlib.h>

void processors_boot(void *context)
{
    processor_latex_boot(context);
    processor_directory_boot(context);
    processor_gnuplot_boot(context);
    processor_imagemagick_boot(context);
    processor_pdfcrop_boot(context);
    processor_stex_boot(context);
    processor_source_boot(context);
}

struct processor *processors_search(void *context, const char *filename)
{
    struct processor *p;

    p = processor_latex_search(context, filename);
    if (p != NULL)
        return p;

    p = processor_directory_search(context, filename);
    if (p != NULL)
        return p;

    p = processor_gnuplot_search(context, filename);
    if (p != NULL)
        return p;

    p = processor_imagemagick_search(context, filename);
    if (p != NULL)
        return p;

    p = processor_pdfcrop_search(context, filename);
    if (p != NULL)
        return p;

    p = processor_stex_search(context, filename);
    if (p != NULL)
        return p;

    p = processor_source_search(context, filename);
    if (p != NULL)
        return p;

    p = processor_xfig_search(context, filename);
    if (p != NULL)
        return p;

    p = processor_pdfcopy_search(context, filename);
    if (p != NULL)
        return p;

    p = processor_svgtex_search(context, filename);
    if (p != NULL)
        return p;

    return NULL;
}
