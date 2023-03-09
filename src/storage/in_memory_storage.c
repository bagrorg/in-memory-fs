#include "in_memory_storage.h"

#include <stdlib.h>
#include <string.h>


void add_im_inode(im_storage *st, im_inode inode) {
    st->_inodes[st->_cur] = inode;
    st->_cur++;
}

im_storage create_im_storage() {
    im_storage st;
    st._cur = 0;
    st._fstree = im_tree_create();
    return st;
}

void delete_im_storage(im_storage *st) {
    ///////////////////////
    // Asserts for debug //
    ///////////////////////
    #ifdef DEBUG
    assert(st != NULL);
    #endif


    //////////////////////////////
    // Free data for all inodes //
    //////////////////////////////
    for (size_t i = 0; i < st->_cur; i++) {
        if (st->_inodes[i]._data == NULL) continue;
        free(st->_inodes[i]._data);
    }

    im_tree_delete(&st->_fstree);
}

static size_t max(size_t x, size_t y) {
    return x > y ? x : y;
}

int im_write(im_inode *inode, const char *data, size_t size, size_t offset) {
    //////////////////////
    // Check logic size //
    //////////////////////
    if (inode->_stat.st_size < offset) {    // TODO OR =?
        return -1;  // TODO
    }

    if (data == NULL) {
        return -1;              //TODO
    }

    ////////////////////////////
    // Realloc data if needed //
    ////////////////////////////
    if (inode->_capacity <= offset + size) {    // TODO =?
        char *new_ptr = realloc(inode->_data, offset + size);
        if (new_ptr == NULL) {
            return -1;      // TODO
        }

        inode->_capacity = offset + size;
        inode->_data = new_ptr;
    }

    ///////////////////////
    // Asserts for debug //
    ///////////////////////
    #ifdef DEBUG
    assert(inode->_data != NULL);
    #endif

    //////////////////////////////
    // Set data and recalc size //
    //////////////////////////////
    if (memcpy(inode->_data + offset, data, size) == NULL) {        //TODO valid?
        return -1;          // TODO errno
    }
    
    inode->_stat.st_size = max(inode->_stat.st_size, offset + size);

    return 0;
}

int im_read(im_inode *inode, char *data, size_t size, size_t offset) {
    //////////////////////
    // Check logic size //
    //////////////////////
    if (inode->_stat.st_size < offset || inode->_stat.st_size < offset + size) { //TODO valid?
        return -1;  //TODO
    }

    // TODO SHOULD WE ASSUME THAT DATA ALLOCATED?

    ///////////////////////
    // Asserts for debug //
    ///////////////////////
    #ifdef DEBUG
    assert(inode->_data != NULL);
    #endif

    if (memcpy(data, inode->_data + offset, size) == NULL) {    //TODO valid?
        return -1; //TODO
    }

    return 0;
}

unsigned long im_create(im_storage *st) {
    ///////////////////////
    // Asserts for debug //
    ///////////////////////
    #ifdef DEBUG
    assert(st != NULL);
    #endif

    //////////////////
    // Create inode //
    //////////////////
    im_inode inode;
    inode._capacity = 0;
    inode._stat.st_size = 0;
    inode._stat.st_ino = st->_cur++;
    inode._data = NULL;

    add_im_inode(st, inode);

    return inode._stat.st_ino;
}

int im_tree_add_entry(im_storage *st, const char *path, im_tree_node *node) {
    ///////////////////////
    // Asserts for debug //
    ///////////////////////
    #ifdef DEBUG
    assert(tree != NULL);
    assert(node != NULL);
    assert(path != NULL);
    #endif

    im_tree_node *parent = im_tree_get_entry(st, path);


    ///////////////////////////////////////
    // Add entry on successfull traverse //
    ///////////////////////////////////////
    if (parent != NULL) {
        if (!parent->dir) {
            return -1;
        } 
        
        im_tree_node **new_data = (im_tree_node **) realloc(parent->entries, (parent->entries_count + 1) * sizeof(im_tree_node *));
        if (new_data == NULL) {
            return -1;
        }

        parent->entries = new_data;
        parent->entries[parent->entries_count] = node;
        parent->entries_count++;
    }

    return 0;
}

im_tree_node* im_tree_get_entry(im_storage *st, const char *path) {
    ///////////////////////
    // Asserts for debug //
    ///////////////////////
    #ifdef DEBUG
    assert(tree != NULL);
    assert(path != NULL);
    #endif

    //////////////////
    // Return value //
    //////////////////
    int ret = 0;

    //////////////////////////
    // Path to iterate over //
    //////////////////////////
    char *path_tmp = strdup(path);
    if (path_tmp == NULL) {
        return NULL;
    }
    char *fname = strtok(path_tmp, "/");

    /////////////////////////
    // Traversing the tree //
    /////////////////////////
    im_tree_node *cur = &st->_fstree.root_node;

    while (fname != NULL) {
        if (!cur->dir) {
            return NULL;
        }

        for (size_t i = 0; i < cur->entries_count; i++) {
            if (strcmp(cur->entries[i]->fname, fname) == 0) {
                cur = cur->entries[i];
                break;
            }
        }

        if (cur == NULL) {
            return NULL;
        }

        fname = strtok(NULL, "/");
    }

    free(path_tmp);
    return cur;
}

bool im_tree_exists(im_storage *st, const char *path) {
    return (im_tree_get_entry(st, path) != NULL);
}

im_tree im_tree_create() {
    im_tree_node root = {
        .entries_count = 0,
        .entries = NULL,
        .fname = "",
        .dir = true,
        .inode = 0                      //TODO
    };

    im_tree tree = {
        .root_node = root, 
    };

    return tree;
}

void im_tree_delete_node(im_tree_node *node) {
    if (node->dir) {
        for (size_t i = 0; i < node->entries_count; i++) {
            im_tree_delete_node(node->entries[i]);
        }
        free(node->entries);
    }

    // Should delete fname here?
}

void im_tree_delete(im_tree *tree) {
    im_tree_delete_node(&tree->root_node); 
}
