#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "../stack.h"
#include "../makefile.h"

struct processor
{
    char *name;
    void (*process) (struct processor *, char *,
                     struct stack *, struct makefile *);
};

extern void processor_process(struct processor *p, char *filename,
                              struct stack *s, struct makefile *mf);

#endif
