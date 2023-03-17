#include <asm-generic/errno.h>
#define FUSE_USE_VERSION 30

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

#include <stdio.h>
#include <stdlib.h>

static im_storage *st;
FILE * fp;

/** Create a directory */
int im_fuse_mkdir(const char *path, mode_t mode) {
    fprintf(fp, "\nim_mkdir(path=\"%s\", mode=0%3o)\n",
	    path, mode);
    fflush(fp);

    if (!im_tree_exists(st, path)) {
        unsigned long node_id = im_create(st);
        if (node_id == -1) {
            return -1;
        }
        im_inode *node = get(st->inodes, node_id);
        node->_path = path;
        node->_stat.st_mode = mode | S_IFDIR;
        node->_stat.st_gid =fuse_get_context()->gid;
        node->_stat.st_uid =fuse_get_context()->uid;
        node->_open = 0; 

        im_tree_add_entry(st, path, true, node_id);             /// BAGRORG TODO

        return 0; 
    }

    return -EEXIST;
}

int im_fuse_mknod(const char *path, mode_t mode, dev_t dev) {
    fprintf(fp, "\nim_mknod(path=\"%s\", mode=0%3o, dev=%ld)\n",
	  path, mode, dev);
    fflush(fp);
    if (!im_tree_exists(st, path)) {
        unsigned long node_id = im_create(st); 
        im_inode *node = get(st->inodes, node_id);
        
        node->_path = path;
        node->_stat.st_mode = mode | S_IFREG;
        node->_stat.st_gid =fuse_get_context()->gid;
        node->_stat.st_uid =fuse_get_context()->uid;
        node->_open = 0;
        
        im_tree_add_entry(st, path, false, node_id);        // BAGRORG TODO

        // TODO ADD PARENT AND AT MKDIR
        //
        // ?
        return 0;
    }
    return -EEXIST;
}

int im_fuse_getattr(const char *path, struct stat *statbuf) {
    fprintf(fp, "\nbb_getattr(path=\"%s\")\n", path);
    fflush(fp);

    if (strcmp(path, "/") == 0) {
        statbuf->st_mode = S_IFDIR | 0755;
        statbuf->st_nlink = 2;
        //statbuf->st_size = SIZE;
        return 0;
    } 

    im_tree_node *fsnode = im_tree_get_entry(st, path);
    if (fsnode == NULL) {
        return -ENOENT;
    }

    size_t id = fsnode->inode;

    if (id != -1) {
        im_inode *node = get(st->inodes, id);
        *statbuf = node->_stat;   
        return 0; 
    }

    return -ENODATA;
}

/** Open directory */
int im_fuse_opendir(const char *path, struct fuse_file_info *fi) {
    fprintf(fp, "\nim_opendir(path=\"%s\")\n", path);
    fflush(fp);

    im_tree_node* entry = im_tree_get_entry(st, path);
    if (entry == NULL || entry->inode == -1) {
        return -ENOENT;
    } 
    im_inode *node = get(st->inodes, entry->inode);

    if (!(node->_stat.st_mode & S_IFDIR)) {
        return -ENOTDIR;
    }

    node->_open++;
    fi->fh = entry->inode;

    return 0;
}

/** Read directory 
 * The readdir implementation ignores the offset parameter, and 
 * passes zero to the filler function's offset. The filler 
 * function will not return '1' (unless an error happens), so the
 * whole directory is read in a single readdir operation.
*/
int im_fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
	       struct fuse_file_info *fi) {
    fprintf(fp, "\nim_readdir(path=\"%s\", offset=%ld)\n",
	    path, offset);
    fflush(fp);

    int node_id = fi->fh;
    if (node_id == -1) {
        return -EBADF;
    }

    im_tree_node* entry = im_tree_get_entry(st, path);

    if (!entry->dir) {
        return -ENOTDIR;
    }

    if (filler(buf, ".", NULL, 0) != 0) {
        return -ENOMEM;
    }

    if (filler(buf, "..", NULL, 0) != 0) {
        return -ENOMEM;
    }

    for (size_t i = 2; i < entry->entries_count; i++) { 
        im_tree_node *node = get(entry->entries, i);
        if (filler(buf, node->fname, NULL, 0) != 0) {
            return -ENOMEM;
        }
    }
    
    return 0;
}

/** Release directory */
int im_fuse_releasedir(const char *path, struct fuse_file_info *fi) {
    fprintf(fp, "\nim_releasedir(path=\"%s\")\n", path);
    fflush(fp);
    int node_id = fi->fh;

    if (node_id == -1) {
        return -ENOENT;
    }
    im_inode *node = get(st->inodes, node_id);

    if (!(node->_stat.st_mode & S_IFDIR)) {
        return -ENOTDIR;
    }

    if (node->_open == 0) {
        return -EBADF;
    }

    node->_open--;

    return 0;
}

