#include <string.h>
#include <malloc.h>
#include "parseArg.h"

int parseArg(int argc, const char* argv[], const char* arg_to_find) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], arg_to_find) == 0){
            return i;
        }
    }
    return ARG_NOT_FOUND;
}
int parseArgBegin(int argc, const char* argv[], const char* arg_to_find) {
    int l = strlen(arg_to_find);
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], arg_to_find, l) == 0){
            return i;
        }
    }
    return ARG_NOT_FOUND;
}

char* repl_file_extension(const char* filename, const char* extension) {
    int inp_len = strlen(filename);
    int ext_pos = inp_len;
    for (int i = inp_len; i >= 0; i--){
        if (filename[i] == '.') {
            ext_pos = i;
        }
    }
    char* res = (char*)calloc(ext_pos + strlen(extension), sizeof(char));
    strcpy(res + ext_pos, extension);
    return res;
}
