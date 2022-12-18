#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "lib/bintree.h"
#include "expr/formule_utils.h"

ExprElem readElemFromFile_st(FILE* file, char c, char* buffer){
    ExprElem ret = {};
    ret.type = EXPR_NULL;
    ret.file_line = 0;
    ret.line_pos = 0;
    ret.file_name = "standart-govno.real";
    if (c == '"'){
        fscanf(file, "\"%[^\"]\"", buffer);
        ret.type = EXPR_VAR;
        ret.name = strdup(buffer);
        return ret;
    }
    if (isdigit(c)){
        ret.type = EXPR_CONST;
        fscanf(file, "%lf", &(ret.val));
        return ret;
    }
    fscanf(file, "%s", buffer);

    if(strcmp(buffer, "NIL") == 0){
        ret.type = EXPR_NULL;
        return ret;
    }

    ret.type = EXPR_STAND;
    #define EXPR_ST_DEF(_enum, _enumval, _std) \
    if (strcmp(buffer, _std) == 0) { \
        ret.stand = _enum;           \
        return ret;                  \
    }
    #include "expr/expr_st_defines.h"
    #undef EXPR_ST_DEF

    ret.type = EXPR_KVAR;
    #define EXPR_KW_DEF(_enum, _enumval, _name, _std) \
    if (strcmp(buffer, _std) == 0) { \
        ret.kword = _enum;           \
        return ret;                  \
    }
    #include "expr/expr_kw_defines.h"
    #undef EXPR_KW_DEF

    ret.type = EXPR_OP;
    #define EXPR_OP_DEF(_enum, _enumval, _name, _pri, _ret, _std) \
    if (strcmp(buffer, _std) == 0) { \
        ret.op = _enum;              \
        return ret;                  \
    }
    #include "expr/expr_op_defines.h"
    #undef EXPR_OP_DEF

    ret.type = EXPR_PAIN;
    return ret;
}

static BinTreeNode* readTreeFromFile_st_(FILE* file, char* buffer){
    char c = fgetc(file);
    while (isspace(c) || iscntrl(c)){
        c = fgetc(file);
    }
    if (c == '}'){
        return nullptr;
    }

    BinTreeNode* node = (BinTreeNode*)malloc(sizeof(*node));
    binTreeNodeCtor(node, {EXPR_NULL});

    int next_set = 0;
    while (c != '}' && c != EOF){
        if (c == '{'){
            BinTreeNode* new_node = readTreeFromFile_st_(file, buffer);
            switch (next_set){
            case 0:
                node->left = new_node;
                break;
            case 1:
                node->right = new_node;
                break;
            }
            next_set++;
        }
        else{
            if(!isspace(c) && !iscntrl(c)){
                ungetc(c, file);
                node->data = readElemFromFile_st(file, c, buffer);
            }
        }

        c = fgetc(file);
    }
    if (c == EOF || node->data.type == EXPR_NULL){
        binTreeDtor(node);
        return nullptr;
    }
    return node;
}

BinTreeNode* readTreeFromFile_st(FILE* file){
    char c = fgetc(file);
    while (c != EOF && c != '{'){
        c = fgetc(file);
    }
    if (c == '{'){
        char* buffer = (char*)calloc(100, sizeof(char));
        BinTreeNode* ret = readTreeFromFile_st_(file, buffer);
        binTreeBuild(ret);
        free(buffer);
        return ret;
    }
    return nullptr;
}

static BinTreeNode* newOpNode(exprOpType_t op, BinTreeNode* a, BinTreeNode* b){
    ExprElem elem = {};
    elem.type = EXPR_OP;
    elem.op   = op;
    BinTreeNode* node = binTreeNewNode(elem);
    node->left = a;
    node->right = b;
    binTreeUpdSize(node);
    return node;
}

