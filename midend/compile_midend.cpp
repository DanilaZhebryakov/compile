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
    if (node->data.type == EXPR_OP && node->data.op == EXPR_O_ENDL){
        BinTreeNode* ret = processCode(node, consts, lvl+1);
        while (consts->size > 0 && (((ConstEntry*)consts->data) + consts->size - 1)->depth >= lvl) {
            binTreeDtor(constTableGetLast(consts)->value); //dtor needed, so just descendlvl will not work

            assert_log(ustackPop(consts, nullptr) == VAR_NOERROR);
        }

        return ret;
    }
    return processCode(node, consts, lvl);
}

enum assDirType_t{
    ASSIGN_REV_MASK = 1,
    ASSIGN_DST_MASK = 2,
    ASSIGN_RTL = 1,
    ASSIGN_LTR  = 0,
    ASSIGN_DST   = 2,
    ASSIGN_SRC   = 0,
    ASSIGN_UNDEFINED = 4
};

static void processAssignChainBreak(BinTreeNode* node, ConstTable* consts, int lvl, BinTreeNode** place, BinTreeNode** top, assDirType_t dir);

static BinTreeNode* processAssignChain(BinTreeNode* node, ConstTable* consts, int lvl, BinTreeNode** place, BinTreeNode** top, assDirType_t dir = ASSIGN_UNDEFINED){
    if (!node){
        return nullptr;
    }
    if (node->data.type != EXPR_OP || !isAssignOp(node->data.op)) {
        if (*place){
            printf("Invalid assign chain\n");
            return nullptr;
        }
        *place = processCode(node, consts, lvl);
        return nullptr;
    }

    if (dir == ASSIGN_UNDEFINED){
        dir = (node->data.op | EF_REV) ? ASSIGN_LTR : ASSIGN_LTR;
    }
    else{
        if (!(dir & ASSIGN_REV_MASK) != !(node->data.op & EF_REV)){ // !. != !. is logical xor. This indicates a broken chain (direction change)
            processAssignChainBreak(node, consts, lvl, place, top, dir);
        }
    }
    
    processAssignChain((dir & ASSIGN_REV_MASK) ? node->right : node->left, consts, lvl, place, top, (assDirType_t)(dir | ASSIGN_DST_MASK));
    BinTreeNode* tmp = binTreeNewNode(node->data);
    if ((exprOpType_t)(tmp->data.op & EF_REV))
        tmp->data.op = (exprOpType_t)(tmp->data.op ^ EF_REV);
    tmp->left = *place;
    *place = tmp;
    place = &(tmp->right);

    BinTreeNode* res = processAssignChain(node->left, consts, lvl, place, top, (assDirType_t)(dir | ASSIGN_DST_MASK));
    return res ? res : tmp;
}

