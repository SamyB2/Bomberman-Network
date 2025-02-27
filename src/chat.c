#include "../include/chat.h"

Chat *create_chat() {
  Chat *c = malloc(sizeof(Chat));
  c->messages = init_dequeue();
  c->nb_messages = 0;
  c->max_messages = 5;
  c->destination = 0;
  c->current_message = malloc(sizeof(Line));
  memset(c->prompt, 0, 10);
  sprintf(c->prompt, "[ALL]: >_");
  clear_line(c->current_message);
  return c;
}

void free_chat(Chat *c) {
  free_dequeue(c->messages, free);
  free(c->current_message);
  free(c);
}

void add_message(Chat *c, char *message) {
  if (c->nb_messages == c->max_messages) {
    Node *n = pop_front(c->messages); 
    free(n->data);
    free(n);
    c->nb_messages--;
  }
  push_back(c->messages, message);
  c->nb_messages++;
}

void add_char(Line *current_message, int c) {
  if (current_message->cursor < TEXT_SIZE) {
    current_message->data[current_message->cursor] = (char)c;
    current_message->cursor++;
    current_message->data[current_message->cursor] = '\0';
  }
}

void remove_char(Line *current_message) {
  if (current_message->cursor > 0) {
    current_message->cursor--;
    current_message->data[current_message->cursor] = '\0';
  }
}

void clear_line(Line *current_message) {
  memset(current_message->data, ' ', TEXT_SIZE);
  current_message->cursor = 0;
}

char *get_message(Chat *c, int i) {
  Node *n = get_node(c->messages, i);
  return n->data;
}