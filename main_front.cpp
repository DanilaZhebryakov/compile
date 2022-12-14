#include <stdio.h>
#include <stdlib.h>
#include "syntax_analyser.h"
#include "standartify.h"

int main(){
    FILE* input_file = fopen("input.txt", "r");
    fscanf(input_file, "%*[^#]");
    fgetc(input_file);
    BinTreeNode* prog = scanProgram(input_file);
    fclose(input_file);
    binTreeDump(prog);

    if(!prog)
        return 2;

    FILE* tree_file = fopen("program_tree.txt", "w");
    binTreePrintToFile(prog, tree_file);
    fclose(tree_file);

    FILE* st_tree_file = fopen("st_program_tree.txt", "w");
    BinTreeNode* stand = standartifyProgram(prog, 0);
    binTreeDump(stand);
    writeProgramToFile_st(stand, st_tree_file);
    binTreeDtor(stand);
    fclose(st_tree_file);

    binTreeDtor(prog);
}
