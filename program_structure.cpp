#include "program_structure.h"

DEF_VAR_TABLE(Var  , var  , int          );
DEF_VAR_TABLE(Const, const, BinTreeNode* );
DEF_VAR_TABLE(Func , func , FuncData     );
DEF_VAR_TABLE(VFunc, vfunc, VarFuncData  );

int varTableGetNewAddr(VarTable* stk){
    if(stk->size == 0){
        return 1;
    }
    return varTableGetLast(stk)->value + 1;
}

VarEntry* varTableCreate(VarTable* stk, char* s_name, int lvl){
    varTablePut(stk, {varTableGetNewAddr(stk), s_name, lvl});
    return varTableGetLast(stk);
}

void programNameTableCtor(ProgramNameTable* table){
    table->vars   = (VarTable*)malloc(sizeof(*table->vars));
    table->consts = (ConstTable*)malloc(sizeof(*table->consts));
    table->funcs  = (VarTable*)malloc(sizeof(*table->funcs));
    table->vfuncs = (VFuncTable*)malloc(sizeof(*table->vfuncs));

    varTableCtor(table->vars);
    constTableCtor(table->consts);
    varTableCtor(table->funcs);
    vfuncTableCtor(table->vfuncs);
}

void programNameTableDtor(ProgramNameTable* table){
    varTableDtor(table->vars);
    constTableDtor(table->consts);
    varTableDtor(table->funcs);
    vfuncTableDtor(table->vfuncs);

    free(table->vars);
    free(table->consts);
    free(table->funcs);
    free(table->vfuncs);
}

void programDescendLvl(ProgramNameTable* objs, int lvl){
    varTableDescendLvl  (objs->vars   , lvl);
    constTableDescendLvl(objs->consts , lvl);
    varTableDescendLvl  (objs->funcs  , lvl);
    vfuncTableDescendLvl(objs->vfuncs , lvl);
}

VFuncEntry* vfuncTableGetRW(VFuncTable* stk, const char* name, bool write){
    for (VFuncEntry* i = (VFuncEntry*)stk->data; i < ((VFuncEntry*)stk->data) + stk->size; i++){
        if(strcmp(i->name, name) == 0 && i->value.write == write){
            return i;
        }
    }
    return nullptr;
}

void programPosDataCtor(ProgramPosData* data){
    data->lvl = 0;
    data->lbl_id = 1;
    data->stack_size = 0;
    data->rbp_offset = 0;
    data->code_block_id = -1;
    ustackCtor(&(data->add_mem), sizeof(void*));
}
void programAddNewMem(ProgramPosData* data, void* mem){
    if(ustackPush(&(data->add_mem), &mem) != VAR_NOERROR){
        Error_log("%s", "Bad stack push\n");
        ustackDump(&(data->add_mem));
    }
}
void programPosDataDtor(ProgramPosData* data){
    for(void** i = (void**)data->add_mem.data; i < data->add_mem.data + data->add_mem.size; i++){
        free(*i);
    }
    ustackDtor(&(data->add_mem));
}
