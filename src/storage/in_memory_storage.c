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


int path_search(im_storage *st, const char *path) {
    for (size_t i = 0; i < st->_cur; i++) {
        if (strcmp(st->_inodes[i]._path, path) == 0) {
            return i; 
        } 
    }
    
    return -1;
}
