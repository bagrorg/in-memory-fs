#include "in_memory_storage.h"
#include "list.h"

#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>

int create_im_storage(im_storage *dest) {
    dest->_cur = 0;
    if (im_tree_create(&dest->_fstree)) {
        return -1; 
    }
    if (create_list(&dest->inodes)) {
        im_tree_delete(&dest->_fstree);
        return -1; 
    }
    if (im_create(dest) != 0) {
        return -1;
    }
    
    im_inode *root = get(dest->inodes, 0);
    if (root == NULL) return -1;

    root->_stat.st_mode = S_IFDIR;
    return 0;
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
    im_inode *it;
    while ((it = get(st->inodes, 0)) != NULL) {
        erase(st->inodes, 0);
        free(it->_data);
        free(it);
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
    im_inode *new_inode = malloc(sizeof(im_inode));
    if (new_inode == NULL) return -1;       // BAGRORG TODO???

    im_inode inode = {
        ._capacity = 0,
        ._stat.st_size = 0,
        ._stat.st_ino = st->_cur++,
        ._data = NULL,
        ._open = 0,
    };

    *new_inode = inode;
    
    push_back(st->inodes, new_inode);

    return new_inode->_stat.st_ino;
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
    int ret = 0;

    if (im_tree_exists(st, path)) {
        return -1; //TODO
    }

    char *basename_cpy = strdup(path);  
    char *dirname_cpy = strdup(path);
    if (dirname_cpy == NULL || basename_cpy == NULL) {
        ret = -1;
        goto func_exit;
    }

    char *dirname_str = dirname(dirname_cpy);
    char *basename_str = basename(basename_cpy);

    im_tree_node *parent = im_tree_get_entry(st, dirname_str);


    ///////////////////////////////////////
    // Add entry on successfull traverse //
    ///////////////////////////////////////
    if (parent != NULL) {
        if (!parent->dir) {
            ret = -1;
            goto func_exit;
        } 
        char *fname = strdup(basename_str);
        if (fname == NULL) {
            ret = -1;
            goto func_exit;
        }

        im_tree_node node = {
            .dir = is_dir,
            .inode = inode,
            .entries_count = 0,
            .fname = fname,
            .parent = parent,
            .parent_id = parent->entries_count,
        };
        im_tree_node *new_node = (im_tree_node *) malloc(sizeof(im_tree_node));         //TODO
        if (new_node == NULL) {
            ret = -1;
            goto func_exit;
        }

        *new_node = node;
        
        if (is_dir) {
            if (create_list(&new_node->entries)) {
                free(new_node);
                ret = -1;
                goto func_exit;
            }
            push_back(new_node->entries, new_node);
            push_back(new_node->entries, parent);
            new_node->entries_count = 2;
        }

        push_back(parent->entries, new_node);
        parent->entries_count++;
    } else {
        ret = -1;
        goto func_exit;
    }

func_exit:
    free(basename_cpy);
    free(dirname_cpy);
    return ret;
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
            free(path_tmp);
            return NULL;
        }
        
        bool found = false;
        if (strcmp(fname, ".") == 0) {
            if (cur->entries_count == 0) {
                free(path_tmp);
                return NULL;
            }
            found = true;
            cur = get(cur->entries, 0);
        } else if (strcmp(fname, "..") == 0) {
            if (cur->entries_count < 2) {
                free(path_tmp);
                return NULL;
            }
            found = true;
            cur = get(cur->entries, 1);
        } else {
            // next->next because two first ones are . and ..
            for (node *i = begin(cur->entries)->next->next; i != end(cur->entries); i = i->next) {
                im_tree_node *next = i->data;
                if (strcmp(next->fname, fname) == 0) {
                    cur = next;
                    found = true;
                    break;
                }
            }
        }

        if (cur == NULL || !found) {
            free(path_tmp);
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

int im_tree_create(im_tree *dest) {
    im_tree_node *root = malloc(sizeof(im_tree_node));
    if (root == NULL) return -1;

    im_tree_node root_v = {
        .entries_count = 0,
        .entries = NULL,
        .fname = strdup(""),
        .dir = true,
        .inode = 0,
    };
    *root = root_v;
    if (create_list(&root->entries)) return -1;

    root->entries_count = 2;
    push_back(root->entries, root); 
    push_back(root->entries, root);

    im_tree tree = {
        .root_node = root, 
    };

    *dest = tree;

    return 0;
}

void im_tree_delete_node(im_tree_node *tnode, bool delete_from_parent) {
    if (tnode->dir) {
        im_tree_node *cur;
        erase(tnode->entries, 0);
        erase(tnode->entries, 0);

        while ((cur = get(tnode->entries, 0)) != NULL) {
            im_tree_delete_node(cur, true);
        }
        delete_list(tnode->entries);
    }

    if (delete_from_parent) {
        size_t id = 2;
        for (node *i = begin(tnode->parent->entries)->next->next; i != end(tnode->parent->entries); i = i->next) {
            im_tree_node *pnode = i->data;
            if (strcmp(pnode->fname, tnode->fname) == 0) {
                erase(tnode->parent->entries, id);
                tnode->parent->entries_count--;
                break;
            }
            id++;
        }
    }
    free(tnode->fname);
    free(tnode);
}

void im_tree_delete(im_tree *tree) {
    im_tree_delete_node(tree->root_node, false); 
}
