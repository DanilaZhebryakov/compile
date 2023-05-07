#include "var_table.h"
#include "lib/bintree.h"
#include "program_structure.h"

struct VarFuncData{
    char* lbl;
    bool write;
};

struct FuncData{
    char* lbl;
};

INC_VAR_TABLE(Var  , var  , int          );
INC_VAR_TABLE(Func , func , FuncData     );
INC_VAR_TABLE(VFunc, vfunc, VarFuncData  );

int varTableGetNewAddr(VarTable* stk);
VarEntry* varTableCreate(VarTable* stk, char* s_name, int lvl);


struct ProgramNameTable{
    VarTable* vars;
    FuncTable* funcs;
    VFuncTable* vfuncs;
};

void programNameTableCtor(ProgramNameTable* table);

void programNameTableDtor(ProgramNameTable* table);

VFuncEntry* vfuncTableGetRW(VFuncTable* stk, const char* name, bool write);

void programCreateVar(ProgramNameTable* objs, ProgramPosData* pos, const char* name, int len = 1);

void programDescendLvl(ProgramNameTable* objs, ProgramPosData* pos, int lvl);