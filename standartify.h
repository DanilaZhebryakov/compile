#ifndef STANDARTIFY_H_INCLUDED
#define STANDARTIFY_H_INCLUDED

void writeElemToFile_st(ExprElem* elem, FILE* file);

BinTreeNode* standartifyProgram(BinTreeNode* expr);

void writeProgramToFile_st(BinTreeNode* expr, FILE* file);

#endif // STANDARTIFY_H_INCLUDED
