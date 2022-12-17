#include <stdio.h>
#include <stdlib.h>
#include "compile_backend.h"
#include "expr/formule_utils.h"

int main(){
    FILE* tree_file = fopen("program_tree.txt", "r");
    BinTreeNode* prog = binTreeReadFromFile(tree_file);
    fclose(tree_file);
    simplifyMathForm(&prog);
    binTreeDump(prog);

    if(!prog)
        return 2;

    FILE* out_file = fopen("program.txt", "w");
    bool res = compileProgram(out_file, prog);
    fclose(out_file);
    binTreeDtor(prog);
    return res ? 0:1;
}
