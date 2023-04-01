#pragma once

#include <stddef.h>

typedef struct node {
    struct node *next;
    struct node *prev;

    void *data;
} node;

typedef struct list {
    node *dummy;
} list;

list create_list();
void push_back(list l, void *data);
void *get(list l, size_t i);
void erase(list l, size_t i);
void delete_list(list l);
