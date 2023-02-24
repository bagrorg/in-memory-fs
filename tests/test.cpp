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

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
