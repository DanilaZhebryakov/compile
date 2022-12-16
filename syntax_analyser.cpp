#include "expr/expr_elem.h"
#include "expr/formule_utils.h"
#include <ctype.h>

static char getNextMeaningChar(FILE* file){
    char c = ' ';
    int comment_st = 0;
    do {
        c = fgetc(file);
        if(c == '<')
            comment_st++;
        if(c == '>')
            comment_st--;
        if(c == EOF)
            return c;
    } while (isspace(c) || comment_st > 0);
    return c;
}

static void refillElemBuffer_(FILE* file, char* buffer, ExprElem* elem_buffer){
    char c = getNextMeaningChar(file);

    if (c == EOF || (strchr(CNTRL_CHARS, c))){
        elem_buffer->type = EXPR_CNTRL;
        elem_buffer->chr  = c;
        return;
    }
    ungetc(c, file);

    *elem_buffer = scanExprElem(file, c, buffer);
}

static bool scanMathExpr_(FILE* file, BinTreeNode** tree_place, char* buffer, ExprElem* elem_buffer, int priority);

static bool scanMathPExpr_(FILE* file, BinTreeNode** tree_place, char* buffer, ExprElem* elem_buffer, bool short_scan = false);

static bool scanMathBExpr_(FILE* file, BinTreeNode** tree_place, char* buffer, ExprElem* elem_buffer, bool short_scan){
    if (elem_buffer->type == EXPR_OP){
            if (!canExprOpBeUnary(elem_buffer->op)){
                error_log("Binary operator %s used as unary\n", exprOpName(elem_buffer->op));
                return false;
            }
            elem_buffer->op = (exprOpType_t)(elem_buffer->op | EXPR_O_UNARY);
            *tree_place = binTreeNewNode(*elem_buffer);
            BinTreeNode** tree_place_cur = &(*tree_place)->right;

            int op_priority = getExprOpPriority(elem_buffer->op);
            refillElemBuffer_(file, buffer, elem_buffer);

            if (short_scan){
                if (!scanMathPExpr_(file, tree_place_cur, buffer, elem_buffer, true))
                    return false;
            }
            else{
                if (!scanMathExpr_(file, tree_place_cur, buffer, elem_buffer, op_priority))
                   return false;
            }
            binTreeUpdSize(*tree_place);
            return true;

    }
    *tree_place = binTreeNewNode(*elem_buffer);
    if (!short_scan)
        refillElemBuffer_(file, buffer, elem_buffer);

    if ((*tree_place)->data.type == EXPR_VAR && (elem_buffer->type == EXPR_CNTRL && elem_buffer->chr == '(') && !short_scan){
        (*tree_place)->data.type = EXPR_FUNC;
        if (!scanMathPExpr_(file, &((*tree_place)->right), buffer, elem_buffer))
            return false;
        binTreeUpdSize(*tree_place);
    }

    return true;
}

static bool scanMathPExpr_(FILE* file, BinTreeNode** tree_place, char* buffer, ExprElem* elem_buffer, bool short_scan){
    if (elem_buffer->type != EXPR_CNTRL){
        return scanMathBExpr_(file, tree_place, buffer, elem_buffer, short_scan);
    }

    char c = elem_buffer->chr;
    if (c == '('){
        refillElemBuffer_(file, buffer, elem_buffer);
        if (!scanMathExpr_(file, tree_place, buffer, elem_buffer, -5))
            return false;
        if(elem_buffer->type != EXPR_CNTRL){
            printf("No closing ')' Got \"");
            printExprElem(stdout, *elem_buffer);
            printf("(%d) instead\n", elem_buffer->type );
            return false;
        }
        c = elem_buffer->chr;
        if (c != ')'){
            error_log("No closing ')' Got '%c' instead\n", c);
            return false;
        }

        if (!short_scan)
            refillElemBuffer_(file, buffer, elem_buffer);
        return true;
    }

    error_log("Unexpected character %c found\n", c);
    return false;

}

static bool isEndOfExpr(ExprElem* elem_buffer){
    return (elem_buffer->type == EXPR_CNTRL && !strchr("({", elem_buffer->chr));
}

static bool scanMathExpr_(FILE* file, BinTreeNode** tree_place, char* buffer, ExprElem* elem_buffer, int priority){
    assert_log(tree_place != nullptr);
    if (isEndOfExpr(elem_buffer)){
        *tree_place = nullptr;
        return true;
    }

    if (priority > MAX_EXPR_OP_PRIORITY){
        return scanMathPExpr_(file, tree_place, buffer, elem_buffer);
    }
    BinTreeNode** tree_place_cur = tree_place;
    while (1){
        if (!scanMathExpr_(file, tree_place_cur, buffer, elem_buffer, priority+1))
            return false;
        if (elem_buffer->type == EXPR_OP && isExprOpUnary(elem_buffer->op)){
            error_log("Unary operator %s used as binary\n", exprOpName(elem_buffer->op));
            return false;
        }
        binTreeUpdSize(*tree_place);

        if (elem_buffer->type == EXPR_CNTRL)
            return true;
        if (elem_buffer->type != EXPR_OP){
            error_log("Op required between two arguments\n");
            printExprElem(stdout, *elem_buffer);
            printf("\n");
            printMathForm(stdout, *tree_place);
            printf("\n");
            return false;
        }
        if (getExprOpPriority(elem_buffer->op) < priority)
            return true;

        BinTreeNode* old_node = *tree_place;
        BinTreeNode* new_node = binTreeNewNode(*elem_buffer);
        *tree_place = new_node;
        new_node->left = old_node;
        tree_place_cur = &(new_node->right);
        refillElemBuffer_(file, buffer, elem_buffer);
        if (isEndOfExpr(elem_buffer)){
            binTreeUpdSize(*tree_place);
            return canExprOpBeUnaryL(new_node->data.op);
        }
    }
    return false;
}

BinTreeNode* scanProgram(FILE* file){
    char* buffer = (char*)calloc(MAX_FORM_WORD_LEN, sizeof(char));
    BinTreeNode* tree =  nullptr;
    ExprElem elem_buffer = {};
    elem_buffer.type = EXPR_PAIN;

    refillElemBuffer_(file, buffer, &elem_buffer);
    bool res = scanMathPExpr_(file, &tree, buffer, &elem_buffer, true);
    free(buffer);

    if (!res){
        binTreeDump(tree);
        binTreeDtor(tree);
        return nullptr;
    }
    return tree;
}
