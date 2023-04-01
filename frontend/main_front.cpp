#include <stdio.h>
#include <stdlib.h>
#include "syntax_analyser.h"
//#include "standartify.h"

int main(){
    FILE* input_file = fopen("input.txt", "r");
    fscanf(input_file, "%*[^#]");
    fgetc(input_file);
    BinTreeNode* prog = scanProgram(input_file);
    fclose(input_file);
    binTreeDump(prog);

    if(!prog)
        return 2;

    FILE* tree_file = fopen("program_tree.tree", "w");
    binTreePrintToFile(prog, tree_file);
    fclose(tree_file);

    /*
    FILE* st_tree_file = fopen("st_program_tree.awp", "w");
    BinTreeNode* stand = standartifyProgram(prog);
    binTreeDump(stand);
    writeProgramToFile_st(stand, st_tree_file);
    binTreeDtor(stand);
    fclose(st_tree_file);
    */

    binTreeDtor(prog);
}
