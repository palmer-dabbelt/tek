#include "latex.h"

#include <string.h>
#include <stdbool.h>
#include <talloc.h>

static bool string_ends_with(const char *string, const char *end);

static char *p_name;

void processor_latex_boot(void *context)
{
    p_name = talloc_strdup(context, "LATEX");
}

struct processor *processor_latex_search(void *context, const char *filename)
{
    struct processor_latex *p;

    p = NULL;

    if (string_ends_with(filename, ".tex"))
        p = talloc(context, struct processor_latex);

    if (p != NULL)
    {
        p->p.name = talloc_reference(p, p_name);
    }

    return (struct processor *)p;
}

bool string_ends_with(const char *string, const char *end)
{
    return strcmp(string + strlen(string) - strlen(end), end) == 0;
}
