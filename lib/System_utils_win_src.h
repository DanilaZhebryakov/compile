#include <windows.h>
#define SYS_UTILS_IMPL_INCLUDED

bool isPtrReadable(const void* ptr, size_t size){
    if (!ptr){
        return 0;
    }
    return !IsBadReadPtr(ptr, size);
}

bool isPtrWritable(void* ptr, size_t size){
    if (!ptr){
        return 0;
    }
    return !IsBadWritePtr(ptr, size);
}
