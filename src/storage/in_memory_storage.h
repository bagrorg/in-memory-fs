#ifndef IN_MEMORY_STORAGE_H
#define IN_MEMORY_STORAGE_H

#include <sys/stat.h>
#include <stddef.h>
#include <stdbool.h>
#include "list.h"

typedef struct im_tree_node {
    bool dir;
    size_t inode;  
    char *fname;
 
    list entries; 
    size_t entries_count;

    struct im_tree_node *parent;
    size_t parent_id;
} im_tree_node;

typedef struct im_tree {
    im_tree_node *root_node;
} im_tree;

typedef struct im_inode {
    struct stat _stat;

    int _open;
    size_t _capacity;
    char *_data;
    const char *_path;
} im_inode;

typedef struct im_storage {
    // We can use fi->fh as pointer and just set it to proper im_inode
    // more info here: https://www.cs.hmc.edu/~geoff/classes/hmc.cs135.201109/homework/fuse/fuse_doc.html
    // at 'FUSE File Handles'
    list inodes;
    size_t _cur;
    im_tree _fstree;
} im_storage;


void add_im_inode(im_storage *st, im_inode inode);
int create_im_storage(im_storage *dest);
void delete_im_storage(im_storage *st);

int im_write(im_inode *inode, const char *data, size_t size, size_t offset);
int im_read(im_inode *inode, char *data, size_t size, size_t offset);
unsigned long im_create(im_storage *st);

int im_tree_add_entry(im_storage *st, const char *path, bool is_dir, size_t inode);
im_tree_node* im_tree_get_entry(im_storage *st, const char *path);
bool im_tree_exists(im_storage *st, const char *path);

int im_tree_create(im_tree *dest);
void im_tree_delete(im_tree *tree);
void im_tree_delete_node(im_tree_node *node, bool delete_from_parent);
#endif
