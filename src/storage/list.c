#include "list.h"

#include <stdlib.h>

int create_list(list *dest) {
    dest->dummy = malloc(sizeof(node));
    if (dest->dummy == NULL) {
        return -1; 
    }

    dest->dummy->next = dest->dummy;
    dest->dummy->prev = dest->dummy;
    return 0;
}

int push_back(list l, void *data) {
    node *new_node = malloc(sizeof(node));
    if (new_node == NULL) {
        return -1;
    }
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