static BinTreeNode* destandartifyProgram_(BinTreeNode* expr, bool arg = false){
    if(!expr)
        return nullptr;
    if(expr->data.type == EXPR_VAR || expr->data.type == EXPR_CONST){
        expr->usedc++;
        return expr;
    }

    if(expr->data.type == EXPR_OP && (expr->data.op == EXPR_O_ENDL || expr->data.op == EXPR_O_COMMA)){
        BinTreeNode* cur_o_node = expr->right;
        BinTreeNode* cur_n_node = destandartifyProgram_(expr->left);
        exprOpType_t new_op = expr->data.op;
        if(arg){
            new_op = EXPR_O_ENDL;
        }
        while(cur_o_node && cur_o_node->data.type == EXPR_OP && cur_o_node->data.op == expr->data.op){
            BinTreeNode* new_node = binTreeNewNode({EXPR_OP});
            new_node->data.op = new_op;
            new_node->left = cur_n_node;
            new_node->right = destandartifyProgram_(cur_o_node->left);
            binTreeUpdSize(new_node);
            cur_n_node = new_node;
            cur_o_node = cur_o_node->right;
        }
        return cur_n_node;
    }

    if (expr->data.type == EXPR_STAND){
        if (expr->data.stand == EXPR_ST_IN || expr->data.stand == EXPR_ST_INCH){
            BinTreeNode* cur_o_node = expr->left->right;
            BinTreeNode* inp_node = binTreeNewNode({EXPR_KVAR});
            inp_node->data.kword = (expr->data.stand == EXPR_ST_IN) ? EXPR_KW_NIO : EXPR_KW_CIO;
            BinTreeNode* cur_n_node = newOpNode(EXPR_O_EQLTR, inp_node, destandartifyProgram_(cur_o_node->left));

            while(cur_o_node && cur_o_node->data.type == EXPR_OP && cur_o_node->data.op == EXPR_O_COMMA){
                BinTreeNode* new_node = newOpNode(EXPR_O_EQLTR, inp_node, destandartifyProgram_(cur_o_node->left));
                inp_node->usedc++;
                BinTreeNode* endl_node = newOpNode(EXPR_O_ENDL, cur_n_node, new_node);
                binTreeUpdSize(endl_node);
                cur_n_node = endl_node;
                cur_o_node = cur_o_node->right;
            }
            return cur_n_node;
        }
        if (expr->data.stand == EXPR_ST_OUT || expr->data.stand == EXPR_ST_OUTCH){
            BinTreeNode* out_node = binTreeNewNode({EXPR_KVAR});
            out_node->data.kword = (expr->data.stand == EXPR_ST_OUT) ? EXPR_KW_NIO : EXPR_KW_CIO;
            return newOpNode(EXPR_O_EQRTL, out_node, destandartifyProgram_(expr->left));
        }
        if (expr->data.stand == EXPR_ST_FUNC){
            BinTreeNode* name_node = binTreeNewNode({EXPR_VAR});
            name_node->data.name = strdup(expr->left->data.name);
            BinTreeNode* new_node = newOpNode(EXPR_O_FDEF, name_node,
                                              newOpNode(EXPR_O_SEP, destandartifyProgram_(expr->left->left, true  )
                                                                  , destandartifyProgram_(expr->right             ) ));
            return new_node;
        }
        if (expr->data.stand == EXPR_ST_CALL){
            BinTreeNode* new_node = binTreeNewNode({EXPR_FUNC});
            new_node->data.name = strdup(expr->left->data.name);
            new_node->right = destandartifyProgram_(expr->left->left, true);
            return new_node;
        }
    }

    if (expr->data.type == EXPR_KVAR && expr->data.kword == EXPR_KW_RET){
        BinTreeNode* ret_node = binTreeNewNode({EXPR_KVAR});
        ret_node->data.kword = EXPR_KW_RET;
        BinTreeNode* retval = destandartifyProgram_(expr->left);
        if (!retval){
            retval = binTreeNewNode({EXPR_CONST});
            retval->data.val = 0;
        }
        return newOpNode(EXPR_O_EQRTL, ret_node, retval);
    }

    if(expr->data.type != EXPR_OP){
        BinTreeNode* new_node = binTreeNewNode(expr->data);
        error_log("Element not supported:");
        printExprElem(stdout  , expr->data);
        printExprElem(_logfile, expr->data);
        printf_log("\n");
        new_node->data.type = EXPR_PAIN;
        return new_node;
    }
    if (expr->data.op == EXPR_O_VDEF){
        return newOpNode(EXPR_O_VDEF, nullptr,
                newOpNode(EXPR_O_EQRTL, destandartifyProgram_(expr->left), destandartifyProgram_(expr->right)));
    }


    BinTreeNode* new_node = binTreeNewNode(expr->data);
    expr->left  = destandartifyProgram_(expr->left );
    expr->right = destandartifyProgram_(expr->right);
    if(canExprOpBeUnary(expr->data.op) && !(expr->right)){
        new_node->right = new_node->left;
        new_node->left = nullptr;
    }
    binTreeUpdSize(new_node);
    return new_node;
}

BinTreeNode* destandartifyProgram(BinTreeNode* expr){
    BinTreeNode* res = destandartifyProgram_(expr);
    BinTreeNode* main_f_node = binTreeNewNode({EXPR_FUNC});
    main_f_node->data.name = strdup("main");
    res = newOpNode(EXPR_O_ENDL, res, main_f_node);

    return res;
}
