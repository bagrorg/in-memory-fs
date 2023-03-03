#ifndef IN_MEMORY_STORAGE_H
#define IN_MEMORY_STORAGE_H

#include <sys/stat.h>
#include <stddef.h>

#define INODES_SIZE 1024

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

} im_storage;

void add_im_inode(im_storage *st, im_inode inode);
im_storage create_im_storage();
void delete_im_storage(im_storage *st);

int im_write(im_inode *inode, const char *data, size_t size, size_t offset);
int im_read(im_inode *inode, char *data, size_t size, size_t offset);
unsigned long im_create(im_storage *st);
int path_search(im_storage *st, const char *path);

#endif
