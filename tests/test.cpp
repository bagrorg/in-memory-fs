#include <cstddef>
#include <gtest/gtest-death-test.h>
#include <gtest/gtest.h>
#include <string>

extern "C" {
    #include "../src/storage/in_memory_storage.h"
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
    ASSERT_EQ(im_write(&st._inodes[ino], data.c_str(), data.size(), 0), 0);

    ASSERT_EQ(std::string(st._inodes[ino]._data), data);

    delete_im_storage(&st);
}

TEST(STORAGE_TESTS, READ) {
    im_storage st = create_im_storage();

    std::string data = "Why there is no 'max' function in C...";

    unsigned long ino = im_create(&st);
    ASSERT_NE(ino, -1);

    st._inodes[ino]._data = (char *)malloc(sizeof(char) * data.size());
    st._inodes[ino]._capacity = data.size();
    st._inodes[ino]._stat.st_size = data.size();
    memcpy(st._inodes[ino]._data, data.c_str(), data.size());

    char *buffer = (char *)malloc(sizeof(char) * data.size());

    ASSERT_EQ(im_read(&st._inodes[ino], buffer, data.size(), 0), 0);

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
    ASSERT_EQ(im_write(&st._inodes[ino], data.c_str(), data.size(), 0), 0);
    ASSERT_EQ(im_read(&st._inodes[ino], buffer, data.size(), 0), 0);

    ASSERT_EQ(std::string(buffer), data);

    delete_im_storage(&st);
}

TEST(STORAGE_TESTS, READ_WRTIE_OFFSET) {
    im_storage st = create_im_storage();

    std::string data = "Why there is no 'max' function in C...";
    char *buffer = (char *)malloc(sizeof(char) * data.size());

    unsigned long ino = im_create(&st);
    ASSERT_NE(ino, -1);
    ASSERT_EQ(im_write(&st._inodes[ino], data.c_str(), data.size(), 0), 0);
    ASSERT_EQ(im_write(&st._inodes[ino], data.c_str(), data.size(), 12), 0);
    ASSERT_EQ(im_read(&st._inodes[ino], buffer, data.size(), 12), 0);

    ASSERT_EQ(std::string(buffer), data);

    ASSERT_EQ(im_read(&st._inodes[ino], buffer, data.size(), 11), 0);
    ASSERT_NE(std::string(buffer), data);

    delete_im_storage(&st);
}

TEST(STORAGE_TESTS, FAILURE_WRTIE_OFFSET) {
    im_storage st = create_im_storage();

    std::string data = "Why there is no 'max' function in C...";

    unsigned long ino = im_create(&st);
    ASSERT_NE(ino, -1);
    ASSERT_EQ(im_write(&st._inodes[ino], data.c_str(), data.size(), 1024), -1);

    delete_im_storage(&st);
}

TEST(STORAGE_TESTS, FAILURE_WRTIE_NULL) {
    im_storage st = create_im_storage();

    unsigned long ino = im_create(&st);
    ASSERT_NE(ino, -1);
    ASSERT_EQ(im_write(&st._inodes[ino], NULL, 2048, 1024), -1);
    
    delete_im_storage(&st);
}


TEST(STORAGE_TREE_TESTS, CREATE_TREE_TEST) {
    im_tree tree = im_tree_create();

    ASSERT_EQ(tree.root_node.entries_count, 0);
    ASSERT_EQ(tree.root_node.entries, nullptr);
    ASSERT_EQ(tree.root_node.inode, 0);
    ASSERT_EQ(tree.root_node.dir, true);
    ASSERT_EQ(strcmp(tree.root_node.fname, ""), 0);
}

TEST(STORAGE_TREE_TESTS, ADD_NODE_TEST) {
    im_tree_node node = {
        .dir = false,
        .inode = 1,
        .fname = "hello.txt",
        .entries = NULL,
        .entries_count = 0,
    };

    im_storage st = create_im_storage();

    im_tree_add_entry(&st, "/hello.txt", &node);

    ASSERT_EQ(st._fstree.root_node.entries_count, 1);
    ASSERT_EQ(st._fstree.root_node.entries[0], &node);

    delete_im_storage(&st);
}

TEST(STORAGE_TREE_TESTS, ADD_MORE_NODES_TEST) {
    im_tree_node dir_node1 = {
        .dir = true,
        .inode = 1,
        .fname = "home",
        .entries = NULL,
        .entries_count = 0
    };

    im_tree_node dir_node2 = {
        .dir = true,
        .inode = 2,
        .fname = "usr",
        .entries = NULL,
        .entries_count = 0,
    };


    im_tree_node reg_node1 = {
        .dir = false,
        .inode = 3,
        .fname = "/usr/hello.txt",
        .entries = NULL,
        .entries_count = 0,
    };

    im_tree_node reg_node2 = {
        .dir = false,
        .inode = 4,
        .fname = "/hello2.txt",
        .entries = NULL,
        .entries_count = 0,
    };
    
    im_storage st = create_im_storage();

    im_tree_add_entry(&st, "/home", &dir_node1);
    im_tree_add_entry(&st, "/usr", &dir_node2);
    im_tree_add_entry(&st, "/usr/hello.txt", &reg_node1);
    im_tree_add_entry(&st, "/hello2.txt", &reg_node2);


    ASSERT_EQ(st._fstree.root_node.entries_count, 3);
    ASSERT_EQ(st._fstree.root_node.entries[0], &dir_node1);
    ASSERT_EQ(st._fstree.root_node.entries[1], &dir_node2);
    ASSERT_EQ(st._fstree.root_node.entries[2], &reg_node2);
    ASSERT_EQ(st._fstree.root_node.entries[1]->entries_count, 1);
    ASSERT_EQ(st._fstree.root_node.entries[1]->entries[0], &reg_node1);



    delete_im_storage(&st);

}


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
