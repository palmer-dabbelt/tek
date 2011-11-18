#include "stack.h"

#include <talloc.h>
#include <assert.h>

struct stack *stack_new(struct clopts *o)
{
    struct stack *s;

    s = talloc(o, struct stack);
    s->head = NULL;

    return s;
}

void stack_push(struct stack *s, char *filename)
{
    struct stack_node *n;

    assert(s != NULL);
    assert(filename != NULL);

    n = talloc(s, struct stack_node);
    assert(n != NULL);
    n->filename = talloc_reference(n, filename);
    assert(n->filename != NULL);
    n->next = s->head;
    s->head = n;
}

char *stack_pop(struct stack *s, void *parent)
{
    char *out;
    struct stack_node *old;

    assert(s != NULL);

    if (s->head == NULL)
	return NULL;

    out = talloc_reparent(s->head, parent, s->head->filename);
    old = s->head;
    s->head = s->head->next;
    TALLOC_FREE(old);

    return out;
}
