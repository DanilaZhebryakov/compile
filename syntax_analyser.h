#ifndef SYNTAX_ANALYSER_H_INCLUDED
#define SYNTAX_ANALYSER_H_INCLUDED

#include "lib/bintree.h"
#include "expr/expr_elem.h"

BinTreeNode* scanProgram(FILE* file);

#endif // SYNTAX_ANALYSER_H_INCLUDED
