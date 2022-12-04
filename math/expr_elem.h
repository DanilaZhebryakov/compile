#ifndef EXPR_ELEM_H_INCLUDED
#define EXPR_ELEM_H_INCLUDED
#include <stdio.h>
#include "expr_op.h"

enum exprDataType_t{
    EXPR_CONST = 1,
    EXPR_VAR   = 2,
    EXPR_OP    = 3,
    EXPR_FUNC  = 4,
    EXPR_CNTRL = 5,
    EXPR_PAIN  = 0xFF
};

#define OP_ONLY_CHARS "-+%*/\\^&|%!?~,;"
#define CNTRL_CHARS "(){}"
#define SPACE_CHARS " \n\t\r"

const double math_eps = 0.0001;

struct ExprElem{
    exprDataType_t type;
    union {double val; char* name; exprOpType_t op; char chr;};
};

#define BAD_EXPR_DATA {EXPR_PAIN}
const int MAX_FORM_WORD_LEN = 1000;

ExprElem scanExprElem (FILE* file, char c, char* buffer);

void printExprElem(FILE* file, ExprElem elem);


#endif
