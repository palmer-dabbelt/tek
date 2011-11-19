#ifndef PROCESSORS_H
#define PROCESSORS_H

#include "processor/processor.h"

extern void processors_boot(void *context);

extern struct processor *processors_search(void *context, const char *filename);

#endif
