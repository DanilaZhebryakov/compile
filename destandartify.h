#ifndef DESTANDARTIFY_H_INCLUDED
#define DESTANDARTIFY_H_INCLUDED

#include <stdio.h>
#include "lib/bintree.h"

BinTreeNode* readTreeFromFile_st(FILE* file);

BinTreeNode* destandartifyProgram(BinTreeNode* expr);

#endif // DESTANDARTIFY_H_INCLUDED
