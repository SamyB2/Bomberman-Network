#ifndef deque_H
#define deque_H

#include <stdlib.h>
#include <unistd.h>

typedef struct Node {
  void *data;
  struct Node *next;
} Node;

typedef struct deque {
  Node *start;
  Node *end;
} Deque;

Deque *init_deque(void);
void push_back(Deque *Deque, void *toAdd);
void push_front(Deque *Deque, void *toAdd);
Node *pop_front(Deque *Deque);
void remove_el(Deque *Deque, void *toRemove);
Deque *append_deque(Deque *deque1, Deque *deque2);
Node *get_node(Deque *Deque, int index);
void free_deque(Deque *Deque, void (*free_func)(void *));

#endif