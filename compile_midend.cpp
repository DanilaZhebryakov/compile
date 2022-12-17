#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include "expr/expr_elem.h"
#include "expr/formule_utils.h"
#include "lib/bintree.h"
#include "compile_midend.h"
#include "lib/debug_utils.h"

DEF_VAR_TABLE(Const, const, BinTreeNode* );

static BinTreeNode* processCode(BinTreeNode* node, ConstTable* consts, int lvl);

static BinTreeNode* processCodeBlock(BinTreeNode* node, ConstTable* consts, int lvl){
    if (!node){
        return nullptr;
    }
    if(node->data.type == EXPR_OP && node->data.op == EXPR_O_ENDL){
        BinTreeNode* ret = processCode(node, consts, lvl+1);
        while(consts->size > 0 && (((ConstEntry*)consts->data) + consts->size - 1)->depth >= lvl) {
            binTreeDtor(constTableGetLast(consts)->value); //dtor needed, so just descendlvl will not work

            assert_log(ustackPop(consts, nullptr) == VAR_NOERROR);
        }

        return ret;
    }
    return processCode(node, consts, lvl);
}


static BinTreeNode* processCode(BinTreeNode* node, ConstTable* consts, int lvl){
    if (!node){
        return nullptr;
    }
    if (node->data.type == EXPR_VAR){
        ConstEntry* ce = constTableGet(consts, node->data.name);
        if (ce){
            ce->value->usedc++;
            return ce->value;
        }
        node->usedc++;
        return node;
    }
    if (node->data.type != EXPR_OP){
        node->usedc++;
        return node;
    }
    if (node->data.op == EXPR_O_CDEF){
        if (!(node->left) || node->left->data.type != EXPR_VAR){
            error_log("bad constant definition\n");
            return nullptr;
        }
        BinTreeNode* const_node = processCodeBlock(node->right, consts, lvl);
        constTablePut(consts, {const_node, node->left->data.name, lvl, lvl});
        return nullptr;
    }
    if (node->data.op == EXPR_O_ENDL){
        BinTreeNode* left_node  = processCode(node->left, consts, lvl);
        BinTreeNode* right_node = processCodeBlock(node->right, consts, lvl);
        if (left_node){
            BinTreeNode* new_node = binTreeNewNode(node->data);
            new_node->left = left_node;
            new_node->right = right_node;
            binTreeUpdSize(new_node);
            return new_node;
        }
        else{
            return right_node;
        }
    }

    BinTreeNode* new_node = binTreeNewNode(node->data);
    new_node->left  = processCodeBlock(node->left , consts, lvl);
    new_node->right = processCodeBlock(node->right, consts, lvl);
    binTreeUpdSize(new_node);
    return new_node;
}

BinTreeNode* processProgram(BinTreeNode* expr){
    ConstTable consts;
    constTableCtor(&consts);
    BinTreeNode* res = processCodeBlock(expr, &consts, 0);
    constTableDtor(&consts);

    simplifyMathForm(&res);
    return res;
}
