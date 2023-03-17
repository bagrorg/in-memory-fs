#include "list.h"

#include <stdlib.h>

list create_list() {
    list l;
    l.dummy = malloc(sizeof(node));
    l.dummy->next = l.dummy;
    l.dummy->prev = l.dummy;
    return l;
}

void push_back(list l, void *data) {
    node *new_node = malloc(sizeof(node));
    new_node->data = data;
    new_node->prev = l.dummy->prev;
    l.dummy->prev->next = new_node;
    l.dummy->prev = new_node;
    new_node->next = l.dummy;
}

void *get(list l, size_t i) {
    if (l.dummy->next == l.dummy) return NULL;

    node *cur = l.dummy->next;
    while (cur != l.dummy && i != 0) {
        cur = cur->next;
        i--;
    }

    if (i != 0 || cur == l.dummy) return NULL;
    else return cur->data;
}

void erase(list l, size_t i) {    
    if (l.dummy->next == l.dummy) return;

    node *cur = l.dummy->next;
    while (cur != l.dummy && i != 0) {
        cur = cur->next;
        i--;
    }

    if (i != 0 || cur == l.dummy) return;
    cur->next->prev = cur->prev;
    cur->prev->next = cur->next;
    free(cur);
}

void delete_list(list l) { 
    node *cur = l.dummy->next;
    while (cur != l.dummy) {
        node *next = cur->next;
        free(cur);
        cur = next;
    }

    free(l.dummy);
}
