
#include "program_tables_v1.h"

DEF_VAR_TABLE(Var  , var  , int          );
DEF_VAR_TABLE(Func , func , FuncData     );
DEF_VAR_TABLE(VFunc, vfunc, VarFuncData  );

int varTableGetNewAddr(VarTable* stk){
    if (stk->size == 0){
        return 1;
    }
    return varTableGetLast(stk)->value + 1;
}

void programNameTableCtor(ProgramNameTable* table){
    table->vars   = (VarTable*)malloc(sizeof(*table->vars));
    table->funcs  = (VarTable*)malloc(sizeof(*table->funcs));
    table->vfuncs = (VFuncTable*)malloc(sizeof(*table->vfuncs));

    varTableCtor(table->vars);
    varTableCtor(table->funcs);
    vfuncTableCtor(table->vfuncs);
}

void programNameTableDtor(ProgramNameTable* table){
    varTableDtor(table->vars);
    varTableDtor(table->funcs);
    vfuncTableDtor(table->vfuncs);

    free(table->vars);
    free(table->funcs);
    free(table->vfuncs);
}

void programDescendLvl(ProgramNameTable* objs, ProgramPosData* pos, int lvl){
    pos->rbp_offset -= varTableDescendLvl  (objs->vars   , lvl);
    varTableDescendLvl  (objs->funcs  , lvl);
    vfuncTableDescendLvl(objs->vfuncs , lvl);
}


VFuncEntry* vfuncTableGetRW(VFuncTable* stk, const char* name, bool write){
    for (VFuncEntry* i = (VFuncEntry*)stk->data; i < ((VFuncEntry*)stk->data) + stk->size; i++){
        if (strcmp(i->name, name) == 0 && i->value.write == write){
            return i;
        }
    }
    return nullptr;
}

void programCreateVar(ProgramNameTable* objs, ProgramPosData* pos, const char* name, int len){
    int offs = pos->rbp_offset;
    varTablePut(objs->vars, {offs, name, pos->lvl, pos->flvl});
    #ifdef LOG_VARS
    printf_log("Var:%s Level:%d Flevel:%d Addr:%d\n", name, pos->lvl, pos->flvl, pos->rbp_offset);
    #endif
    pos->rbp_offset += len;
}