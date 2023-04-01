#include <string.h>
#ifdef WINDOWS
    #include <windows.h>
#else
    #include <unistd.h>
#endif

bool isPtrReadable(const void* ptr, size_t size){
    if (!ptr){
        return 0;
    }
    #ifdef WINDOWS
        return !IsBadReadPtr(ptr, size);
    #else
        while (size > 0){
            if (write(1, ptr, 0) == -1)
                return 0;
            ptr = (const void*) ((const char*)ptr + 1);
            size--;
        }
        return 1;
    #endif
}

bool isPtrWritable(void* ptr, size_t size){
    if (!ptr){
        return 0;
    }
    #ifdef WINDOWS
        return !IsBadWritePtr(ptr, size);
    #else
        while (size > 0){
            if (write(1, ptr, 0) == -1)
                return 0;
            ptr = (void*) ((char*)ptr + 1);
            size--;
        }
        return 1;
    #endif
}
