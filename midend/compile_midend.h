#ifndef COMPILE_MIDEND_H_INCLUDED
#define COMPILE_MIDEND_H_INCLUDED

#include "program/var_table.h"
INC_VAR_TABLE(Const, const, BinTreeNode* )

BinTreeNode* processProgram(BinTreeNode* expr, int lvl);

#endif // COMPILE_MIDEND_H_INCLUDED
