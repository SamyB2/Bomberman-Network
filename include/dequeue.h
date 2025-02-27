#ifndef DEQUEUE_H
#define DEQUEUE_H

#include <unistd.h>
#include <stdlib.h>

typedef struct Node{
    void* data;
    struct Node* next;
}Node;

typedef struct Dequeue{
    Node* start;
    Node* end;
} Dequeue;

Dequeue* init_dequeue(void);
void push_back(Dequeue* dequeue , void* toAdd);
void push_front(Dequeue* dequeue , void* toAdd);
Node* pop_front(Dequeue* dequeue);
void remove_el(Dequeue* dequeue , void* toRemove);
Dequeue* append_dequeue(Dequeue* dequeue1 , Dequeue* dequeue2);
Node* get_node(Dequeue* dequeue , int index);
void free_dequeue(Dequeue* dequeue , void (*free_func)(void*));

#endif