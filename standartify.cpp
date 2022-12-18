#include <stdio.h>
#include <stdlib.h>

#include "lib/bintree.h"
#include "expr/formule_utils.h"

//DO NOT GO THERE
//CONTAINS GOVNOCODE
//WORKS A BIT
//I WILL DO STH WITH IT LATER

void writeElemToFile_st(ExprElem* elem, FILE* file){
    if(elem->type == EXPR_PAIN){
        fprintf(file, "\"*/?^%%!&$\"");
        return;
    }
    if(elem->type == EXPR_VAR){
        fprintf(file, "\"%s\"", elem->name);
        return;
    }
    if(elem->type == EXPR_CONST){
        fprintf(file, "%d", int(elem->val));
        return;
    }
    if(elem->type == EXPR_FUNC){
        fprintf(file, "\"%s\"", elem->name);
        return;
    }
    if(elem->type == EXPR_OP){
        fputs(exprStdOpName(elem->op), file);
        return;
    }
    if(elem->type == EXPR_KVAR){
        fputs(getExprStdKWName(elem->kword), file);
        return;
    }
    if(elem->type == EXPR_STAND){
        switch(elem->stand){
        case EXPR_ST_FUNC:
            fprintf(file, "FUNC");
            return;
        case EXPR_ST_CALL:
            fprintf(file, "CALL");
            return;
        case EXPR_ST_IN:
            fprintf(file, "IN");
            return;
        case EXPR_ST_OUT:
            fprintf(file, "OUT");
            return;
        case EXPR_ST_INCH:
            fprintf(file, "INCH");
            return;
        case EXPR_ST_OUTCH:
            fprintf(file, "OUTCH");
            return;
        case EXPR_ST_VOID:
            fprintf(file, "VOID");
            return;
        case EXPR_ST_TYPE:
            fprintf(file, "TYPE");
            return;
        default:
            fprintf(file, "STRASH");
            return;
        }
    }
    fprintf(file, "UNKNOWN");
    return;
}

static void writeProgramToFile_st_(BinTreeNode* expr, FILE* file, int layer){
    #define place_tabs(_count)      \
    for(int x = 0; x < (_count); x++){ \
        fprintf(file, "  ");        \
    }

    place_tabs(layer)
    writeElemToFile_st(&expr->data, file);
    fprintf (file, "\n");
    place_tabs(layer)
    if (expr->left){
        fprintf(file, "{\n");
        writeProgramToFile_st_(expr->left, file, layer+1);
    }
    else{
        fprintf(file, "{NIL}\n");
    }

    place_tabs(layer)
    if (expr->right){
        fprintf(file, "{\n");
        writeProgramToFile_st_(expr->right, file, layer+1);
    }
    else{
        fprintf(file, "{NIL}\n");
    }

    place_tabs(layer-1)
    fprintf(file, "}\n");
    return;
}

void writeProgramToFile_st(BinTreeNode* expr, FILE* file){
    if (binTreeError(expr, false)){
        printf("bad tree");
        return;
    }
    fprintf(file, "{\n");
    writeProgramToFile_st_(expr, file, 1);
}

// warning: you are entering pointer hell made to standartify one line of code to multiple without creating excess code blocks
static BinTreeNode** addStNode(BinTreeNode* expr, BinTreeNode** tgt, exprOpType_t link_op = EXPR_O_ENDL){
    if(*tgt == nullptr){
        *tgt = expr;
        return tgt;
    }
    BinTreeNode* temp = (*tgt);
    (*tgt) = binTreeNewNode({});
    (*tgt)->data.type = EXPR_OP;
    (*tgt)->data.op = link_op;
    (*tgt)->left  = temp;
    (*tgt)->right = expr;

    return &((*tgt)->right);
}

static void insertStNode(BinTreeNode* expr, BinTreeNode** tgt, exprOpType_t link_op = EXPR_O_ENDL){
    if(*tgt == nullptr){
        *tgt = expr;
        return;
    }
    BinTreeNode* temp = (*tgt);
    (*tgt) = binTreeNewNode({EXPR_OP});
    (*tgt)->data.op = link_op;
    (*tgt)->right  = temp;
    (*tgt)->left = expr;

}

static BinTreeNode** standartifyProgram_(BinTreeNode* expr, BinTreeNode** tgt, BinTreeNode** ret_val_node);

static BinTreeNode* standartifyCodeBlock_(BinTreeNode* expr, BinTreeNode** ret_val_node, exprOpType_t link_op = EXPR_O_ENDL){
    BinTreeNode* tgt = nullptr;
    BinTreeNode** block_end = standartifyProgram_(expr, &tgt, ret_val_node);
    if(*block_end)
        addStNode(nullptr, block_end, link_op);
    return tgt;
}

