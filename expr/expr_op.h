#ifndef EXPR_OP_H_INCLUDED
#define EXPR_OP_H_INCLUDED


#define EXPR_OP_DEF(_enum, _enumval, _name, _pri, _ret) \
_enum = _enumval,

enum exprOpType_t{
    EXPR_O_UNARY = 0x80,
    EXPR_O_MATH  = 0x40,
    EXPR_O_NOTOP = 0,
    #include "expr_op_defines.h"
};
#undef EXPR_OP_DEF

const int MAX_EXPR_OP_PRIORITY = 5;

bool isExprOpUnary(exprOpType_t op);

bool isMathOp(exprOpType_t op);

bool isAssignOp(exprOpType_t op);

bool canExprOpBeUnary(exprOpType_t op);

bool canExprOpBeUnaryL(exprOpType_t op);

int getExprOpPriority(exprOpType_t op_type);

const char* exprOpName(exprOpType_t op_type);

exprOpType_t scanExprOp(const char* buffer);

double calcMathOp(exprOpType_t op_type, double a, double b);

#endif // EXPR_OP_H_INCLUDED
