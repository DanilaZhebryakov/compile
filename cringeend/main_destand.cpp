#include <stdio.h>
#include <stdlib.h>
#include "destandartify.h"
#include "expr/formule_utils.h"

int main(){
    FILE* input_file = fopen("input.awp", "r");

    if(!input_file)
        return 4;

    BinTreeNode* prog_st = readTreeFromFile_st(input_file);
    fclose(input_file);
    binTreeDump(prog_st);

    if(!prog_st)
        return 2;

    BinTreeNode* prog = destandartifyProgram(prog_st);
    binTreeDump(prog);
    if(!prog)
        return 3;
    FILE* prog_out_file = fopen("prog_out.txt", "w");
    printMathForm(prog_out_file, prog);
    fclose(prog_out_file);

    FILE* tree_file = fopen("program_tree.txt", "w");
    binTreePrintToFile(prog, tree_file);
    fclose(tree_file);

    binTreeDtor(prog_st);
}