static BinTreeNode* standartifySetDst_(BinTreeNode* expr, BinTreeNode** pre, bool create_vars, BinTreeNode** ret_val_node){
    if(expr->data.type == EXPR_VAR){
        if(create_vars){
            BinTreeNode* var_cr_node = binTreeNewNode({EXPR_OP});
            var_cr_node->data.op = EXPR_O_VDEF;
            var_cr_node->left = expr;
            expr->usedc++;
            insertStNode(var_cr_node, pre);
        }
        expr->usedc++;
        *ret_val_node = expr;
        return expr;
    }
    if(expr->data.type == EXPR_KVAR){
        expr->usedc++;
        *ret_val_node = expr;
        return expr;
    }
    if(expr->data.type == EXPR_OP){
        BinTreeNode* op_node = binTreeNewNode(expr->data);
        op_node->left  = standartifySetDst_(expr->left , pre, create_vars, ret_val_node);
        op_node->right = standartifySetDst_(expr->right, pre, create_vars, ret_val_node);
        return op_node;
    }

    error_log("Invalid elem for write:");
    printExprElem(_logfile, expr->data);
    printExprElem(stderr  , expr->data);
    printf_log("\n");
    return binTreeNewNode({EXPR_PAIN});
}

static BinTreeNode** standartifySetVar_(BinTreeNode* expr, BinTreeNode** tgt, BinTreeNode** pre, bool create_vars, BinTreeNode** ret_val_node){
    assert_log(tgt);
    assert_log(expr);
    if(expr->data.type == EXPR_VAR){
        if(create_vars){
            BinTreeNode* var_cr_node = binTreeNewNode({EXPR_OP});
            var_cr_node->data.op = EXPR_O_VDEF;
            var_cr_node->left = expr;
            expr->usedc++;
            insertStNode(var_cr_node, pre);
        }
        return tgt;
    }
    if(expr->data.type != EXPR_OP){
        error_log("Bad elem ");
        printExprElem(stdout  , expr->data);
        printExprElem(_logfile, expr->data);
        printf_log("in var assignment\n");
        return tgt;
    }
    if(expr->data.op == EXPR_O_COMMA){
        tgt = standartifySetVar_(expr->left , tgt, pre, create_vars, ret_val_node);
        tgt = standartifySetVar_(expr->right, tgt, pre, create_vars, ret_val_node);
        return tgt;
    }

    if(!(expr->left && expr->right)){
        error_log("Assignment should have both L and R values\n");
        return tgt;
    }

    BinTreeNode* src = expr->right;
    BinTreeNode* dst = expr->left ;
    if (expr->data.op == EXPR_O_EQLTR){
        src = expr->left;
        dst = expr->right;
    }

    if (src->data.type == EXPR_KVAR && (src->data.kword == EXPR_KW_NIO || src->data.kword == EXPR_KW_CIO)) {
        BinTreeNode* new_node = binTreeNewNode({EXPR_STAND});
        new_node->data.stand = (dst->data.kword == EXPR_KW_NIO) ? EXPR_ST_IN : EXPR_ST_INCH;
        new_node->left = standartifyCodeBlock_(dst, ret_val_node, EXPR_O_COMMA);
        return addStNode(new_node ,tgt);
    }
    if (dst->data.type == EXPR_KVAR){

        if (dst->data.kword == EXPR_KW_NIO || dst->data.kword == EXPR_KW_CIO){
            BinTreeNode* new_node = binTreeNewNode({EXPR_STAND});
            new_node->data.stand = (dst->data.kword == EXPR_KW_NIO) ? EXPR_ST_OUT : EXPR_ST_OUTCH;
            new_node->left = standartifyCodeBlock_(src, ret_val_node, EXPR_O_COMMA);
            return addStNode(new_node ,tgt);
        }
        if (dst->data.kword == EXPR_KW_RET){
            BinTreeNode* new_node = binTreeNewNode(dst->data);
            standartifyProgram_(src, &(new_node->left), ret_val_node);
            return addStNode(new_node ,tgt);
        }
        if (dst->data.kword == EXPR_KW_HALT){
            return tgt; // just let it (not) be and create bugs
        }
        if (dst->data.kword == EXPR_KW_BAD){
            BinTreeNode* new_node = binTreeNewNode({EXPR_KVAR});
            new_node->data.kword = EXPR_KW_RET;
            new_node->left = binTreeNewNode({EXPR_CONST});
            new_node->left->data.val = rand(); // this will maybe create the bug and segfault/error in user program, so this is what we need :)
            return addStNode(new_node ,tgt);
        }
    }
    BinTreeNode* new_node = binTreeNewNode({EXPR_OP});
    new_node->data.op = EXPR_O_EQRTL;
    standartifyProgram_(src, &(new_node->right), ret_val_node);

    if (dst->data.type == EXPR_VAR){
        new_node->left  = standartifySetDst_(dst, pre, create_vars, ret_val_node);
    }


    return addStNode(new_node ,tgt);
}

