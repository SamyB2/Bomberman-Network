#include "../include/dequeue.h"

Dequeue *init_dequeue() {
  Dequeue *dequeue = malloc(sizeof(Dequeue));
  dequeue->start = NULL;
  dequeue->end = NULL;
  return dequeue;
}

void push_back(Dequeue *dequeue, void *toAdd) {
  Node *node = malloc(sizeof(Node));
  node->data = toAdd;
  node->next = NULL;
  if (dequeue->start == NULL) {
    dequeue->start = node;
    dequeue->end = node;
  } else {
    dequeue->end->next = node;
    dequeue->end = node;
  }
}

void push_front(Dequeue *dequeue, void *toAdd) {
  Node *node = malloc(sizeof(Node));
  node->data = toAdd;
  node->next = dequeue->start;
  dequeue->start = node;
  if (dequeue->end == NULL) {
    dequeue->end = node;
  }
}

Node *pop_front(Dequeue *dequeue) {
  if (dequeue->start == NULL) {
    return NULL;
  }
  Node *node = dequeue->start;
  dequeue->start = node->next;
  if (dequeue->start == NULL) {
    dequeue->end = NULL;
  }
  return node;
}

void remove_el(Dequeue *dequeue, void *toRemove) {
  Node *node = dequeue->start;
  Node *prev = NULL;
  while (node != NULL) {
    if (node->data == toRemove) {
      if (prev == NULL) {
        dequeue->start = node->next;
      } else {
        prev->next = node->next;
      }
      if (node == dequeue->end) {
        dequeue->end = prev;
      }
      free(node);
      return;
    }
    prev = node;
    node = node->next;
  }
}

Dequeue *append_dequeue(Dequeue *dequeue1, Dequeue *dequeue2) {
  if (dequeue1->start == NULL) {
    return dequeue2;
  }
  if (dequeue2->start == NULL) {
    return dequeue1;
  }
  dequeue1->end->next = dequeue2->start;
  dequeue1->end = dequeue2->end;
  return dequeue1;
}

Node *get_node(Dequeue *dequeue, int index) {
  Node *node = dequeue->start;
  for (int i = 0; i < index; i++) {
    if (node == NULL) {
      return NULL;
    }
    node = node->next;
  }
  return node;
}

void free_dequeue(Dequeue *dequeue, void (*free_func)(void *)) {
  Node *node = dequeue->start;
  while (node != NULL) {
    Node *next = node->next;
    free_func(node->data);
    free(node);
    node = next;
  }
  free(dequeue);
}
