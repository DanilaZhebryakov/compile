#include <ctype.h>
#include <math.h>

#include "expr_elem.h"
#include "lib/bintree.h"

const char* getExprStandName(exprStandartThing_t stand){
    #define EXPR_ST_DEF(_enum, _enumval, _std) \
    if(_enum == stand){    \
        return _std;                           \
    }
    #include "expr/expr_st_defines.h"
    #undef EXPR_ST_DEF
    return "BADSTD";
}

const char* getExprKWName(exprKWType_t kw){
    #define EXPR_KW_DEF(_enum, _enumval, _name, _std) \
    if(_enum == kw){    \
        return _name;                           \
    }
    #include "expr/expr_kw_defines.h"
    #undef EXPR_KW_DEF
    return "BADKW";
}

const char* getExprStdKWName(exprKWType_t kw){
    #define EXPR_KW_DEF(_enum, _enumval, _name, _std) \
    if(_enum == kw){    \
        return _std;                           \
    }
    #include "expr/expr_kw_defines.h"
    #undef EXPR_KW_DEF
    return "BADKW";
}

ExprElem scanExprElem (FILE* file, char c, char* buffer){
    ExprElem ret = {};
    if (isdigit(c)){
        ret.type = EXPR_CONST;
        fscanf(file, "%lg", &(ret.val));
        return ret;
    }
    if (c == '"'){

        fscanf(file, "\"%[^\"]\"", buffer);
        ret.type = EXPR_STLIT;
        ret.name = strdup(buffer);
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
    #define EXPR_KW_DEF(_enum, _enumval, _name, _std) \
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
        fputs(exprOpName(elem.op), file);
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
    case EXPR_STAND:
        fprintf(file, "%s", getExprStandName(elem.stand));
        break;
    case EXPR_STLIT:
        fprintf(file, "\"%s\"", elem.name);
        break;
    default:
        fprintf(file, "TRASH");
        break;
    }
}
