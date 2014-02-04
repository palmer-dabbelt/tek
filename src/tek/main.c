
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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "clopts.h"
#include "stack.h"
#include "processors.h"
#include "makefile.h"
#include "global.h"
#include "version.h"

#ifdef HAVE_TALLOC
#include <talloc.h>
#else
#include "extern/talloc.h"
#endif

int main(int argc, char **argv)
{
    struct clopts *o;
    struct stack *s;
    struct makefile *m;
    char *filename;
    void *root_context;
    void *context_argstrdup;
    void *context_search;
    void *context_pop;
    int exitvalue;
    int i;

    /* Starts with no errors */
    exitvalue = 0;

    /* talloc initialization, needs to come before any talloc calls */
    talloc_enable_leak_report();
    talloc_set_log_stderr();

    root_context = talloc_init("main(): root_context");

    /* Checks for command-line arguments */
    if (argc > 1 && strcmp(argv[1], "--with-html") == 0) {
        global_with_html = true;
        argc--;
        argv++;
    }

    if (argc > 1 && strcmp(argv[1], "--version") == 0) {
        printf("tek %s\n", TEK_VERSION);
        argc--;
        argv++;
        TALLOC_FREE(root_context);
        exit(0);
    }

    /* Parses the command-line options and starts up all submodules */
    o = clopts_new(root_context, argc, argv);
    s = stack_new(o);
    processors_boot(o);
    m = makefile_new(o);

    /* Contexts to keep track of any potentially allocated memory in some
     * helper functions.  Hopefully all of these will eventually go away. */
    context_argstrdup = talloc_new(root_context);
    context_pop = talloc_new(root_context);
    context_search = talloc_new(root_context);

    /* No arguments means find all possible tex files */
    if (argc == 1) {
        filename = talloc_strdup(context_argstrdup, "");
        stack_push(s, filename);
        talloc_unlink(context_argstrdup, filename);
    } else {
        for (i = 1; i < argc; i++) {
            filename = talloc_strdup(context_argstrdup, argv[i]);
            stack_push(s, filename);
            talloc_unlink(context_argstrdup, filename);
        }
    }

    /* Keeps parsing files until there aren't any files left. */
    while ((filename = stack_pop(s, context_pop)) != NULL) {
        struct processor *proc;

#ifdef DEBUG
        fprintf(stderr, "Processing '%s'\n", filename);
#endif

        /* Checks that a processor actually exists for this file type */
        proc = processors_search(context_search, filename);
        if (proc == NULL) {
            fprintf(stderr, "No processor for '%s'\n", filename);
            talloc_unlink(context_pop, filename);
            exitvalue = 1;
            continue;
        }

        /* If the processor exists, then use it to process the file. */
        proc->process(proc, filename, s, m);

        /* Cleans up everything we just allocated */
        talloc_unlink(context_search, proc);
        talloc_unlink(context_pop, filename);
    }

    /* Cleanup code, this is checked by talloc automatically on program
     * termination. */
    TALLOC_FREE(root_context);

    return exitvalue;
}
