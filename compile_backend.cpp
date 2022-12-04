#include "lib/bintree.h"
#include "math/expr_elem.h"
#include "var_table.h"
#include "program_structure.h"

#define F_ARGS(...) file, __VA_ARGS__ , objs, pos
#define F_DEF_ARGS FILE* file, BinTreeNode* expr , ProgramNameTable* objs, ProgramPosData* pos, bool req_val

static void compileFuncCall(FILE* file, ProgramPosData* pos, int nargs){
    fprintf(file, "#call of func\n");
    fprintf(file, "push rbp\n");
    fprintf(file, "push %d \n", pos->rbp_offset);
    fprintf(file, "add"       , pos->rbp_offset);
    fprintf(file, "pop rbp"   , pos->rbp_offset);

    for(int i = 0; i < nargs; i++){
        fprintf(file, "pop [rbp + %d]\n", i);
    }
    fprintf(file, "call rax\n");

    fprintf(file, "push rbp\n");
    fprintf(file, "push %d \n", pos->rbp_offset);
    fprintf(file, "sub"       , pos->rbp_offset);
    fprintf(file, "pop rbp"   , pos->rbp_offset);
    fprintf(file, "#end of func call\n");
}

static bool compileCode(F_DEF_ARGS);

static bool compileCodeBlock(F_DEF_ARGS){
    if(!expr)
        return !req_val;
    if(expr->data.type != EXPR_OP || expr->data.op != EXPR_O_ENDL){
        return compileCode(F_ARGS(expr), req_val);
    }
    int code_block_id_t = 0;
    if(req_val){
        code_block_id_t = pos->code_block_id;
        pos->code_block_id = pos->lbl_id;
        (pos->lbl_id)++;
        printf("#begin of rcb %d\n", pos->code_block_id);
    }
    pos->lvl++;
    if(!compileCode(F_ARGS(expr), false))
        return false;
    programDescendLvl(objs, pos->lvl);
    pos->lvl--;
    if(req_val){
        pos->code_block_id = code_block_id_t;
        printf("eocb_%d:\n", pos->code_block_id);
        fprintf(file, "push rax\n");
    }
    return true;
}

static bool compileMathOp(F_DEF_ARGS){
    if(!req_val)
        return true;

    if (!compileCodeBlock(F_ARGS(expr->left), true))
        return false;

    if (!isExprOpUnary(expr->data.op)){
        pos->stack_size++;
        if (!compileCodeBlock(F_ARGS(expr->left), true))
            return false;
        pos->stack_size--;
    }


    switch(expr->data.op){
        case EXPR_MO_ADD:
            fprintf(file, "add\n");
            break;
        case EXPR_MO_SUB:
            fprintf(file, "sub\n");
            break;
        case EXPR_MO_MUL:
            fprintf(file, "mul\n");
            break;
        case EXPR_MO_DIV:
            fprintf(file, "div\n");
            break;
        case EXPR_MO_POW:
            fprintf(file, "pow\n");
            break;
        case EXPR_MO_SQRT:
            fprintf(file, "sqrt\n");
            break;
    }
    return true;
}



static bool compileSetVar(F_DEF_ARGS, bool create_vars = false){
    if(!(expr && expr->left && expr->right))
        return false;
    BinTreeNode* src = expr->right;
    BinTreeNode* dst = expr->left ;
    if(expr->data.op == EXPR_O_EQLTR){
        src = expr->left;
        dst = expr->right;
    }
    if(dst->data.type != EXPR_VAR){
        return false;
    }



    compileCodeBlock(F_ARGS(src), true);

    if(req_val){
        fprintf(file, "dup\n");
    }

    VFuncEntry* vfunc = vfuncTableGetRW(objs->vars, dst->data.name, true);
    if(vfunc){
        fprintf(file, "push %d\n", vfunc->value.addr);
        fprintf(file, "pop rax\n", vfunc->value.addr);
        compileFuncCall(file, pos, 1);
    }

    if(strcmp(dst->data.name, u8"▲") == 0){ //ᐃ{
        fprintf(file, "out\n");
        return true;
    }

    if(strcmp(dst->data.name, u8"🚪") == 0){
        fprintf(file, "pop rax\n");
        for(int i = 0; i < pos->stack_size; i++){
            fprintf(file, "pop\n");
        }
        fprintf(file, "jmp :eocb_%d\n", pos->code_block_id);
        return true;
    }

    VarEntry* var = create_vars ? varTableCreate(objs->vars, dst->data.name, pos->lvl) : varTableGet(objs->vars, dst->data.name);
    if(!var){
        return false;
    }
    fprintf(file, "pop [rbp + %d]\n", var->value);
    return true;
}

