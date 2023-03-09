#include "src/storage/in_memory_storage.h"

#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <fuse.h>
#include <fuse/fuse.h>
#include <sys/stat.h>

static im_storage *st;

/** Create a directory */
int im_fuse_mkdir(const char *path, mode_t mode) {
    if (!im_tree_exists(st, path)) {
        unsigned long node_id = im_create(st); //TODO ERROR?
        
        st->_inodes[node_id]._path = path;
        st->_inodes[node_id]._stat.st_mode = mode | S_IFDIR;
        st->_inodes[node_id]._stat.st_gid =fuse_get_context()->gid;
        st->_inodes[node_id]._stat.st_uid =fuse_get_context()->uid;
       
        im_tree_add_entry(st, path, true, node_id); 

        return 0; 
    }

    return -1;  // TODO ERROR
}

int im_fuse_mknod(const char *path, mode_t mode, dev_t dev) {
    if (!im_tree_exists(st, path)) {
        unsigned long node_id = im_create(st);

        st->_inodes[node_id]._path = path;
        st->_inodes[node_id]._stat.st_mode = mode | S_IFREG;
        st->_inodes[node_id]._stat.st_gid =fuse_get_context()->gid;
        st->_inodes[node_id]._stat.st_uid =fuse_get_context()->uid;
        
        im_tree_add_entry(st, path, false, node_id);

        // TODO ADD PARENT AND AT MKDIR
        //
        //
        return 0;
    }
    return -1;  // TODO OR NEED TO ERROR?
}

int im_fuse_getattr(const char *path, struct stat *statbuf) {
    if (strcmp(path, "/") == 0) {
	statbuf->st_mode = S_IFDIR | 0755;
	statbuf->st_nlink = 2;
	//statbuf->st_size = SIZE;
	return 0;
    } 

    im_tree_node *fsnode = im_tree_get_entry(st, path); 
    size_t id = fsnode->inode;

    if (id != -1) {
        *statbuf = st->_inodes[id]._stat;   
        return 0; 
    }

    return -ENODATA;  ///TODO OR ERROR
}

struct fuse_operations bb_oper = {
    .mkdir = im_fuse_mkdir,
    .mknod = im_fuse_mknod,
    .getattr = im_fuse_getattr,
};

int main(int argc, char **argv) {
    st = malloc(sizeof(im_storage));
    *st = create_im_storage();
    
    printf("Started\n");
    fuse_main(argc, argv, &bb_oper);
    printf("Ended\n");

    delete_im_storage(st);
    free(st);
    return 0;
}
