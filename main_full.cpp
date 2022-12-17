#include <stdio.h>
#include <stdlib.h>
#include "syntax_analyser.h"
#include "compile_midend.h"
#include "compile_backend.h"
#include "expr/formule_utils.h"

int main()
{

    FILE* input_file = fopen("input.txt", "r");
    fscanf(input_file, "%*[^#]");
    fgetc(input_file);
    BinTreeNode* prog = scanProgram(input_file);

    fclose(input_file);
    header_log("Read");
    binTreeDump(prog);
    if(!prog)
        return 2;

    header_log("Mid");
    BinTreeNode* prog_mid = processProgram(prog);
    binTreeDump(prog_mid);
    if(!prog_mid)
        return 3;

    FILE* out_file = fopen("program.txt", "w");
    bool res = compileProgram(out_file, prog_mid);
    fclose(out_file);
    binTreeDtor(prog);
    if(res)
        system("run_code.bat");
    return res ? 0:1;
}
