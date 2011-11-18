#ifndef STACK_H
#define STACK_H

#include "clopts.h"

struct stack_node
{
    char *filename;
    struct stack_node *next;
};

struct stack
{
    struct stack_node *head;
};

extern struct stack *stack_new(struct clopts *o);

extern void stack_push(struct stack *s, char *filename);

extern char *stack_pop(struct stack *s, void *parent);

#endif
