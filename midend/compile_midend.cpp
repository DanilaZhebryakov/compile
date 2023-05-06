#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>

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

struct assDirType_t{
    bool dir_ltr:1;
    bool dir_dst:1;
    bool dir_undefined:1;
};

/*
struct AssChainInfo{
    BinTreeNode* place;
    BinTreeNode* top;
    BinTreeNode* bottom_assign_op;
}*/

/*
// Assignment chain is a subtree consisting of only assignment type (EF_EQL) operations
// Assignment may have left or right direction and these directions can even combine.
// Situations which should be taken care of: assignment of two values to one element from both sides, assignment from one expression to both sides
// Dfs is used to go over the tree in order from values to destinations. Destinations are added by attaching existing tree to left subtree of new op.
*/
#define expandAssChain(new_elem)        \
    if(*place){                         \
        assert(!((*place)->left));      \
        (*place)->left = new_elem;      \
        binTreeUpdSize((*place));       \
    }                                   \
    else{                               \
        *place = new_elem;              \
    }                                   \

#define expandAfterChain(new_elem)      \
    if (*after) {                       \
        BinTreeNode* new_node = binTreeNewNode(new_elem->data);   \
        new_node->data.type = EXPR_OP;       \
        new_node->data.op   = EXPR_O_COMMA;    \
        new_node->left = new_elem;      \
        new_node->right = *after;       \
        *after = new_node->right;       \
    }                                   \
    else{                               \
        *after = new_elem;              \
    }                                   \

static BinTreeNode* processAssignChain(BinTreeNode* node, ConstTable* consts, int lvl, BinTreeNode** place, BinTreeNode** top, BinTreeNode** bottom_assign_op, assDirType_t dir = {0,0,1});

static void processAssignChainSubtree(BinTreeNode* node, ConstTable* consts, int lvl, BinTreeNode** place, BinTreeNode** bottom_assign_op = nullptr){
    BinTreeNode* st_bottom = nullptr;
    BinTreeNode* st_after  = nullptr;
    
    processAssignChain(node, consts, lvl, place, &st_after, bottom_assign_op ? bottom_assign_op : &st_bottom);
    if (st_after) {
        BinTreeNode* link_node = binTreeNewNode(node->data);
        link_node->data.type = EXPR_OP;
        link_node->data.op   = EXPR_O_CEL;
        link_node->left  = *place;
        link_node->right = st_after;
        binTreeUpdSize(link_node);
        *place = link_node;
    }
}

static void processAssignChainBreak(BinTreeNode* node, ConstTable* consts, int lvl, BinTreeNode** place, BinTreeNode** top, BinTreeNode** bottom_assign_op, assDirType_t dir);

static BinTreeNode* processAssignChain(BinTreeNode* node, ConstTable* consts, int lvl, BinTreeNode** place, BinTreeNode** top, BinTreeNode** bottom_assign_op, assDirType_t dir){
    if (!node){
        return nullptr;
    }

    if (node->data.type != EXPR_OP || !isAssignOp(node->data.op)) {
        BinTreeNode* temp = processCode(node, consts, lvl);
        expandAssChain(temp)
        return nullptr;
    }

    if (dir.dir_undefined){
        dir.dir_undefined = false;
        dir.dir_ltr = (node->data.op & EF_REV);
    }
    else{
        if (!(dir.dir_ltr) != !(node->data.op & EF_REV)){ // !. != !. is logical xor. This indicates a broken chain (direction change)
            processAssignChainBreak(node, consts, lvl, place, top, bottom_assign_op, dir);
            return nullptr;
        }
    }

    dir.dir_dst = false;
    processAssignChain((dir.dir_ltr) ? node->left : node->right, consts, lvl, place, top, bottom_assign_op, dir);
    
    BinTreeNode* op_node = binTreeNewNode(node->data);
    if ((exprOpType_t)(op_node->data.op & EF_REV))
        op_node->data.op = (exprOpType_t)(op_node->data.op ^ EF_REV);
    op_node->right = *place;
    if (!*bottom_assign_op){
        *bottom_assign_op = op_node;
    }

    *place = op_node;
    dir.dir_dst = true;
    processAssignChain((dir.dir_ltr) ? node->right : node->left, consts, lvl, place, top, bottom_assign_op, dir);
    return nullptr;
}

