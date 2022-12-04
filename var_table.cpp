#include <stdlib.h>
#include "var_table.h"

void varTableCtor(VarTable* stk){
    ustackCtor(stk, sizeof(VarEntry));
}

bool varTablePut(VarTable* stk, VarEntry var){
    return ustackPush(stk, &var) == VAR_NOERROR;
}

VarEntry* varTableGet(VarTable* stk, char* s_name){
    for (VarEntry* i = (VarEntry*)stk->data; i < ((VarEntry*)stk->data) + stk->size; i++){
        if(strcmp(i->name, s_name) == 0){
            return i;
        }
    }
    return nullptr;
}

VarEntry* varTableGetLast(VarTable* stk){
    return ((VarEntry*)stk->data) + stk->size - 1;
}

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


void varTableDescendLvl(VarTable* stk, int lvl){
    while(stk->size > 0 && (((VarEntry*)stk->data) + stk->size - 1)->depth >= lvl && (ustackPop(stk, nullptr) == VAR_NOERROR)) {}
}
