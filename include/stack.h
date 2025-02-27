#ifndef STACK_H
#define STACK_H

#include "pos.h"

void push(void* elem, void* stack, int *top, size_t elemSize);
void pop(void* elem, void* stack, int *top, size_t elemSize);
void peek(void* elem, void* stack, int *top, size_t elemSize);
int isEmpty(int *top);

#endif
