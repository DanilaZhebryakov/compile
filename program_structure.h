#ifndef PROGRAM_STRUCTURE_H_INCLUDED
#define PROGRAM_STRUCTURE_H_INCLUDED

#include "var_table.h"
#include "lib/bintree.h"

struct VarFuncData{
    int addr;
    bool write;
};

INC_VAR_TABLE(Var  , var  , int          );
INC_VAR_TABLE(Const, const, BinTreeNode* );
INC_VAR_TABLE(VFunc, vfunc, VarFuncData  );

int varTableGetNewAddr(VarTable* stk);
VarEntry* varTableCreate(VarTable* stk, char* s_name, int lvl);


struct ProgramNameTable{
    VarTable* vars;
    ConstTable* consts;
    VarTable* funcs;
    VFuncTable* vfuncs;
};

VFuncEntry* vfuncTableGetRW(VFuncTable* stk, const char* name, bool write);

void programNameTableCtor(ProgramNameTable* table);

void programNameTableDtor(ProgramNameTable* table);

void programDescendLvl(ProgramNameTable* objs, int lvl);

struct ProgramPosData{
    int lvl;
    int lbl_id;
    int stack_size;
    int rbp_offset;
    int code_block_id;
};

void programPosDataCtor(ProgramPosData* data);

#endif // PROGRAM_STRUCTURE_H_INCLUDED
