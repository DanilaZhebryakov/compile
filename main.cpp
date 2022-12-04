#include <stdio.h>
#include <stdlib.h>
#include "syntax_analyser.h"
#include "compile_backend.h"

int main()
{

    FILE* input_file = fopen("input.txt", "r");
    fscanf(input_file, "%*[^#]");
    fgetc(input_file);
    BinTreeNode* prog = scanProgram(input_file);
    binTreeDump(prog);
    bool res = compileProgram(stdout, prog);
    printf_log("\n%d\n", res);
}
