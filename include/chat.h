#ifndef CHAT_H
#define CHAT_H

#include "../include/deque.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define TEXT_SIZE 255

typedef struct Line
{
    char data[TEXT_SIZE];
    size_t cursor;
} Line;

typedef struct Chat
{
    Deque *messages;     // game messages
    int nb_messages;       // number of messages
    int max_messages;      // maximum number of messages
    int destination;       // 0 for ALL, 1 for TEAM
    char prompt[10];       // prompt message
    Line *current_message; // current player message
} Chat;

Chat *create_chat(void);
void free_chat(Chat *c);
void add_message(Chat *c, char *message);
void add_char(Line *current_message, int c);
void remove_char(Line *current_message);
void clear_line(Line *current_message);
char *get_message(Chat *c, int i);

#endif