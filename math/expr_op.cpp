#include <math.h>
#include <string.h>

#include "expr_op.h"

struct MathOp{
    const char* name;
    exprOpType_t op;
    int priorirty;
};

#define EXPR_OP_DEF(_enum, _enumval, _name, _pri, _ret) \
{_name, _enum, _pri},

static const MathOp oplist[] = {
    #include "expr_op_defines.h"
};
#undef EXPR_OP_DEF

bool isExprOpUnary(exprOpType_t op){
    return op & EXPR_O_UNARY;
}
bool isMathOp(exprOpType_t op){
    return(op & EXPR_O_MATH);
}
bool isAssignOp(exprOpType_t op){
    return(op == EXPR_O_EQRTL || op == EXPR_O_EQLTR);
}

static const int opcount = sizeof(oplist) / sizeof(MathOp);

bool canExprOpBeUnary(exprOpType_t op){
    for (int i = 0; i < opcount; i++){
        if (oplist[i].op == (op | EXPR_O_UNARY)){
            return true;
        }
    }
    return false;
}

bool canExprOpBeUnaryL(exprOpType_t op){
    return op == EXPR_O_ENDL;
}

int getExprOpPriority(exprOpType_t op_type){
    for (int i = 0; i < opcount; i++){
        if (oplist[i].op == op_type){
            return oplist[i].priorirty;
        }
    }
    return -1;
}

const char* mathOpName(exprOpType_t op_type){
    for (int i = 0; i < opcount; i++){
        if (oplist[i].op == op_type){
            return oplist[i].name;
        }
    }
    return "BADOP";
}

#define EXPR_OP_DEF(_enum, _enumval, _name, _pri, _ret) \
case _enum:\
    return _ret;

double calcMathOp(exprOpType_t op_type, double a, double b){
    switch (op_type){
    #include "expr_op_defines.h"
    default:
        return NAN;
    }
}
#undef EXPR_OP_DEF


exprOpType_t scanExprOp(const char* buffer){
    for (int i = 0; i < opcount; i++){
        if (strcmp(oplist[i].name, buffer) == 0){
            return oplist[i].op;
        }
    }
    return EXPR_O_NOTOP;
}
