#include "../include/deque.h"

// Ini
Deque *init_deque() {
  Deque *d = malloc(sizeof(Deque));
  d->start = NULL;
  d->end = NULL;
  return d;
}

void push_back(Deque *d, void *toAdd) {
  Node *node = malloc(sizeof(Node));
  node->data = toAdd;
  node->next = NULL;
  if (d->start == NULL) {
    d->start = node;
    d->end = node;
  } else {
    d->end->next = node;
    d->end = node;
  }
}

void push_front(Deque *d, void *toAdd) {
  Node *node = malloc(sizeof(Node));
  node->data = toAdd;
  node->next = d->start;
  d->start = node;
  if (d->end == NULL) {
    d->end = node;
  }
}

Node *pop_front(Deque *d) {
  if (d->start == NULL) {
    return NULL;
  }
  Node *node = d->start;
  d->start = node->next;
  if (d->start == NULL) {
    d->end = NULL;
  }
  node->next = NULL;
  return node;
}

void remove_el(Deque *d, void *toRemove) {
  Node *node = d->start;
  Node *prev = NULL;
  while (node != NULL) {
    if (node->data == toRemove) {
      if (prev == NULL) {
        d->start = node->next;
      } else {
        prev->next = node->next;
      }
      if (node == d->end) {
        d->end = prev;
      }
      free(node);
      return;
    }
    prev = node;
    node = node->next;
  }
}

Deque *append_deque(Deque *d1, Deque *d2) {
  if (d1->start == NULL) {
    return d2;
  }
  if (d2->start == NULL) {
    return d1;
  }
  d1->end->next = d2->start;
  d1->end = d2->end;
  return d1;
}

Node *get_node(Deque *d, int index) {
  Node *node = d->start;
  for (int i = 0; i < index; i++) {
    if (node == NULL) {
      return NULL;
    }
    node = node->next;
  }
  return node;
}

void free_deque(Deque *d, void (*free_func)(void *)) {
  Node *node = d->start;
  while (node != NULL) {
    Node *next = node->next;
    free_func(node->data);
    free(node);
    node = next;
  }
  free(d);
}