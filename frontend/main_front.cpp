#include <stdio.h>
#include <stdlib.h>

#include "lib/parseArg.h"
#include "lib/cmd_tool_utils.h"

#include "syntax_analyser.h"
#include "misc_default_vals.h"
//#include "standartify.h"

const char* const help_str = 
"The ;⸵⁉ compiler front-end utility\n"
"Options:\n"
"-f <filename> [out filename] : specify a file. Multiple files can be specified\n"
"If no output filename is specified, it is the same as input, but with \"" LANG_TREE_PRE_FILE_EXTENSION "\" extension\n"
"If no files are specified, defaults to \"" LANG_SOURCE_DEFAULT_FILE "\" \n"
"-l[level] : set a processing level. (default: max level) This is used mainly for debugging\n"
"-S  : use stdin and stdout as in and out files \n"
DEFAULT_ARG_DESCRIPTION
;

int processFile(FILE* input_file, FILE* tree_file){
    fscanf(input_file, "%*[^#]");
    fgetc(input_file);
    BinTreeNode* prog = scanProgram(input_file);
    binTreeDump(prog);

    if(!prog){
        IF_NQ(fprintf(stderr, "Errors occured while scanning program\n");)
        return 2;
    }

    binTreePrintToFile(prog, tree_file);
    binTreeDtor(prog);
    return 0;
}

int processFile(const char* input_filename, const char* out_filename) {
    IF_VB(fprintf(stderr, "Input file: %s ", input_filename);)

    bool alloc_str = false;
    if (!out_filename){
        alloc_str = true;
        out_filename = repl_file_extension(input_filename, LANG_TREE_PRE_FILE_EXTENSION);
    }
    IF_VB(fprintf(stderr, "Output: %s\n", out_filename);)

    FILE* inp_file = fopen(input_filename, "r");
    FILE* out_file = inp_file ? fopen(out_filename, "w") : nullptr;

    int ret = -2;
    if (inp_file && out_file){
        ret = processFile(inp_file, out_file);
    }
    else{
        ret = 1;
    }
    if (inp_file){
        fclose(inp_file);
        if (out_file){
            fclose(out_file);
        }
        else{
            IF_NQ(fprintf(stderr, "Can not open output file (%s)\n", out_filename);)
        }
    }
    else{
        IF_NQ(fprintf(stderr, "Can not open input file (%s). Probably it does not exist\n", input_filename);)
    }
    
    if (alloc_str) {
        free((void*)out_filename);
    }
    return ret;
}

int main(int argc, const char* argv[]){
    HANDLE_DEFAULT_ARGS

    int file_counter = 0;

    if (parseArg(argc, argv, "-S") >= 0) {
        int ret = processFile(stdin, stdout);
        file_counter++;
        if(ret != 0){
            return ret;
        }
    }

    for (int arg_i = 0; arg_i < argc; arg_i++){
        if (strcmp(argv[arg_i], "-f") == 0) {
            if (arg_i == argc-1) {
                IF_NQ(fprintf(stderr, "-f: expected a filename\n");)
                return 1;
            }
            const char* inp_filename = argv[arg_i+1];
            const char* out_filename = nullptr;
            arg_i++;
            if (arg_i < argc-1 && argv[arg_i + 1][0] != '-') {
                out_filename = argv[arg_i + 1];
                arg_i++;
            }
            int ret = processFile(inp_filename, out_filename);
            file_counter++;
            if (ret != 0) {
                return ret;
            }
        }
    }
    if (file_counter == 0) {
        IF_NQ(fprintf(stderr, "Warning: no files specified. Using default. (run with --help for help)\n");)

        int ret = processFile(LANG_SOURCE_DEFAULT_FILE, LANG_TREE_PRE_DEFAULT_FILE);
        file_counter++;
            if (ret != 0) {
                return ret;
            }
    }    

    IF_VB(fprintf(stderr, "Succcessfully processed %d files\n", file_counter);)
    return 0;

    /*
    FILE* st_tree_file = fopen("st_program_tree.awp", "w");
    BinTreeNode* stand = standartifyProgram(prog);
    binTreeDump(stand);
    writeProgramToFile_st(stand, st_tree_file);
    binTreeDtor(stand);
    fclose(st_tree_file);
    */
}
