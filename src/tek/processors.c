#include "processor/processor.h"
#include "processor/latex.h"

#include <stdlib.h>

void processors_boot(void *context)
{
    processor_latex_boot(context);
}

struct processor *processors_search(void *context, const char *filename)
{
    struct processor *p;

    p = processor_latex_search(context, filename);
    if (p != NULL)
        return p;

    return NULL;
}
