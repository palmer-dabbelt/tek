#ifndef PROCESSOR_LATEX_H
#define PROCESSOR_LATEX_H

#include "processor.h"

struct processor_latex
{
    struct processor p;
};

extern void processor_latex_boot(void *context);

extern struct processor *processor_latex_search(void *context,
						const char *filename);

#endif

