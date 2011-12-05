#include <stdlib.h>
#include <stdio.h>
#include <talloc.h>
#include <assert.h>

#include "clopts.h"
#include "stack.h"
#include "processors.h"

int main(int argc, char **argv)
{
    struct clopts *o;
    struct stack *s;
    char *filename;
    void *context_argstrdup;
    void *context_search;
    void *context_pop;
    int exitvalue;

    /* Starts with no errors */
    exitvalue = 0;

    /* talloc initialization, needs to come before any talloc calls */
    talloc_enable_leak_report();
    talloc_set_log_stderr();

    /* Parses the command-line options and starts up all submodules */
    o = clopts_new(argc, argv);
    s = stack_new(o);
    processors_boot(o);

    /* Contexts to keep track of any potentially allocated memory in some
     * helper functions.  Hopefully all of these will eventually go away. */
    context_argstrdup = talloc_init("main(): argv strdup");
    context_pop = talloc_init("main(): pop");
    context_search = talloc_init("main(): search");

    /* FIXME: Don't assume the first option is a tex file */
    assert(argc == 2);
    assert(argv[1] != NULL);
    filename = talloc_strdup(context_argstrdup, argv[1]);
    stack_push(s, filename);
    talloc_unlink(context_argstrdup, filename);

    /* Keeps parsing files until there aren't any files left. */
    while ((filename = stack_pop(s, context_pop)) != NULL)
    {
        struct processor *proc;

        fprintf(stderr, "Processing '%s'\n", filename);

        /* Checks that a processor actually exists for this file type */
        proc = processors_search(context_search, filename);
        if (proc == NULL)
        {
            fprintf(stderr, "No processor for '%s'\n", filename);
            talloc_unlink(context_pop, filename);
            exitvalue = 1;
            continue;
        }

        /* If the processor exists, then use it to process the file. */

        talloc_unlink(context_search, proc);
        talloc_unlink(context_pop, filename);
    }

    /* Cleanup code, this is checked by talloc automatically on program
     * termination. */
    TALLOC_FREE(context_search);
    TALLOC_FREE(context_pop);
    TALLOC_FREE(context_argstrdup);
    TALLOC_FREE(o);

    return exitvalue;
}
