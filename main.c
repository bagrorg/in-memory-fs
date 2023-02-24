#include "src/storage/in_memory_storage.h"

int main() {
    im_storage st = create_im_storage();
    delete_im_storage(&st);
    return 0;
}