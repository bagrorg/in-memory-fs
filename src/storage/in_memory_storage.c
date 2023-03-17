#include "in_memory_storage.h"
#include "list.h"

#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>

im_storage create_im_storage() {
    im_storage st;
    st._cur = 0;
    st._fstree = im_tree_create();
    st.inodes = create_list();
    im_create(&st);
    
    im_inode *root = get(st.inodes, 0);
    root->_stat.st_mode = S_IFDIR;
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
        im_inode *cur = get(st->inodes, i);
        free(cur->_data);
        free(cur);
    }

    im_tree_delete(&st->_fstree);
    delete_list(st->inodes);
}

static size_t max(size_t x, size_t y) {
    return x > y ? x : y;
}

int im_write(im_inode *inode, const char *data, size_t size, size_t offset) {
    //////////////////////
    // Check logic size //
    //////////////////////
    if (inode->_stat.st_size < offset) {    // TODO OR =?
        return -EOVERFLOW;  // TODO
    }

    if (data == NULL) {
        return -EFAULT;              //TODO
    }

    ////////////////////////////
    // Realloc data if needed //
    ////////////////////////////
    if (inode->_capacity <= offset + size) {    // TODO =?
        char *new_ptr = realloc(inode->_data, offset + size);
        if (new_ptr == NULL) {
            return -ENOMEM;      // TODO
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
        return -EFAULT;          // TODO errno
    }
    
    inode->_stat.st_size = max(inode->_stat.st_size, offset + size);

    return size;
}

static size_t min(size_t x, size_t y) {
    return x < y ? x : y;
}

int im_read(im_inode *inode, char *data, size_t size, size_t offset) {
    //////////////////////
    // Check logic size //
    //////////////////////
    if (inode->_stat.st_size < offset) { //TODO valid?
        return -EOVERFLOW;  //TODO
    }

    // TODO SHOULD WE ASSUME THAT DATA ALLOCATED?

    ///////////////////////
    // Asserts for debug //
    ///////////////////////
    #ifdef DEBUG
    assert(inode->_data != NULL);
    #endif

    size_t result_size = min(offset + size, inode->_stat.st_size) - offset;

    if (memcpy(data, inode->_data + offset, result_size) == NULL) {    //TODO valid?
        return -EFAULT; //TODO
    }

    return result_size;
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

    im_inode *inode = malloc(sizeof(im_inode));

    inode->_capacity = 0;
    inode->_stat.st_size = 0;
    inode->_stat.st_ino = st->_cur++;
    inode->_data = NULL;
    
    push_back(st->inodes, inode);

    return inode->_stat.st_ino;
}

int im_tree_add_entry(im_storage *st, const char *path, bool is_dir, size_t inode) {
    ///////////////////////
    // Asserts for debug //
    ///////////////////////
    #ifdef DEBUG
    assert(tree != NULL);
    assert(node != NULL);
    assert(path != NULL);
    #endif
    if (im_tree_exists(st, path)) {
        return -1; //TODO
    }

    char *basename_cpy = strdup(path);  
    char *dirname_cpy = strdup(path);
    if (dirname_cpy == NULL || basename_cpy == NULL) {
        return -1; //TODO
    }

    char *dirname_str = dirname(dirname_cpy);
    char *basename_str = basename(basename_cpy);

    im_tree_node *parent = im_tree_get_entry(st, dirname_str);


    ///////////////////////////////////////
    // Add entry on successfull traverse //
    ///////////////////////////////////////
    if (parent != NULL) {
        if (!parent->dir) {
            return -1;
        } 
         
        im_tree_node node = {
            .dir = is_dir,
            .inode = inode,
            .entries_count = 0,
            .fname = basename_str,
            .parent = parent,
            .parent_id = parent->entries_count,
        };
        im_tree_node *new_node = (im_tree_node *) malloc(sizeof(im_tree_node));         //TODO
        *new_node = node;
        
        if (is_dir) {
            new_node->entries = create_list();
            push_back(new_node->entries, new_node);
            push_back(new_node->entries, parent);
            new_node->entries_count = 2;
        }

        push_back(parent->entries, new_node);
        parent->entries_count++;
    } else {
        return -1;
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
    im_tree_node *cur = st->_fstree.root_node;

    while (fname != NULL) {
        if (!cur->dir) {
            return NULL;
        }
        
        bool found = false;
        if (strcmp(fname, ".") == 0) {
            if (cur->entries_count == 0) {
                return NULL;
            }
            found = true;
            cur = get(cur->entries, 0);
        } else if (strcmp(fname, "..") == 0) {
            if (cur->entries_count < 2) {
                return NULL;
            }
            found = true;
            cur = get(cur->entries, 1);
        } else {
            for (size_t i = 0; i < cur->entries_count; i++) {
                im_tree_node *next = get(cur->entries, i);
                if (strcmp(next->fname, fname) == 0) {
                    cur = next;
                    found = true;
                    break;
                }
            }
        }

        if (cur == NULL || !found) {
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
    im_tree_node *root = malloc(sizeof(im_tree_node));
    im_tree_node root_v = {
        .entries_count = 0,
        .entries = NULL,
        .fname = "",
        .dir = true,
        .inode = 0,
    };
    *root = root_v;
    root->entries = create_list();
    root->entries_count = 2;
    push_back(root->entries, root); 
    push_back(root->entries, root);

    im_tree tree = {
        .root_node = root, 
    };

    return tree;
}

void im_tree_delete_node(im_tree_node *node, bool delete_from_parent) {
    if (node->dir) {
        im_tree_node *cur;
        erase(node->entries, 0);
        erase(node->entries, 0);

        while ((cur = get(node->entries, 0)) != NULL) {
            im_tree_delete_node(cur, true);
        }
        delete_list(node->entries);
    }

    if (delete_from_parent) {
        for (size_t i = 0; i < node->parent->entries_count; i++) {
            im_tree_node *pnode = get(node->parent->entries, i);
            if (strcmp(pnode->fname, node->fname) == 0) {
                erase(node->parent->entries, i);
                node->parent->entries_count--;
                break;
            }
        }
    }
    free(node);
}

void im_tree_delete(im_tree *tree) {
    im_tree_delete_node(tree->root_node, false); 
}
