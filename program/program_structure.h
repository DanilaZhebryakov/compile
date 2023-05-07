#ifndef PROGRAM_STRUCTURE_H_INCLUDED
#define PROGRAM_STRUCTURE_H_INCLUDED

#include "lib/UStack.h"
#define LOG_VARS


struct ProgramPosData{
    int lvl;
    int flvl;
    size_t stack_size;
    size_t code_block_id;
    size_t rbp_offset;
    size_t lbl_id;
    UStack add_mem;
};

void programPosDataCtor(ProgramPosData* data);
void programAddNewMem(ProgramPosData* data, void* mem);
void programPosDataDtor(ProgramPosData* data);

#endif // PROGRAM_STRUCTURE_H_INCLUDED
