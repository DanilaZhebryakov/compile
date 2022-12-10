#include <stdio.h>
#include <stdlib.h>
#include "syntax_analyser.h"

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
    binTreeDtor(prog);
}