/** Remove a file */
int im_fuse_unlink(const char *path) {
    fprintf(fp, "bb_unlink(path=\"%s\")\n",
	    path);
    fflush(fp);

    im_tree_node *fsnode = im_tree_get_entry(st, path); 

    if (fsnode == NULL) {
        return -ENOENT;
    }


    if (fsnode->dir) {
        return -EISDIR;
    }
    
    im_inode *node = get(st->inodes, fsnode->inode);

    if (node->_open > 0) {
        return -EBUSY;
    }

    im_tree_delete_node(fsnode, true);
    return 0;
    
}

/** Remove a directory */
int im_fuse_rmdir(const char *path) {
    fprintf(fp, "bb_rmdir(path=\"%s\")\n",
	    path);
    fflush(fp);

    im_tree_node *fsnode = im_tree_get_entry(st, path); 

    if (fsnode == NULL) {
        return -ENOENT;
    }


    if (!fsnode->dir) {
        return -ENOTDIR;
    }

    im_inode *node = get(st->inodes, fsnode->inode);

    if (node->_open > 0) {
        return -EBUSY;
    }


    im_tree_delete_node(fsnode, true);
    return 0;
}


/** Change the size of a file */
int im_fuse_truncate(const char *path, off_t newsize) {
    fprintf(fp, "\nim_truncate(path=\"%s\", newsize=%ld)\n",
	    path, newsize);
    fflush(fp);

    im_tree_node *fsnode = im_tree_get_entry(st, path); 
    size_t node_id = -1;
    im_inode *node;
    if (fsnode == NULL) {
        node_id = im_create(st);
        node = get(st->inodes, node_id);
        node->_path = path;
    } else if (fsnode->dir) {
        return -EISDIR;
    } else {
        node_id = fsnode->inode; 
        node = get(st->inodes, node_id);
    }

    if (node->_capacity >= newsize) {
        node->_capacity = newsize;
    } else {
        node->_capacity = newsize;
        char* new_data = calloc(newsize, sizeof(char));
        memcpy(new_data, node->_data, node->_capacity);
        free(node->_data);
        node->_data = new_data;
    }

    return 0;
}

/** Release an open file */
int im_fuse_release(const char *path, struct fuse_file_info *fi) {
    fprintf(fp, "\nim_release(path=\"%s\")\n", path);
    fflush(fp);
    if (fi == NULL) {
        return -EBADF;
    }

    int node_id = fi->fh;
    im_inode *node = get(st->inodes, node_id);

    if (!(node->_stat.st_mode & S_IFDIR)) {
        return -ENOTDIR;
    }

    if (node->_open == 0) {
        return -EBADF;
    }

    node->_open--;
    return 0;
}

/** File open operation */
int im_fuse_open(const char *path, struct fuse_file_info *fi) {
    fprintf(fp, "\nim_open(path\"%s\")\n", path);
    fflush(fp);
    im_tree_node *fsnode = im_tree_get_entry(st, path); 
    if (fsnode == NULL || fsnode->inode == -1) {
        return -ENOENT;
    }


    im_inode *node = get(st->inodes, fsnode->inode);
    if (node->_stat.st_mode & S_IFDIR) {
        return -EISDIR;
    }

    node->_open++;
    fi->fh = fsnode->inode;              
    return 0;
}

/** Read data from an open file */
int im_fuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    fprintf(fp, "\nim_read(path=\"%s\", size=%ld, offset=%ld)\n", path, size, offset);
    fflush(fp);
    size_t node_id = fi->fh;

    im_inode *node = get(st->inodes, node_id);
    if (node_id == -1 || node->_open == 0) {
        return -EBADF;
    }

    return im_read(node, buf, size, offset);
}

/** Write data to an open file */
int im_fuse_write(const char *path, const char *buf, size_t size, off_t offset,
	     struct fuse_file_info *fi) {
    fprintf(fp, "\nim_write(path=\"%s\", size=%ld, offset=%ld)\n",
	    path, size, offset);
    fflush(fp);
    size_t node_id = fi->fh;
    im_inode *node = get(st->inodes, node_id);

    if (node_id == -1 || node->_open == 0) {
        return -EBADF;
    }
    return im_write(node, buf, size, offset);
}


struct fuse_operations bb_oper = {
    .mkdir = im_fuse_mkdir,
    .mknod = im_fuse_mknod,

    .getattr = im_fuse_getattr,
    .opendir = im_fuse_opendir,
    .readdir = im_fuse_readdir,
    .releasedir = im_fuse_releasedir,

    .unlink = im_fuse_unlink,
    .rmdir = im_fuse_rmdir,

    .open = im_fuse_open,
    .read = im_fuse_read,
    .write = im_fuse_write,
    .release = im_fuse_release,
    .truncate = im_fuse_truncate,
};

int main(int argc, char **argv) {
    fp = fopen("logs.txt", "w+");

    st = malloc(sizeof(im_storage));
    *st = create_im_storage();

    printf("Started\n");
    fuse_main(argc, argv, &bb_oper, NULL);
    printf("Ended\n");

    delete_im_storage(st);
    free(st);
    return 0;
}
