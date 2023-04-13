#define SYS_UTILS_IMPL_INCLUDED

bool isPtrReadable(const void* ptr, size_t size){
    return ptr != nullptr;
}

bool isPtrWritable(void* ptr, size_t size){
    return ptr != nullptr;
}