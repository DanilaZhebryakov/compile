#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include "math/expr_elem.h"
#include "lib/bintree.h"

static BinTreeNode* exprReplaceKW_(BinTreeNode* expr){
    BinTreeNode* newL = nullptr;
    BinTreeNode* newR = nullptr;
    if(expr->left)
        newL = exprReplaceKW_(expr->left);
    if(expr->right)
        newR = exprReplaceKW_(expr->right);
    exprKWType_t kw = EXPR_KW_NOTKW;
    if(expr->data.type == EXPR_VAR){
        #define EXPR_KW_DEF(_enum, _enumval, _name) \
        if(strcmp(expr->data.name, _name) == 0){    \
            kw = _enum;                             \
        }
        #include "math/expr_kw_defines.h"
        #undef EXPR_KW_DEF
    }
    if(kw == EXPR_KW_NOTKW && newL == expr->left && newR == expr->right)
        return expr;
    BinTreeNode* ret = binTreeNewNode(expr->data, 0);
    ret->left  = newL;
    ret->right = newR;
    if(newL)
        newL->usedc++;
    if(newR)
        newR->usedc++;
    if(kw != EXPR_KW_NOTKW){
        ret->data.type  = EXPR_KVAR;
        ret->data.kword = kw;
    }
    binTreeUpdSize(ret);
    return ret;
}

BinTreeNode* exprReplaceKW(BinTreeNode* expr){
    BinTreeNode* ret = exprReplaceKW_(expr);
    if(ret)
        ret->usedc++;
    return ret;
}
