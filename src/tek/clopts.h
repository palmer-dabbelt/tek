#ifndef CLOPTS_H
#define CLOPTS_H

#include <stdbool.h>

struct clopts
{
    bool verbose;
};

extern struct clopts *clopts_new(int argc, char **argv);

#endif
