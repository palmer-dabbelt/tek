
/*
 * Copyright (C) 2011 Daniel Dabbelt
 *   <palmem@comcast.net>
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
#include <talloc.h>
#include <assert.h>

#include "clopts.h"
#include "stack.h"
#include "processors.h"
#include "makefile.h"

int main(int argc, char **argv)
{
    struct clopts *o;
    struct stack *s;
    struct makefile *m;
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
    m = makefile_new(o);

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

#ifdef DEBUG
        fprintf(stderr, "Processing '%s'\n", filename);
#endif

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
	proc->process(proc, filename, s, m);

	/* Cleans up everything we just allocated */
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
