#include "program_structure.h"

#define LOG_VARS


void programPosDataCtor(ProgramPosData* data){
    data->lvl = 0;
    data->flvl = 0;
    data->lbl_id = 1;
    data->stack_size = 0;
    data->rbp_offset = 0;
    data->code_block_id = -1;
    ustackCtor(&(data->add_mem), sizeof(void*));
}

void programAddNewMem(ProgramPosData* data, void* mem){
    if (ustackPush(&(data->add_mem), &mem) != VAR_NOERROR){
        Error_log("%s", "Bad stack push\n");
        ustackDump(&(data->add_mem));
    }
}
void programPosDataDtor(ProgramPosData* data){
    for (void** i = (void**)data->add_mem.data; i < (void**)data->add_mem.data + data->add_mem.size; i++){
        free(*i);
    }
    ustackDtor(&(data->add_mem));
}
