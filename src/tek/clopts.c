#include "clopts.h"

#include <talloc.h>

struct clopts *clopts_new(int argc, char **argv)
{
    struct clopts *o;

    o = talloc(NULL, struct clopts);
    o->verbose = false;

    return o;
}