static BinTreeNode** standartifyProgram_(BinTreeNode* expr, BinTreeNode** tgt, BinTreeNode** ret_val_node){
    assert_log(tgt != nullptr);
    if(!expr)
        return tgt;
    if(expr->data.type == EXPR_VAR || expr->data.type == EXPR_CONST){
        expr->usedc++;
        return addStNode(expr, tgt);
    }

    if(expr->data.type == EXPR_OP && expr->data.op == EXPR_O_ENDL){
        tgt = standartifyProgram_(expr->left , tgt, ret_val_node);
        if(expr->right && expr->right->data.type == EXPR_OP && expr->right->data.type == EXPR_O_ENDL){ // code block creation
            BinTreeNode* cb_node = standartifyCodeBlock_(expr->right, ret_val_node);
            tgt = addStNode(cb_node, tgt);
        }
        else{
            tgt = standartifyProgram_(expr->right, tgt, ret_val_node);
        }
        if(*tgt)
            tgt = addStNode(nullptr, tgt);
        return tgt;
    }
    if(expr->data.type == EXPR_OP && expr->data.op == EXPR_O_COMMA){
        tgt = standartifyProgram_(expr->left , tgt, ret_val_node); // comma (param) does not create code blocks, but works exactly the same otherwise
        tgt = standartifyProgram_(expr->right, tgt, ret_val_node);
        if(*tgt)
            tgt = addStNode(nullptr, tgt, EXPR_O_COMMA);
        return tgt;
    }

    if(expr->data.type == EXPR_FUNC){
        BinTreeNode* new_node  = binTreeNewNode({EXPR_STAND});
        BinTreeNode* name_node = binTreeNewNode({EXPR_VAR});
        new_node->data.stand = EXPR_ST_CALL;
        name_node->data.name = strdup(expr->data.name);

        new_node->left  = name_node;
        name_node->left = standartifyCodeBlock_(expr->right, ret_val_node, EXPR_O_COMMA);
        return addStNode(new_node, tgt);
    }

    if(expr->data.type == EXPR_KVAR){
        if (expr->data.kword == EXPR_KW_HALT){
            return tgt; // not required
        }
        BinTreeNode* new_node = binTreeNewNode(expr->data);
        switch(expr->data.kword){
        case EXPR_KW_RET:
            new_node->data.type  = EXPR_KVAR;
            new_node->data.kword = EXPR_KW_RET;
            break;
        case EXPR_KW_BAD:
            new_node->data.type  = EXPR_KVAR;
            new_node->data.kword = EXPR_KW_RET;
            new_node->left = binTreeNewNode({EXPR_CONST});
            new_node->left->data.val = rand(); //let's see what happens
            break;
        default:
            break;
        }
        return addStNode(new_node, tgt);
    }

    if(expr->data.type != EXPR_OP){
        BinTreeNode* new_node = binTreeNewNode(expr->data);
        error_log("Element not supported");
        new_node->data.type = EXPR_PAIN;
        return addStNode(new_node, tgt);

    }

    if(expr->data.op == EXPR_O_VDEF){
        if(*tgt){
            tgt = addStNode(nullptr, tgt);
        }
        tgt = standartifySetVar_(expr->right, tgt, tgt, true, ret_val_node);
        return tgt;
    }
    if(expr->data.op == EXPR_O_EQLTR || expr->data.op == EXPR_O_EQRTL){
        if(*tgt){
            tgt = addStNode(nullptr, tgt);
        }
        tgt = standartifySetVar_(expr, tgt, tgt, true, ret_val_node);
        return tgt;
    }

    if(expr->data.op == EXPR_O_FDEF){
        BinTreeNode* new_node  = binTreeNewNode({EXPR_STAND});
        BinTreeNode* name_node = binTreeNewNode({EXPR_VAR});
        new_node->data.stand = EXPR_ST_FUNC;
        printf_log("%p", expr->left->data.name);
        name_node->data.name = strdup(expr->left->data.name);

        new_node ->left  = name_node;
        name_node->left  = standartifyCodeBlock_(expr->right->left, ret_val_node, EXPR_O_COMMA);
        name_node->right = binTreeNewNode({EXPR_STAND});
        name_node->right->data.stand = EXPR_ST_TYPE;

        new_node ->right = standartifyCodeBlock_(expr->right->right, ret_val_node);
        return addStNode(new_node, tgt);
    }
    BinTreeNode* new_node = binTreeNewNode(expr->data);

    if(!expr->left){
       standartifyProgram_(expr->right, &(new_node->left), ret_val_node); //again, standartic order of nodes
       return addStNode(new_node, tgt);
    }

    //op does not need special handling by default
    standartifyProgram_(expr->left , &(new_node->left ), ret_val_node);
    standartifyProgram_(expr->right, &(new_node->right), ret_val_node);
    return addStNode(new_node, tgt);
}

BinTreeNode* standartifyProgram(BinTreeNode* expr){
    BinTreeNode* res = nullptr;
    BinTreeNode* retval = nullptr;
    standartifyProgram_(expr, &res, &retval);

    BinTreeNode* main_func_node = binTreeNewNode({EXPR_STAND});
    main_func_node->data.stand = EXPR_ST_FUNC;

    main_func_node->left  = binTreeNewNode({EXPR_VAR});
    main_func_node->left ->data.name = strdup("main");
    main_func_node->left->right = binTreeNewNode({EXPR_STAND});
    main_func_node->left->right->data.stand = EXPR_ST_VOID;
    main_func_node->right = res;
    addStNode(nullptr, &main_func_node);

    binTreeBuild(main_func_node); // due to pointer hell, tree size needs to be updated
    return main_func_node;
}