static bool compileCode(F_DEF_ARGS){
    if(!expr){
        return !req_val;
    }
    if(expr->data.type == EXPR_VAR){

        VFuncEntry* vfunc = vfuncTableGetRW(objs->vfuncs, expr->data.name, false);
        if(vfunc){
            fprintf(file, "push %d\n", vfunc->value.addr);
            fprintf(file, "pop rax\n", vfunc->value.addr);
            compileFuncCall(file, pos, 0);
            if(!req_val){
                fprintf(file, "pop\n");
            }
            return true;
        }
        if(req_val){
            VarEntry* var = varTableGet(objs->vars, expr->data.name);
            if(!var){
                error_log("Var \"%s\" not found\n", expr->data.name);
                return false;
            }
            fprintf(file, "push [rbp + %d]\n", varTableGet(objs->vars, expr->data.name)->value);
        }
        return true;
    }
    if(expr->data.type == EXPR_CONST){
        if(req_val){
            fprintf(file, "push %d\n", expr->data.val);
        }
        return true;
    }
    if(expr->data.type == EXPR_OP){
        //printf_log("%s " , mathOpName(expr->data.op));
        if(isMathOp(expr->data.op))
            return compileMathOp(F_ARGS(expr), false);
        int instr_lbl_n = pos->lbl_id;

        switch(expr->data.op){
        case EXPR_O_ENDL:
            if(!compileCode(F_ARGS(expr->left), false))
                return false;
            if(!compileCodeBlock(F_ARGS(expr->right), false))
                return false;
            return true;
        case EXPR_O_IF:
            if(!compileCodeBlock(F_ARGS(expr->left), true))
                return false;
            (pos->lbl_id)++;
            fprintf(file, "push 0\njeq :if_%d\n", instr_lbl_n);
            if(!compileCodeBlock(F_ARGS(expr->right), req_val))
                return false;
            fprintf(file, "if_%d:\n", instr_lbl_n);
            return true;
        case EXPR_O_WHILE:
            (pos->lbl_id)++;
            fprintf(file, "while_%d_beg:\n", instr_lbl_n);
            if(!compileCodeBlock(F_ARGS(expr->left), true))
                return false;
            fprintf(file, "push 0\njeq :while_%d_end\n", instr_lbl_n);
            if(!compileCodeBlock(F_ARGS(expr->right), false))
                return false;
            fprintf(file, "jmp :while_%d_beg\n", instr_lbl_n);
            fprintf(file, "while_%d_end:\n", instr_lbl_n);
            return true;
        case EXPR_O_EQRTL:
            return compileSetVar(F_ARGS(expr), req_val, false);
        case EXPR_O_EQLTR:
            return compileSetVar(F_ARGS(expr), req_val, false);
        case EXPR_O_VDEF:
            return compileSetVar(F_ARGS(expr->right), false, true);

        }

    }
    return false;
}

bool compileProgram(FILE* file, BinTreeNode* code){
    ProgramNameTable objs = {};
    ProgramPosData pos = {};
    programNameTableCtor(&objs);
    programPosDataCtor(&pos);

    return compileCodeBlock(file, code, &objs, &pos, false);

    programNameTableDtor(&objs);
}