static void processAssignChainBreak(BinTreeNode* node, ConstTable* consts, int lvl, BinTreeNode** place, BinTreeNode** top, assDirType_t dir){
    if (!node){
        return;
    }
    
    if (dir & ASSIGN_DST_MASK) { // we reached .... → a ← .[we are here]. situation
        // *top is currently nullptr
        if (*top){
            printf("Invalid assign chain\n");
            return;
        }

        processAssignChain(node, consts, lvl, top, top); //first, process the chain on the other side
        // it can split away to chains before or after this directly altering *top. It will hang its subtree at top
        if (!top){
            return;
        }
        // returned subtree will be like:
        //       top -> (OP, ← )
        //             /        \  
        //        (VAR, a)    [some subtree]
                
        // need to modify it this way:
        //     top -> (OP, !←!)
        //           /         \ 
        //      (VAR, a)    (OP, ⁝ )
        //                 /        \ 
        //          [some subtree]  [nullptr] <- our place point (*place)

        BinTreeNode* eq_spec_node = binTreeNewNode(node->data);
        eq_spec_node->data.op = EXPR_O_EQSPEC;
        eq_spec_node->left = (*top)->left;

        BinTreeNode* sep_node = binTreeNewNode(node->data);
        sep_node->data.op = EXPR_O_SEP;
        eq_spec_node->right = sep_node;

        if (dir & ASSIGN_REV_MASK) { // define an order in which arguents are passed. In this case, always left to right.
            sep_node->right = (*top)->right;
            *top = sep_node->left;
        }
        else {
            sep_node->left = (*top)->right;
            *top = sep_node->right;
        }
        *place = *top;
        return;
    }
    else { // reached .[we are here]. ← a → ...... situation
        /* top points at our top assignment, place points at where the last element should be put.*/
        /* The only problem is that this last element it is shared*/

        BinTreeNode* new_top = binTreeNewNode(node->data);
        new_top->data.op = EXPR_O_COMMA;

        BinTreeNode* sep_node = binTreeNewNode(node->data); // prepare node containing (!🇷! ⁝ !🇹!), as it is used in reads
        sep_node->data.op = EXPR_O_SEP;

        sep_node->left = binTreeNewNode(node->data);
        sep_node->left->data.type = EXPR_KVAR;
        sep_node->left->data.kword = EXPR_KW_CRET;
        sep_node->right = binTreeNewNode(node->data);
        sep_node->right->data.type = EXPR_KVAR;
        sep_node->right->data.kword = EXPR_KW_TEMP;

        BinTreeNode* eq_spec_node = binTreeNewNode(node->data); // prepare double-read node
        eq_spec_node->data.op = EXPR_O_EQSPEC;
        eq_spec_node->left = sep_node;

                
        if (dir & ASSIGN_REV_MASK) { //assignment left to right and shared elem is on the left. So SECOND result is required.
            // so other half of the chain should be run FIRST (it gets first result and puts second into T)
            new_top->right = *top;
            *top = new_top;
            BinTreeNode* shared_read_node = processAssignChain(node, consts, lvl, &(new_top->left), &(new_top->left));

            // new_top-> (op, ",")                                 eq_spec_node ->  (op, !←!)
            //          /         \                                                /         \         
            //   (op, ←)       (op, ←) <- old *top                            (op, ⁝ )      (var, a)
            //  .......................                                      /        \   
            //             \                \                       (Kvar, !🇷!)     (Kvar, !🇹!)
            //shared_read-> (op, ←)        (nullptr) <- *place
            //             /       \   
            //     (var, ?)       (var, a)

            // need to replace plain read of a with double read (second to T) and place T in *place
            //  .......................
            //             \                \ 
            //shared_read-> (op, ←)          \ <- *place
            //             /       \          \ 
            //     (var, ?)       (op, !←!)    | 
            //                   /         \ 
            //              (op, ⁝ )      (var, a)
            //             /        \   
            //  (Kvar, !🇷!)     (Kvar, !🇹!)

            eq_spec_node->right = shared_read_node->right; 
            shared_read_node->right = eq_spec_node;

            *place = sep_node->right; //T for reading
            (*place)->usedc++;
            return;
        }
        else{ // assignment was right-to-left and shared var is on the RIGHT
            // need to double-read in this half of the chain and save SECOND value in T. Then run the other half.
            // *place should contain eq_spec node
            new_top->left = *top;
            *top = new_top;
            BinTreeNode* temp_read_node = processAssignChain(node, consts, lvl, &(new_top->right), &(new_top->right));

            BinTreeNode* eq_spec_node = binTreeNewNode(node->data);
            eq_spec_node->data.op = EXPR_O_EQSPEC;
            eq_spec_node->right = temp_read_node->right;
            temp_read_node->right = sep_node->right;  //T for reading
            temp_read_node->right->usedc++;
            return;
        }
    }
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

    if (isAssignOp(node->data.op) && lvl > 1){
        BinTreeNode* ret = nullptr;
        processAssignChain(node, consts, lvl, &ret, &ret);
        return ret;
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
