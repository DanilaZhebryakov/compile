#ifndef EXPR_ELEM_H_INCLUDED
#define EXPR_ELEM_H_INCLUDED
#include <stdio.h>
#include "expr_op.h"

enum exprKWType_t{
    EXPR_KW_NOTKW = 0,
    #define EXPR_KW_DEF(_enum, _enumval, _name) _enum = _enumval,
    #include "expr_kw_defines.h"
    #undef EXPR_KW_DEF
};
const char* getExprKWName(exprKWType_t kw);

enum exprDataType_t{
    EXPR_CONST = 1,
    EXPR_VAR   = 2,
    EXPR_KVAR  = 3,
    EXPR_OP    = 4,
    EXPR_FUNC  = 5,
    EXPR_CNTRL = 6,
    EXPR_PAIN  = 0xFF
};

#define OP_ONLY_CHARS "-+%*/\\^&|%!?~,;"
#define CNTRL_CHARS "(){}\"'#"
#define SPACE_CHARS " \n\t\r"

const double math_eps = 0.0001;

struct ExprElem{
    exprDataType_t type;
    union {double val; char* name; exprOpType_t op; exprKWType_t kword; char chr;};
};

#define BAD_EXPR_DATA {EXPR_PAIN}
const int MAX_FORM_WORD_LEN = 1000;

ExprElem scanExprElem (FILE* file, char c, char* buffer);

void printExprElem(FILE* file, ExprElem elem);


#endif
