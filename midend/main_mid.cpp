#include <stdio.h>
#include <stdlib.h>
#include "expr/formule_utils.h"
#include "compile_midend.h"

int main(){
    FILE* tree_file = fopen("program_tree.tree", "r");
    BinTreeNode* prog = binTreeReadFromFile(tree_file);

    if(!prog)
        return 2;

    fclose(tree_file);
    binTreeDump(prog);

    header_log("Mid");
    BinTreeNode* prog_mid = processProgram(prog);
    binTreeDump(prog_mid);
    if(!prog_mid)
        return 3;
    FILE* out_tree_file = fopen("program_tree.three", "w");
    binTreePrintToFile(prog_mid, out_tree_file);
    fclose(out_tree_file);
}