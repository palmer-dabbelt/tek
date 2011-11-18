#include <stdlib.h>
#include <stdio.h>
#include <talloc.h>
#include <assert.h>

#include "clopts.h"
#include "stack.h"

int main(int argc, char **argv)
{
    struct clopts *o;
    struct stack *s;
    char *filename;

    talloc_enable_leak_report();
    talloc_set_log_stderr();

    o = clopts_new(argc, argv);
    s = stack_new(o);

    /* FIXME: Don't assume the first option is a tex file */
    assert(argc == 2);
    assert(argv[1] != NULL);
    filename = talloc_strdup(NULL, argv[1]);
    stack_push(s, filename);
    talloc_unlink(NULL, filename);

    filename = stack_pop(s, NULL);
    printf("filename: %s\n", filename);
    talloc_unlink(NULL, filename);

    TALLOC_FREE(o);

    return 0;
}
