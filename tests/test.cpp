#include <asm-generic/errno.h>
#include <cstddef>
#include <cstring>
#include <gtest/gtest-death-test.h>
#include <gtest/gtest.h>
#include <string>

extern "C" {
    #include "../src/storage/in_memory_storage.h"
    #include "../src/storage/list.h"
}


TEST(STORAGE_TESTS, CREATION_DELETION) {
    im_storage st = create_im_storage();
    ASSERT_NO_FATAL_FAILURE(delete_im_storage(&st));
}

TEST(STORAGE_TESTS, WRTIE) {
    im_storage st = create_im_storage();

    std::string data = "Why there is no 'max' function in C...";

    unsigned long ino = im_create(&st);
    ASSERT_NE(ino, -1);


    im_inode *node = (im_inode *) get(st.inodes, ino);
    ASSERT_EQ(im_write(node, data.c_str(), data.size(), 0), data.size());

    ASSERT_EQ(std::string(node->_data), data);

    delete_im_storage(&st);
}

TEST(STORAGE_TESTS, READ) {
    im_storage st = create_im_storage();

    std::string data = "Why there is no 'max' function in C...";

    unsigned long ino = im_create(&st);
    ASSERT_NE(ino, -1);
    

    im_inode *node = (im_inode *) get(st.inodes, ino);
    node->_data = (char *)malloc(sizeof(char) * data.size());
    node->_capacity = data.size();
    node->_stat.st_size = data.size();
    memcpy(node->_data, data.c_str(), data.size());

    char *buffer = (char *)malloc(sizeof(char) * data.size());

    ASSERT_EQ(im_read(node, buffer, data.size(), 0), data.size());

    ASSERT_EQ(std::string(buffer), data);

    delete_im_storage(&st);
    free(buffer);
}

TEST(STORAGE_TESTS, READ_WRTIE) {
    im_storage st = create_im_storage();

    std::string data = "Why there is no 'max' function in C...";
    char *buffer = (char *)malloc(sizeof(char) * data.size());

    unsigned long ino = im_create(&st);
    ASSERT_NE(ino, -1);

    im_inode *node = (im_inode *) get(st.inodes, ino);
    ASSERT_EQ(im_write(node, data.c_str(), data.size(), 0), data.size());
    ASSERT_EQ(im_read(node, buffer, data.size(), 0), data.size());

    ASSERT_EQ(std::string(buffer), data);

    delete_im_storage(&st);
}

TEST(STORAGE_TESTS, READ_WRTIE_OFFSET) {
    im_storage st = create_im_storage();

    std::string data = "Why there is no 'max' function in C...";
    char *buffer = (char *)malloc(sizeof(char) * data.size());

    unsigned long ino = im_create(&st);
    ASSERT_NE(ino, -1);
    
    im_inode *node = (im_inode *) get(st.inodes, ino);
    ASSERT_EQ(im_write(node, data.c_str(), data.size(), 0), data.size());
    ASSERT_EQ(im_write(node, data.c_str(), data.size(), 12), data.size());
    ASSERT_EQ(im_read(node, buffer, data.size(), 12), data.size());

    ASSERT_EQ(std::string(buffer), data);

    ASSERT_EQ(im_read(node, buffer, data.size(), 11), data.size());
    ASSERT_NE(std::string(buffer), data);

    delete_im_storage(&st);
}

TEST(STORAGE_TESTS, FAILURE_WRTIE_OFFSET) {
    im_storage st = create_im_storage();

    std::string data = "Why there is no 'max' function in C...";

    unsigned long ino = im_create(&st);
    ASSERT_NE(ino, -1);

    im_inode *node = (im_inode *) get(st.inodes, ino);
    ASSERT_EQ(im_write(node, data.c_str(), data.size(), 1024), -EOVERFLOW);

    delete_im_storage(&st);
}

TEST(STORAGE_TESTS, FAILURE_WRTIE_NULL) {
    im_storage st = create_im_storage();

    unsigned long ino = im_create(&st);
    ASSERT_NE(ino, -1);


    im_inode *node = (im_inode *) get(st.inodes, ino);
    ASSERT_EQ(im_write(node, NULL, 2048, 1024), -EOVERFLOW);
    
    delete_im_storage(&st);
}


TEST(STORAGE_TREE_TESTS, CREATE_TREE_TEST) {
    im_tree tree = im_tree_create();

    im_tree_node *node0 = (im_tree_node *) get(tree.root_node->entries, 0);
    im_tree_node *node1 = (im_tree_node *) get(tree.root_node->entries, 1);
    ASSERT_EQ(tree.root_node->entries_count, 2);
    ASSERT_EQ(node0, tree.root_node);
    ASSERT_EQ(node1, tree.root_node);
    ASSERT_EQ(tree.root_node->inode, 0);
    ASSERT_EQ(tree.root_node->dir, true);
    ASSERT_EQ(strcmp(tree.root_node->fname, ""), 0);
}

