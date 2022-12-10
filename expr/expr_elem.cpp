#include <ctype.h>
#include <math.h>

#include "expr_elem.h"
#include "lib/bintree.h"

const char* getExprKWName(exprKWType_t kw){
    #define EXPR_KW_DEF(_enum, _enumval, _name) \
    if(_enum == kw){    \
        return _name;                           \
    }
    #include "expr/expr_kw_defines.h"
    #undef EXPR_KW_DEF
    return "BADKW";
}


ExprElem scanExprElem (FILE* file, char c, char* buffer){
    ExprElem ret = {};
    if (isdigit(c)){
        ret.type = EXPR_CONST;
        char c1 = fgetc(file);
        ungetc(c1, file);
        fscanf(file, "%lg", &(ret.val));
        return ret;
    }

    bool op_char = strchr(OP_ONLY_CHARS, c) != nullptr;
    if (op_char)
        fscanf(file, "%[" OP_ONLY_CHARS "]", buffer);
    else
        fscanf(file, "%[^" OP_ONLY_CHARS CNTRL_CHARS SPACE_CHARS"]", buffer);

    if (*buffer == '\0'){
        ret.type = EXPR_PAIN;
        return ret;
    }

    exprOpType_t op_type = scanExprOp(buffer);
    if (op_type != EXPR_O_NOTOP){
        ret.type = EXPR_OP;
        ret.op   = op_type;
        return ret;
    }
    if (op_char){
        ret.type = EXPR_PAIN;
        return ret;
    }
    exprKWType_t kw = EXPR_KW_NOTKW;
    #define EXPR_KW_DEF(_enum, _enumval, _name) \
    if(strcmp(buffer, _name) == 0){    \
        kw = _enum;                             \
    }
    #include "expr/expr_kw_defines.h"
    #undef EXPR_KW_DEF

    if(kw != EXPR_KW_NOTKW){
        ret.type = EXPR_KVAR;
        ret.kword = kw;
        return ret;
    }

    ret.type = EXPR_VAR;
    ret.name = strdup(buffer);
    return ret;
}

void printExprElem(FILE* file, ExprElem elem){
    switch (elem.type){
    case EXPR_PAIN:
        fprintf(file, "PAIN");
        break;
    case EXPR_OP:
        fprintf(file, exprOpName(elem.op));
        break;
    case EXPR_CONST:
        fprintf(file, "%lg", elem.val);
        break;
    case EXPR_VAR:
        fprintf(file, "%s", elem.name);
        break;
    case EXPR_FUNC:
        fprintf(file, "%s", elem.name);
        break;
    case EXPR_CNTRL:
        fprintf(file, "$%c", elem.chr);
        break;
    case EXPR_KVAR:
        fprintf(file, "%s", getExprKWName(elem.kword));
        break;
    default:
        fprintf(file, "TRASH");
        break;
    }
}
