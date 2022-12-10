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
    BinTreeNode* prog = scanProgram(input_file);
    fclose(input_file);
    binTreeDump(prog);

    FILE* tree_file = fopen("program_tree.txt", "w");
    binTreePrintToFile(prog, tree_file);
    fclose(tree_file);
    tree_file = fopen("program_tree.txt", "r");
    BinTreeNode* prog_n = binTreeReadFromFile(tree_file);
    fclose(tree_file);
    binTreeDump(prog_n);


    FILE* out_file = fopen("program.txt", "w");
    bool res = compileProgram(out_file, prog_n);
    fclose(out_file);
    binTreeDtor(prog);
    binTreeDtor(prog_n);
    printf_log("\n%d\n", res);
    system("run_code.bat");
}
