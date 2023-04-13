#include <unistd.h>
#define SYS_UTILS_IMPL_INCLUDED

bool isPtrReadable(const void* ptr, size_t size){
    if (!ptr){
        return 0;
    }
    while (size > 0){
        if (write(1, ptr, 0) == -1)
            return 0;
        ptr = (const void*) ((const char*)ptr + 1);
        size--;
    }
    return 1;
}

bool isPtrWritable(void* ptr, size_t size){
    if (!ptr){
        return 0;
    }
    while (size > 0){
        if (write(1, ptr, 0) == -1)
            return 0;
        ptr = (void*) ((char*)ptr + 1);
        size--;
    }
    return 1;
}