TEST(STORAGE_TREE_TESTS, ADD_NODE_TEST) {
    bool dir = false;
    size_t inode = 1;
    im_storage st = create_im_storage();

    im_tree_add_entry(&st, "/hello.txt", dir, inode);

    im_tree_node *node0 = (im_tree_node *) get(st._fstree.root_node->entries, 0);
    im_tree_node *node1 = (im_tree_node *) get(st._fstree.root_node->entries, 1);
    im_tree_node *node2 = (im_tree_node *) get(st._fstree.root_node->entries, 2);
    ASSERT_EQ(st._fstree.root_node->entries_count, 3);
    ASSERT_EQ(node2->inode, 1);
    ASSERT_EQ(node2->dir, false);
    ASSERT_TRUE(strcmp(node2->fname, "hello.txt") == 0);
   
    ASSERT_EQ(node0, st._fstree.root_node);
    ASSERT_EQ(node1, st._fstree.root_node);
    delete_im_storage(&st);
}

TEST(STORAGE_TREE_TESTS, ADD_MORE_NODES_TEST) { 
    im_storage st = create_im_storage();

    im_tree_add_entry(&st, "/home", true, 1);
    im_tree_add_entry(&st, "/usr", true, 2);
    im_tree_add_entry(&st, "/usr/hello.txt", false, 3);
    im_tree_add_entry(&st, "/hello2.txt", false, 4);
    

    im_tree_node *node2 = (im_tree_node *) get(st._fstree.root_node->entries, 2);
    im_tree_node *node3 = (im_tree_node *) get(st._fstree.root_node->entries, 3);
    im_tree_node *node4 = (im_tree_node *) get(st._fstree.root_node->entries, 4);

    im_tree_node *node30 = (im_tree_node *) get(node3->entries, 0);
    im_tree_node *node31 = (im_tree_node *) get(node3->entries, 1);
    im_tree_node *node32 = (im_tree_node *) get(node3->entries, 2);
    im_tree_node *node20 = (im_tree_node *) get(node2->entries, 0);
    im_tree_node *node21 = (im_tree_node *) get(node2->entries, 1);

    ASSERT_EQ(st._fstree.root_node->entries_count, 5);

    ASSERT_EQ(node2->dir, true);
    ASSERT_EQ(node2->inode, 1);
    ASSERT_TRUE(strcmp(node2->fname, "home") == 0);
    ASSERT_EQ(node2->entries_count, 2);
    ASSERT_EQ(node20, node2);
    ASSERT_EQ(node21, st._fstree.root_node);

    ASSERT_EQ(node3->dir, true);
    ASSERT_EQ(node3->inode, 2);
    ASSERT_TRUE(strcmp(node3->fname, "usr") == 0);
    ASSERT_EQ(node3->entries_count, 3);
    ASSERT_EQ(node30, node3);
    ASSERT_EQ(node31, st._fstree.root_node);

    ASSERT_EQ(node4->dir, false);
    ASSERT_EQ(node4->inode, 4);
    ASSERT_TRUE(strcmp(node4->fname, "hello2.txt") == 0);

    ASSERT_EQ(node3->entries_count, 3);
    ASSERT_EQ(node32->dir, false);
    ASSERT_EQ(node32->inode, 3);
    ASSERT_TRUE(strcmp(node32->fname, "hello.txt") == 0);

    delete_im_storage(&st);
}

TEST(LIST_TESTS, TEST1) {
    list l = create_list();

    const char *data = "Hello";
    const char *data2 = "Goodbye";
    push_back(l, (void *) data);
    push_back(l, (void *) data2);

    ASSERT_EQ(get(l, 0), data);
    ASSERT_EQ(get(l, 1), data2);
    ASSERT_EQ(get(l, 0), data);

    erase(l, 1);
    ASSERT_EQ(get(l, 0), data);
    ASSERT_EQ((char *) get(l, 1), nullptr);

    erase(l, 0);
    ASSERT_EQ((char *)get(l, 0), nullptr);

    push_back(l, (void *)data);
    ASSERT_EQ(get(l, 0), data);

    push_back(l, (void *) data2);
    ASSERT_EQ(get(l, 0), data);
    ASSERT_EQ(get(l, 1), data2);

    push_back(l, (void *) data);
    ASSERT_EQ(get(l, 2), data);

    erase(l, 1);
    ASSERT_EQ(get(l, 0), data);
    ASSERT_EQ(get(l, 1), data);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
