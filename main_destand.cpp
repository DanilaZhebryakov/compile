#include <stdio.h>
#include <stdlib.h>
#include "destandartify.h"

int main(){
    FILE* input_file = fopen("input.awp", "r");
    BinTreeNode* prog_st = readTreeFromFile_st(input_file);
    fclose(input_file);
    binTreeDump(prog_st);

    if(!prog_st)
        return 2;

    BinTreeNode* prog = destandartifyProgram(prog_st);
    binTreeDump(prog);
    if(!prog)
        return 3;



    FILE* tree_file = fopen("program_tree.txt", "w");
    binTreePrintToFile(prog, tree_file);
    fclose(tree_file);

    binTreeDtor(prog_st);
}
