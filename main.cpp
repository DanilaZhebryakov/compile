#include <stdio.h>
#include <stdlib.h>
#include "syntax_analyser.h"
#include "compile_midend.h"
#include "compile_backend.h"

int main()
{

    FILE* input_file = fopen("input.txt", "r");
    fscanf(input_file, "%*[^#]");
    fgetc(input_file);
    BinTreeNode* expr = scanProgram(input_file);
    fclose(input_file);
    BinTreeNode* prog = exprReplaceKW(expr);
    binTreeDtor(expr);

    binTreeDump(prog);
    FILE* out_file = fopen("program.txt", "w");
    bool res = compileProgram(out_file, prog);
    fclose(out_file);
    binTreeDtor(prog);
    printf_log("\n%d\n", res);
    system("run_code.bat");
}
