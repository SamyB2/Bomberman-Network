#include "../include/stack.h"
#include <string.h>

void push(void *elem, void *stack, int *top, size_t elemSize) {
  memcpy((char *)stack + (size_t)(*top) * elemSize, elem, elemSize);
  *top = *top + 1;
}

void pop(void *elem, void *stack, int *top, size_t elemSize) {
  *top = *top - 1;
  memcpy(elem, (char *)stack + (size_t)(*top) * elemSize, elemSize);
}

void peek(void *elem, void *stack, int *top, size_t elemSize) {
  memcpy(elem, (char *)stack + ((size_t)(*top - 1) * elemSize), elemSize);
}

int isEmpty(int *top) { return !*top; }