static void processAssignChainBreak(BinTreeNode* node, ConstTable* consts, int lvl, BinTreeNode** place, BinTreeNode** after, BinTreeNode** bottom_assign_op, assDirType_t dir){
    if (!node){
        return;
    }
    if (dir.dir_dst) { // we reached .... → a ← .[we are here]. situation

        BinTreeNode* new_place = nullptr;
        BinTreeNode* new_bottom_op = nullptr;
        processAssignChain(node, consts, lvl, &new_place, &new_bottom_op, &new_place); //first, process the chain on the other side
        // it can split away to chains before or after this directly altering *top. It will hang its subtree at place
        if (!new_place){
            printf("Invalid assign chain");
            return;
        }

        /*
        // returned subtree will be like:
        // new_place -> (OP, ← )
        //             /        \  
        //        (VAR, a)    [some subtree]
                
        // need to modify it this way:
        //             (OP, !←!) <- *place
        //            /         \ 
        //       (VAR, a)    (OP, ⁝ )
        //                  /        \ 
        //new subtree(OP, ← )         (OP, ← ) <- current subtree
        //           |      \           |        \ 
        //    (nullptr) [some subtree] (nullptr)  [some subtree]
        */

        BinTreeNode* eq_spec_node = binTreeNewNode(node->data);
        eq_spec_node->data.op = EXPR_O_EQSPEC;
        eq_spec_node->left = new_place->left;
        new_place->left = nullptr;

        BinTreeNode* sep_node = binTreeNewNode(node->data);
        sep_node->data.op = EXPR_O_SEP;
        eq_spec_node->right = sep_node;

        if (dir.dir_ltr) { // define an order in which arguents are passed. In this case, always left to right.
            sep_node->left   = *place;
            sep_node->right  = new_place;
        }
        else {
            sep_node->right = *place;
            sep_node->left  = new_place;
        }
        binTreeUpdSize(sep_node);
        binTreeUpdSize(eq_spec_node);
        *place = eq_spec_node;

        return;
    }
    else { // reached .[we are here]. ← a → ...... situation
        /* top points at our top assignment, place points at where the last element should be put.*/
        /* The only problem is that this last element it is shared*/

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
                
        if (dir.dir_ltr) { //assignment left to right and shared elem is on the left. So SECOND result is required.
            // so other half of the chain should be run FIRST (it gets first result and puts second into T)
            BinTreeNode* new_top = binTreeNewNode(node->data);
            new_top->data.op = EXPR_O_CER; //T on the right
            new_top->right = sep_node->right;
            sep_node->right->usedc++;

            BinTreeNode* shared_read_node = nullptr;

            processAssignChainSubtree(node, consts, lvl, &(new_top->left), &shared_read_node);
            assert(shared_read_node);
            assert(shared_read_node->usedc == 1);
            assert(shared_read_node->data.type == EXPR_OP);

            if (!*bottom_assign_op){
                *bottom_assign_op = shared_read_node;
            }

            BinTreeNode* st_shared_var = shared_read_node->right;

            if (st_shared_var && st_shared_var->data.type == EXPR_OP && st_shared_var->data.op == EXPR_O_EQSPEC){ // already multiread. Just adding T at the end
                BinTreeNode* subt_sep_node = st_shared_var->left;
                if (subt_sep_node->right && subt_sep_node->data.type == EXPR_OP && subt_sep_node->data.op == EXPR_O_SEP){
                    BinTreeNode* new_sep_node = binTreeNewNode(subt_sep_node->data);
                    binTreeDtor(sep_node->left);
                    sep_node->left = subt_sep_node->right;
                    subt_sep_node->right->usedc++;
                    new_sep_node->right = sep_node;
                    sep_node->usedc++;
                    new_sep_node->left  = subt_sep_node->left;
                    subt_sep_node->left->usedc++;

                    binTreeDtor(subt_sep_node);
                    binTreeDtor(eq_spec_node);
                    assert(st_shared_var->usedc == 1);
                    st_shared_var->left = new_sep_node;
                
                    binTreeUpdSize(sep_node);
                    binTreeUpdSize(new_sep_node);
                    binTreeUpdSize(st_shared_var);
                    binTreeUpdSize(shared_read_node);

                    expandAssChain(new_top) 
                    return;
                }
            }

            /*
            // new_top-> (op, "⸢")                                 eq_spec_node ->  (op, !←!)
            //          /         \                                                /         \         
            //   (op, ←)           >----------------------------v             (op, ⁝ )      (var, a)
            //  .......................                         |            /        \   
            //             \                \                   |   (Kvar, !🇷!)     (Kvar, !🇹!)
            //shared_read-> (op, ←)        (nullptr) <- *place  >------------------>/
            //             /       \   
            //     (var, ?)       (var, a)

            // need to replace plain read of a with double read (second to T) and place everything in *place
            //       new_top-> (op, "⸢")  <-*place
            //  .......................
            //                /             \
            //shared_read-> (op, ←)          \ 
            //             /       \          \ 
            //     (var, ?)       (op, !←!)    \ 
            //                   /         \    \
            //              (op, ⁝ )      (var, a)\
            //             /        \             /
            //  (Kvar, !🇷!)     (Kvar, !🇹!)<----<      
            */

            eq_spec_node->right = shared_read_node->right; 
            shared_read_node->right = eq_spec_node;

            expandAssChain(new_top) 
            return;
        }
        else{ // assignment was right-to-left and shared var is on the RIGHT
            // need to double-read in this half of the chain and save SECOND value in T. Then run the other half.
            // *place should contain eq_spec node

            BinTreeNode* new_place = nullptr;
            BinTreeNode* temp_read_node = nullptr;
            processAssignChainSubtree(node, consts, lvl, &new_place, &temp_read_node);
            assert(temp_read_node);
            assert(temp_read_node->data.type == EXPR_OP);
            assert(temp_read_node->usedc == 1);

            BinTreeNode* st_shared_var = temp_read_node->right;
            BinTreeNode* reading_t_node = sep_node->right;

            if (st_shared_var && st_shared_var->data.type == EXPR_OP && st_shared_var->data.op == EXPR_O_EQSPEC){
                BinTreeNode* subt_sep_node = st_shared_var->left;
                if (subt_sep_node->right && subt_sep_node->data.type == EXPR_OP && subt_sep_node->data.op == EXPR_O_SEP){
                    if (subt_sep_node->left && subt_sep_node->left->data.type == EXPR_KVAR
                    && subt_sep_node->left->data.kword == EXPR_KW_CRET) {
                        BinTreeNode* new_sep_node = binTreeNewNode(subt_sep_node->data);
                        new_sep_node->right = subt_sep_node->right;
                        subt_sep_node->right->usedc++;
                        subt_sep_node->left = sep_node->right;
                        sep_node->right = new_sep_node;
                        
                        temp_read_node->right = st_shared_var->right;
                        st_shared_var->right->usedc++;
                        binTreeDtor(st_shared_var);

                        binTreeUpdSize(new_sep_node);
                        binTreeUpdSize(sep_node);
                        binTreeUpdSize(eq_spec_node);
                    }
                    else{
                        printf("Error: non-standard multiread sequence\n");
                        printExprElem(stdout, subt_sep_node->left->data);
                        printf("\n");
                    }
                }
            }

            expandAfterChain(new_place)

            eq_spec_node->right = temp_read_node->right;
            temp_read_node->right = binTreeNewNode(temp_read_node->data);  //T for reading
            temp_read_node->right->data.type = EXPR_VAR;
            temp_read_node->right->data.name = "aaaa";
            reading_t_node->usedc++;

            expandAssChain(eq_spec_node)
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
        processAssignChainSubtree(node, consts, lvl, &ret);
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

BinTreeNode* processProgram(BinTreeNode* expr, int lvl){
    ConstTable consts;
    constTableCtor(&consts);
    BinTreeNode* res = processCodeBlock(expr, &consts, lvl);
    constTableDtor(&consts);
    binTreeDump(res);

    simplifyMathForm(&res);
    return res;
}
