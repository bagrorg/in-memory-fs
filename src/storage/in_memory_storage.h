#ifndef IN_MEMORY_STORAGE_H
#define IN_MEMORY_STORAGE_H

#include <sys/stat.h>
#include <stddef.h>
#include <stdbool.h>

#define INODES_SIZE 1024

typedef struct im_tree_node {
    bool dir;
    size_t inode;  
    const char *fname;
 
    struct im_tree_node *entries; 
    size_t entries_count;
} im_tree_node;

typedef struct im_tree {
    im_tree_node root_node;
} im_tree;

typedef struct im_inode {
    struct stat _stat;
    struct im_inode *_parent;

    size_t _capacity;
    char *_data;
    const char *_path;
} im_inode;

typedef struct im_storage {
    // We can use fi->fh as pointer and just set it to proper im_inode
    // more info here: https://www.cs.hmc.edu/~geoff/classes/hmc.cs135.201109/homework/fuse/fuse_doc.html
    // at 'FUSE File Handles'
    im_inode _inodes[INODES_SIZE];
    size_t _cur;
    im_tree _fstree;
} im_storage;


void add_im_inode(im_storage *st, im_inode inode);
im_storage create_im_storage();
void delete_im_storage(im_storage *st);

int im_write(im_inode *inode, const char *data, size_t size, size_t offset);
int im_read(im_inode *inode, char *data, size_t size, size_t offset);
unsigned long im_create(im_storage *st);

int im_tree_add_entry(im_storage *st, const char *path, bool is_dir, size_t inode);
im_tree_node* im_tree_get_entry(im_storage *st, const char *path);
bool im_tree_exists(im_storage *st, const char *path);

im_tree im_tree_create();
void im_tree_delete(im_tree *tree);
#endif
