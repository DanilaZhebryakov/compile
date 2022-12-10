#ifndef PROGRAM_STRUCTURE_H_INCLUDED
#define PROGRAM_STRUCTURE_H_INCLUDED

#include "var_table.h"
#include "lib/bintree.h"

struct VarFuncData{
    char* lbl;
    bool write;
};

struct FuncData{
    char* lbl;
};

INC_VAR_TABLE(Var  , var  , int          );
INC_VAR_TABLE(Const, const, BinTreeNode* );
INC_VAR_TABLE(Func , func , FuncData     );
INC_VAR_TABLE(VFunc, vfunc, VarFuncData  );

int varTableGetNewAddr(VarTable* stk);
VarEntry* varTableCreate(VarTable* stk, char* s_name, int lvl);


struct ProgramNameTable{
    VarTable* vars;
    ConstTable* consts;
    FuncTable* funcs;
    VFuncTable* vfuncs;
};

VFuncEntry* vfuncTableGetRW(VFuncTable* stk, const char* name, bool write);

void programNameTableCtor(ProgramNameTable* table);

void programNameTableDtor(ProgramNameTable* table);

struct ProgramPosData{
    int lvl;
    int flvl;
    int lbl_id;
    int stack_size;
    int rbp_offset;
    int code_block_id;
    UStack add_mem;
};

void programDescendLvl(ProgramNameTable* objs, ProgramPosData* pos, int lvl);

void programPosDataCtor(ProgramPosData* data);
void programAddNewMem(ProgramPosData* data, void* mem);
void programPosDataDtor(ProgramPosData* data);

#endif // PROGRAM_STRUCTURE_H_INCLUDED
