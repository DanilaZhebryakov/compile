#include "lib/bintree.h"
#include "expr/expr_elem.h"
#include "program/program_structure.h"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic push

#define F_ARGS(...) file, __VA_ARGS__ , objs, pos, regs, expr
#define F_DEF_ARGS FILE* file, BinTreeNode* expr , ProgramNameTable* objs, ProgramPosData* pos, RegInfo* regs, BinTreeNode* oexpr, bool req_val

#define ASM_OUT(...) \
fprintf(file, __VA_ARGS__)

static const int BASE_BUFF_SIZE = 1000;
static const int RBP_BASE_POS = 99;

#define REG_ADD_ARGS FILE* file , ProgramPosData* pos,
#define REG_ADD_ARGS_CALL file, pos,
#include "program/reg_info.h"

#define compilationError(...) \
{error_log(__VA_ARGS__);        \
printf_log("at:'");           \
printExprElem(stderr  , expr->data);\
printExprElem(_logfile, expr->data);\
printf_log("' (%s:%d:%d)\n", expr->data.file_name, expr->data.file_line, expr->data.line_pos);\
}

static void writeToVar(FILE* file, ProgramPosData* pos, int v_flvl, int addr, bool arr = false){
    if (v_flvl == pos->flvl){
        if (arr){
            ASM_OUT("push rbp\n");
            ASM_OUT("add\n");
            ASM_OUT("pop rax\n");
            ASM_OUT("pop [rax + %d]\n", addr);
        }
        else{
            ASM_OUT("pop [rbp + %d]\n", addr);
        }
    }
    else{
        ASM_OUT("push [%d]\n", RBP_BASE_POS - v_flvl);
        if (arr){
            ASM_OUT("add\n");
        }
        ASM_OUT("pop rax\n");
        ASM_OUT("pop [rax + %d]\n", addr);
    }
}
static void readFromVar(FILE* file, ProgramPosData* pos, int v_flvl, int addr, bool arr = false){
    if (v_flvl == pos->flvl){
        if (arr){
            ASM_OUT("push rbp\n");
            ASM_OUT("add\n");
            ASM_OUT("pop rax\n");
            ASM_OUT("push [rax + %d]\n", addr);
        }
        else{
            ASM_OUT("push [rbp + %d]\n", addr);
        }
    }
    else{
        ASM_OUT("push [%d]\n", RBP_BASE_POS - v_flvl);
        if (arr){
            ASM_OUT("add\n");
        }
        ASM_OUT("pop rax\n");
        ASM_OUT("push [rax + %d]\n", addr);
    }
}


static void unloadVarFromReg(REG_ADD_ARGS RegInfo* regs, int reg_n, bool write){
    assert_log(regs[reg_n].var);
    if(write){
        fprintf(file, "push r%d\n" , reg_n + REG_USE_FIRST);
        writeToVar(file, pos, regs[reg_n].var->fdepth, regs[reg_n].var->value);
    }
    regs[reg_n].var = nullptr;
    regs[reg_n].load_prog_lvl = -1;
    regs[reg_n].load_mem_n = 0;
}
static void loadVarToReg(REG_ADD_ARGS RegInfo* regs, int reg_n, VarEntry* var, bool read){
    ASM_OUT("#Load var %s to reg r%d\n", var->name, reg_n + REG_USE_FIRST);
    if(read){
        readFromVar(file, pos, var->fdepth, var->value);
        fprintf(file, "pop r%d\n" , reg_n + REG_USE_FIRST);
    }
    regs[reg_n].var = var;
}

static void compileVarRead(FILE* file, ProgramPosData* pos, RegInfo* regs, VarEntry* var){
    int reg_n = getRegWithVar(file, pos, regs, var);
    if(reg_n == -1){
        reg_n = findRegForVar(file, pos, regs, var, pos->lvl, true);
    }
    ASM_OUT("push r%d\n", reg_n + REG_USE_FIRST);
}


static void compileFuncCall(FILE* file, ProgramPosData* pos, RegInfo* regs, int nargs, bool ret_val, int func_flvl){
    regsDescendLvl(file, pos, regs, 0, true);
    ASM_OUT("#call of func\n");
    ASM_OUT("push rbp\n");
    ASM_OUT("push %d \n", pos->rbp_offset);
    ASM_OUT("add\n"       );
    ASM_OUT("pop rbp\n"   );

    for (int i = nargs-1; i >= 0; i--){// args are read backwards because stack
        if (i > REG_USE_CNT)
            ASM_OUT("pop [rbp + %d]\n", i);
        else
            ASM_OUT("pop r%d\n", i + REG_USE_FIRST);
    }

    //save old element of rbp level table to stack. Then overwrite.
    ASM_OUT("push [%d]\n", RBP_BASE_POS - func_flvl);
    ASM_OUT("push rbp\n" );
    ASM_OUT("pop [%d]\n" , RBP_BASE_POS - func_flvl);

    ASM_OUT("call rax\n");

    if(ret_val){
        ASM_OUT("swap\n");
    }
    ASM_OUT("pop [%d]\n", RBP_BASE_POS - func_flvl);
    ASM_OUT("push rbp\n");
    ASM_OUT("push %d \n", pos->rbp_offset);
    ASM_OUT("sub\n"       );
    ASM_OUT("pop rbp\n"   );
    ASM_OUT("#end of func call\n");
}

//two types of code blocks exist: RCB (returnable code block) (req_val = 1) and normal code block
//RCB puts a result on a stack. But ret operator puts result in rax
//ret operator jumps to the end of last RCB it is in (out of any non-rcb) (in top block it creates assembly error)

#define COMPILE_CODE_BLOCK(_req_val, ...)                       \
    {                                                           \
        ASM_OUT("#{\n");                                   \
        int code_block_id_t = 0;                                \
        if (_req_val){                                           \
            code_block_id_t = pos->code_block_id;               \
            pos->code_block_id = pos->lbl_id;                   \
            (pos->lbl_id)++;                                    \
            ASM_OUT("#begin of rcb %d\n", pos->code_block_id);   \
        }                                                       \
        pos->lvl++;                                             \
        __VA_ARGS__                                             \
        programDescendLvl(objs, pos, pos->lvl);                 \
        pos->lvl--;                                             \
        if (_req_val){                                           \
            ASM_OUT("eocb_%d:\n", pos->code_block_id);    \
            pos->code_block_id = code_block_id_t;               \
            ASM_OUT("push rax\n");                        \
        }                                                       \
        ASM_OUT("#}\n");                                   \
    }

#define CHECK(...)      \
    if(!(__VA_ARGS__))  \
        cmp_ok = false;

static bool compileCode(F_DEF_ARGS);

static bool compileCodeBlock(F_DEF_ARGS){
    bool cmp_ok = true;
    if (!expr){
        if(req_val){
            expr = oexpr;
            compilationError("Empty expression can not return value ");
        }
        return !req_val;
    }
    if (expr->data.type != EXPR_OP || expr->data.op != EXPR_O_ENDL){
        return compileCode(F_ARGS(expr), req_val);
    }
    COMPILE_CODE_BLOCK(req_val,
            CHECK(compileCode(F_ARGS(expr), false))
    )
    return cmp_ok;
}

static bool compileConditionJump(F_DEF_ARGS, bool inv, const char* lbl_id){
    bool cmp_ok = true;
    if (expr->data.type == EXPR_CONST){
        if ((expr->data.val != 0) ^ inv){
            ASM_OUT("jmp %s\n", lbl_id);
        }
        return cmp_ok;
    }
    if (expr->data.type == EXPR_OP){
        switch(expr->data.op){
        case EXPR_MO_CGT:
            CHECK(compileCodeBlock(F_ARGS(expr->left), true));
            CHECK(compileCodeBlock(F_ARGS(expr->right), true));
            if(inv){  ASM_OUT("jle :%s\n", lbl_id); }
            else{     ASM_OUT("jgt :%s\n", lbl_id); }
            return cmp_ok;
        case EXPR_MO_CLT:
            CHECK(compileCodeBlock(F_ARGS(expr->left), true));
            CHECK(compileCodeBlock(F_ARGS(expr->right), true));
            if(inv){  ASM_OUT("jge :%s\n", lbl_id); }
            else{     ASM_OUT("jlt :%s\n", lbl_id); }
            return cmp_ok;
        case EXPR_MO_CGE:
            CHECK(compileCodeBlock(F_ARGS(expr->left), true));
            CHECK(compileCodeBlock(F_ARGS(expr->right), true));
            if(inv){  ASM_OUT("jlt :%s\n", lbl_id); }
            else{     ASM_OUT("jge :%s\n", lbl_id); }
            return cmp_ok;
        case EXPR_MO_CLE:
            CHECK(compileCodeBlock(F_ARGS(expr->left), true));
            CHECK(compileCodeBlock(F_ARGS(expr->right), true));
            if(inv){  ASM_OUT("jgt :%s\n", lbl_id); }
            else{     ASM_OUT("jle :%s\n", lbl_id); }
            return cmp_ok;
        case EXPR_MO_CEQ:
            CHECK(compileCodeBlock(F_ARGS(expr->left), true));
            CHECK(compileCodeBlock(F_ARGS(expr->right), true));
            if(inv){  ASM_OUT("jne :%s\n", lbl_id); }
            else{     ASM_OUT("jeq :%s\n", lbl_id); }
            return cmp_ok;
        case EXPR_MO_CNE:
            CHECK(compileCodeBlock(F_ARGS(expr->left), true));
            CHECK(compileCodeBlock(F_ARGS(expr->right), true));
            if(inv){  ASM_OUT("jeq :%s\n", lbl_id); }
            else{     ASM_OUT("jne :%s\n", lbl_id); }
            return cmp_ok;
        default:
            break;
        }
    }
    CHECK(compileCodeBlock(F_ARGS(expr), true));
    ASM_OUT("push 0\n");
    if(inv){  ASM_OUT("jeq :%s\n", lbl_id); }
    else{     ASM_OUT("jne :%s\n", lbl_id); }
    return cmp_ok;
}

static int compileArgList(F_DEF_ARGS){
    if (!expr)
        return 0;
    if  (expr->data.type != EXPR_OP || expr->data.op != EXPR_O_ENDL){
        if (!compileCodeBlock(F_ARGS(expr), true))
            return -1;
        return 1;
    }
    printf("R");
    int r = compileArgList(F_ARGS(expr->left), true) + 1;
    if (!compileCodeBlock(F_ARGS(expr->right), true))
        return -1;
    printf("AL%d \n", r);
    return r;
}

static bool compileMathOp(F_DEF_ARGS){
    bool cmp_ok = true;
    if (!req_val)
        return cmp_ok;
    
    if ((!canExprOpBeUnary(expr->data.op)) && (!expr->left)){
        compilationError("Binary-only op used as unary");
    }
    if ((isExprOpUnary(expr->data.op)) && (expr->left)){
        compilationError("Unary-only op used as binary");
    }

    if (expr->left){
        pos->stack_size++;
        CHECK(compileCodeBlock(F_ARGS(expr->left), true))
        pos->stack_size--;
    }
    CHECK(compileCodeBlock(F_ARGS(expr->right), true))

    switch(expr->data.op){
        case EXPR_MO_ADD:
            ASM_OUT("add\n");
            break;
        case EXPR_MO_SUB:
            ASM_OUT("sub\n");
            break;
        case EXPR_MO_MUL:
            ASM_OUT("mul\n");
            break;
        case EXPR_MO_DIV:
            ASM_OUT("div\n");
            break;
        case EXPR_MO_POW:
            ASM_OUT("pow\n");
            break;
        case EXPR_MO_SQRT:
            ASM_OUT("sqrt\n");
            break;
        case EXPR_MO_TANP:
            ASM_OUT("tan\n");
            break;
        case EXPR_MO_BOOL:
            ASM_OUT("bool\n");
            break;
        case EXPR_MO_BOR:
            ASM_OUT("or\n");
            break;
        case EXPR_MO_BAND:
            ASM_OUT("and\n");
            break;
        case EXPR_MO_BXOR:
            ASM_OUT("xor\n");
            break;
        case EXPR_MO_BSL:
            ASM_OUT("bsl\n");
            break;
        case EXPR_MO_BSR:
            ASM_OUT("bsr\n");
            break;
        case EXPR_MO_UMIN:
            ASM_OUT("push 0\n");
            ASM_OUT("swap\n");
            ASM_OUT("sub\n");
            break;
        default:
            compilationError("Bad math op");
            return false;
    }
    return cmp_ok;
}

static bool compileSetDst(F_DEF_ARGS, bool create_vars = false, bool arr = false, int arlen = 1){
    bool cmp_ok = true;
    if (expr->data.type == EXPR_OP){
        if (expr->data.op == EXPR_O_COMMA){
            if(arr){
                ASM_OUT("pop rax\n");
            }
            ASM_OUT("dup\n");
            if(arr){
                ASM_OUT("push rax\n");
                ASM_OUT("swap\n");
                ASM_OUT("push rax\n");
                pos->stack_size++;
            }
            pos->stack_size++;
            CHECK( compileSetDst(F_ARGS(expr->left), false  , create_vars, arr, arlen) )
            pos->stack_size--;
            if(arr){
                pos->stack_size--;
            }
            CHECK( compileSetDst(F_ARGS(expr->right), req_val, create_vars, arr, arlen) )
            return cmp_ok;
        }
        if (expr->data.op == EXPR_O_IF){
            char t_lbl[30] = "";
            int if_lbl_id = pos->lbl_id;
            (pos->lbl_id)++;
            regsDescendLvl(file, pos, regs, 0, true);
            sprintf(t_lbl, "a_if_else_%d\n", if_lbl_id);
            CHECK(compileConditionJump(F_ARGS(expr), false, true, t_lbl));

            if (expr->right->data.type == EXPR_OP && expr->right->data.op == EXPR_O_SEP){
                CHECK( compileSetDst(F_ARGS(expr->right->left ), req_val, create_vars, arr, arlen) )
                regsDescendLvl(file, pos, regs, 0, true);
                ASM_OUT("jmp :a_if_end_%d\n" , if_lbl_id);

                ASM_OUT("a_if_else_%d:\n", if_lbl_id);
                CHECK( compileSetDst(F_ARGS(expr->right->right), req_val, create_vars, arr, arlen) )
                regsDescendLvl(file, pos, regs, 0, true);
                ASM_OUT("a_if_end_%d:\n" , if_lbl_id);
            }

            else {
                CHECK( compileSetDst(F_ARGS(expr->right), req_val, create_vars, arr, arlen) );
                ASM_OUT("jmp :a_if_end_%d\n", if_lbl_id);

                ASM_OUT("a_if_else_%d:\n", if_lbl_id);
                if(!req_val){
                    ASM_OUT("pop rnn\n");
                    if(arr)
                        ASM_OUT("pop rnn\n");
                }
                regsDescendLvl(file, pos, regs, 0, true);
                ASM_OUT("a_if_end_%d:\n", if_lbl_id);
            }

            return cmp_ok;
        }
        if (expr->data.op == EXPR_O_ARDEF){
            if (!expr->right){
                compilationError("For now, you should always specify the length of created array");
                return false;
            }
            if (expr->right->data.type != EXPR_CONST){
                compilationError("Arrays of variable length not supported");
                return false;
            }
            CHECK(compileSetDst(F_ARGS(expr->left), req_val, create_vars, arr, expr->right->data.val));
            return cmp_ok;
        }
        if (expr->data.op == EXPR_O_ARIND){
            if (arr){
                compilationError("Multidimensional arrays not supported");
                return false;
            }
            if (!expr->right){
                compilationError("Index in array should be specified");
                return false;
            }
            CHECK(compileCodeBlock(F_ARGS(expr->right), true));
            CHECK(compileSetDst(F_ARGS(expr->left), req_val, create_vars, true, arlen));
            return cmp_ok;
        }
    }

    if(expr->data.type == EXPR_KVAR){
        if(arr){ // kvars as well as vfuncs ignore array indexes
          ASM_OUT("pop rnn\n");
        }
        switch(expr->data.kword){
        case EXPR_KW_RET:
            ASM_OUT("pop rax\n");
            for (int i = 0; i < pos->stack_size; i++){
                ASM_OUT("pop rnn\n");
            }
            ASM_OUT("jmp :eocb_%d\n", pos->code_block_id);
            break;
        case EXPR_KW_NIO:
            ASM_OUT("out\n");
            if (req_val)
                ASM_OUT("inp\n");
            break;
        case EXPR_KW_CIO:
            ASM_OUT("outch\n");
            if (req_val)
                ASM_OUT("inpch\n");
            break;
        case EXPR_KW_BAD:
            ASM_OUT("bad\n");
            return cmp_ok;
        case EXPR_KW_HALT:
            ASM_OUT("halt\n");
            return cmp_ok;
        case EXPR_KW_NULL:
            ASM_OUT("pop rnn\n");
            if (req_val)
                ASM_OUT("push 0\n");
            return cmp_ok;
        default:
            compilationError("Unknown kvar for write: %s\n", getExprKWName(expr->data.kword));
            return false;
        }
        return cmp_ok;
    }

    if(expr->data.type == EXPR_VAR){
        VFuncEntry* vfunc = vfuncTableGetRW(objs->vfuncs, expr->data.name, true);
        if (vfunc){
            if(arr){ // kvars as well as vfuncs ignore array indexes
                ASM_OUT("pop rnn\n");
            }
            ASM_OUT("push :%s\n", vfunc->value.lbl);
            ASM_OUT("pop rax\n");
            compileFuncCall(file, pos, regs, 1, false, vfunc->fdepth);
            if (req_val)
                compileCode(F_ARGS(expr), true);
        }
        else{
            if(arr){
                create_vars = false;
            }
            if (req_val){
                ASM_OUT("dup\n");
                pos->stack_size++;
            }
            if (create_vars){
                programCreateVar(objs, pos, expr->data.name, arlen);
            }
            VarEntry* var = varTableGet(objs->vars, expr->data.name);
            if (!var){
                compilationError("Assignment var %s not found\n", expr->data.name);
                return false;
            }
            if (arr){
                int reg_n = getRegWithVar(file, pos, regs, var);
                if(reg_n != -1){
                    unloadVarFromReg(file, pos, regs, reg_n, true);
                }
                writeToVar(file, pos, var->fdepth, var->value, true);
            }
            else{
                int reg_n = getRegWithVar(file, pos, regs, var);
                if(reg_n == -1){
                    reg_n = findRegForVar(file, pos, regs, var, pos->lvl, !create_vars);
                }

                ASM_OUT("pop r%d\n", reg_n + REG_USE_FIRST);
            }
        }
        return cmp_ok;
    }
    compilationError("Invalid elem for write");
    return false;
}

static int compileSetArray(F_DEF_ARGS, BinTreeNode* dst, bool create_vars, int ind){
    bool cmp_ok = true;
    if (!expr)
        return 0;
    if (expr->data.type == EXPR_STLIT){
        int i = 0;
        while(expr->data.name[i] != '\0'){
            ASM_OUT("push '%c'\n", expr->data.name[i]);
            if(ind + i != 0){
                ASM_OUT("push %d\n"  , ind + i);
            }
            compileSetDst(F_ARGS(dst), false, create_vars, ind + i != 0);
            i++;
        }
        return i;
    }
    if  (expr->data.type == EXPR_OP && expr->data.op == EXPR_O_ARDEF){
        if(!expr->right || expr->right->data.type != EXPR_CONST){
            compilationError("Arraify operator requires number as right arg");
            return -1;
        }
        for(int i = 0; i < expr->right->data.val; i++){
            if(expr->left && expr->left->data.type == EXPR_VAR){
                VarEntry* var = varTableGet(objs->vars, expr->left->data.name);
                if(!var){
                    compilationError("Var \"%s\" not found", expr->data.name);
                    return false;
                }
                int regn = getRegWithVar(file, pos, regs, var);
                if(regn != -1){
                    unloadVarFromReg(file, pos, regs, regn, true);
                }
                readFromVar(file, pos, var->fdepth, var->value + i);
            }
            else{
                CHECK(compileCodeBlock(F_ARGS(expr->left), true));
            }
            if(ind + i != 0){
                ASM_OUT("push %d\n"  , ind + i);
            }
            compileSetDst(F_ARGS(dst), false, create_vars, ind + i != 0);
        }
        return expr->right->data.val;
    }
    if  (expr->data.type == EXPR_OP && expr->data.op == EXPR_O_COMMA){
        int t = compileSetArray(F_ARGS(expr->left), true, dst, create_vars, ind);
        if(t == -1){
            return -1;
        }
        ind += t;
        t = compileSetArray(F_ARGS(expr->right), true, dst, create_vars, ind);
        if(t == -1){
            return -1;
        }
        ind += t;
        return ind;
    }

    if (!compileCodeBlock(F_ARGS(expr), true))
        return -1;

    if(ind != 0){
        ASM_OUT("push %d\n", ind);
    }
    compileSetDst(F_ARGS(dst), false, create_vars, ind != 0);
    return 1;
}

static bool compileSetVar(F_DEF_ARGS, bool create_vars = false){
    bool cmp_ok = true;
    if (!expr){
        compilationError("Empty var definition not allowed");
        return false;
    }
    if (expr->data.type != EXPR_OP){
        compilationError("Bad elem in var assignment");
        return false;
    }
    if (expr->data.op == EXPR_O_COMMA){
        CHECK(compileSetVar(F_ARGS(expr->left), false  , create_vars))
        CHECK(compileSetVar(F_ARGS(expr->left), req_val, create_vars))
        return cmp_ok;
    }

    if (!(expr->left && expr->right)){
        compilationError("Assignment should have both L and R values");
        return false;
    }

    BinTreeNode* src = expr->right;
    BinTreeNode* dst = expr->left ;
    if(expr->data.op == EXPR_O_EQLTR){
        src = expr->left;
        dst = expr->right;
    }

    if ((src->data.type == EXPR_OP && (src->data.op == EXPR_O_COMMA || src->data.op == EXPR_O_ARDEF)) || src->data.type == EXPR_STLIT){
        if(req_val){
            compilationError("assignment returning value not supported for arrays");
            return false;
        }
        ASM_OUT("#aset:\n");
        if(compileSetArray(F_ARGS(src), false, dst, create_vars, 0) == -1){
            compilationError("array error");
            return false;
        }
        ASM_OUT("#Easet:\n");
        return cmp_ok;
    }
    else{
        ASM_OUT("#src:\n");
        CHECK(compileCodeBlock(F_ARGS(src), true))
        ASM_OUT("#Esrc:\n");
        CHECK(compileSetDst(F_ARGS(dst), req_val, create_vars));
        return cmp_ok;
    }

}

static bool compileVarDef(F_DEF_ARGS){
    bool cmp_ok = true;
    if (!expr){
        return cmp_ok;
    }
    if (expr->data.type == EXPR_VAR){
        programCreateVar(objs, pos, expr->data.name);
        int regn = getFreeReg(file, pos, regs);
        if (regn != -1){// if there is free reg -> 'load', is not -> do nothing
            loadVarToReg(file, pos, regs, regn, varTableGetLast(objs->vars), false);
            regs[regn].load_prog_lvl = pos->lvl;
            regs[regn].load_mem_n = 1;
        }
        return cmp_ok;
    }
    if (expr->data.type == EXPR_OP){
        if (isAssignOp(expr->data.op)){
            return compileSetVar(F_ARGS(expr), false, true);
        }
        if (expr->data.op == EXPR_O_COMMA){
            CHECK(compileVarDef(F_ARGS(expr->left) , false));
            CHECK(compileVarDef(F_ARGS(expr->right), false));
            return cmp_ok;
        }
    }

    compilationError("Invalid elem for var def");
    return false;
}

static bool compileFuncDef(F_DEF_ARGS){
    bool cmp_ok = true;
    bool var_func = expr->data.op == EXPR_O_VFDEF;
    if (!(expr->left && expr->left->data.type == EXPR_VAR)){
        compilationError("Defined function has shit instead of name")
        return false;
    }

    pos->flvl++;
    int old_offset = pos->rbp_offset;
    pos->rbp_offset = 0;

    RegInfo new_regs[16] = {}; // function def does not change registers, but contained code does
    regs = new_regs;

    char name_buff[BASE_BUFF_SIZE] = "";
    sprintf(name_buff, "%s_%s_%d",var_func ? "v-function": "function", expr->left->data.name, pos->lbl_id);
    pos->lbl_id++;
    ASM_OUT("jmp :%s*skip\n", name_buff);
    ASM_OUT("%s:\n", name_buff);
    char* name_str = strdup(name_buff);
    programAddNewMem(pos, name_str);

    if (!(expr->right && expr->right->data.type == EXPR_OP && expr->right->data.op == EXPR_O_SEP)){
        if (var_func){
            vfuncTablePut(objs->vfuncs, {{name_str, false}, (expr->left->data.name), (pos->lvl), (pos->flvl)});
            COMPILE_CODE_BLOCK( true, //force-apply RCB
                CHECK(compileCode(F_ARGS(expr->right), true))
            )

        }
        else{
            compilationError("normal function needs to be defined with <sep> even with no args");
            return false;
        }
    }
    else{
        if (var_func)
            vfuncTablePut(objs->vfuncs, {{name_str, false}, (expr->left->data.name), (pos->lvl), (pos->flvl)});
        else
            funcTablePut(objs->funcs  , {{name_str}       , (expr->left->data.name), (pos->lvl), (pos->flvl)});

        // function with its arguments is just compiled into one returnable code block
        COMPILE_CODE_BLOCK( true,
            CHECK(compileCode(F_ARGS(expr->right->left), false))
            CHECK(compileCode(F_ARGS(expr->right->right), false))
        )
    }
    pos->rbp_offset = old_offset;
    ASM_OUT("swap\n");// function code, being RCB puts a result on a stack. Return adress is under it
    ASM_OUT("ret\n");
    ASM_OUT("%s*skip:\n", name_buff);
    pos->flvl--;
    return cmp_ok;
}

static bool compileCode(F_DEF_ARGS){
    bool cmp_ok = true;
    if(!expr){
        if(req_val){
            expr = oexpr;
            compilationError("Empty expression can not return a value\n");
        }
        return !req_val;
    }
    if(expr->data.type == EXPR_STAND){
        compilationError("💩СТАНДАРТ--ГОВНО💩");
        return false;
    }
    if(expr->data.type == EXPR_VAR){
        VFuncEntry* vfunc = vfuncTableGetRW(objs->vfuncs, expr->data.name, false);
        if(vfunc){
            ASM_OUT("push :%s\n", vfunc->value.lbl);
            ASM_OUT("pop rax\n");
            compileFuncCall(file, pos, regs, 0, true, vfunc->fdepth);
            if(!req_val){
                ASM_OUT("pop rnn\n");
            }
            return cmp_ok;
        }
        if(req_val){
            VarEntry* var = varTableGet(objs->vars, expr->data.name);
            if(!var){
                compilationError("Var \"%s\" not found %d", expr->data.name);
                return false;
            }
            compileVarRead(file, pos, regs, var);
        }
        return cmp_ok;
    }
    if(expr->data.type == EXPR_KVAR){
        switch(expr->data.kword){
        case EXPR_KW_RET:
            compilationError("Return with no value NYI");
            return false;
        case EXPR_KW_NIO:
            if(req_val)
                ASM_OUT("inp\n");
            return cmp_ok;
        case EXPR_KW_CIO:
            if(req_val)
                ASM_OUT("inpch\n");
            return cmp_ok;
        case EXPR_KW_BAD:
            ASM_OUT("bad\n");
            return cmp_ok;
        case EXPR_KW_HALT:
            ASM_OUT("halt\n");
            return cmp_ok;
        case EXPR_KW_NULL:
            ASM_OUT("push 0\n");
            return cmp_ok;
        default:
            compilationError("Unknown kvar for write: %s", getExprKWName(expr->data.kword));
            return false;
        }
    }
    if(expr->data.type == EXPR_FUNC){
        int nargs = compileArgList(F_ARGS(expr->right), true);
        if(nargs == -1){
            compilationError("Function arg list error");
            return false;
        }
        FuncEntry* func = funcTableGet(objs->funcs, expr->data.name);
        if(!func){
            compilationError("Function %s not found", expr->data.name);
            return false;
        }
        ASM_OUT("push :%s\n", func->value.lbl);
        ASM_OUT("pop rax\n");
        compileFuncCall(file, pos, regs, nargs, true, func->fdepth);
        if(!req_val)
            ASM_OUT("pop rnn\n");
        return cmp_ok;
    }
    if(expr->data.type == EXPR_CONST){
        if(req_val){
            ASM_OUT("push %d\n", int(expr->data.val));
        }
        return cmp_ok;
    }
    if(expr->data.type == EXPR_OP){
        //printf_log("%s " , exprOpName(expr->data.op));
        if(isMathOp(expr->data.op))
            return compileMathOp(F_ARGS(expr), true);
        int instr_lbl_n = pos->lbl_id;

        int t = 0;
        VarEntry* var = nullptr;
        char t_lbl [30] = "";
        switch(expr->data.op){
        case EXPR_O_ENDL:
            CHECK( compileCode     (F_ARGS(expr->left ), false) )
            CHECK( compileCodeBlock(F_ARGS(expr->right), false) )
            return cmp_ok;

        case EXPR_O_IF:
            (pos->lbl_id)++;
            if(!expr->right){
                compilationError("'if' should have both L and R values");
                return false;
            }
            regsDescendLvl(file, pos, regs, 0, true);
            if(expr->right->data.type == EXPR_OP && expr->right->data.op == EXPR_O_SEP){

                sprintf(t_lbl, "if_else_%d", instr_lbl_n);
                CHECK(compileConditionJump(F_ARGS(expr->left),false , true, t_lbl));

                CHECK( compileCodeBlock(F_ARGS(expr->right->left ), req_val) )
                regsDescendLvl(file, pos, regs, 0, true);
                ASM_OUT("jmp :if_end_%d\n" , instr_lbl_n);

                ASM_OUT("if_else_%d:\n", instr_lbl_n);
                CHECK( compileCodeBlock(F_ARGS(expr->right->right), req_val) )
                regsDescendLvl(file, pos, regs, 0, true);
                ASM_OUT("if_end_%d:\n" , instr_lbl_n);
            }
            else{

                if(req_val){
                    compilationError("if with no else can not return a value");
                    return false;
                }

                sprintf(t_lbl, "if_end_%d", instr_lbl_n);
                CHECK(compileConditionJump(F_ARGS(expr->left),false , true, t_lbl));

                CHECK( compileCodeBlock(F_ARGS(expr->right), false) )
                regsDescendLvl(file, pos, regs, 0, true);
                ASM_OUT("if_end_%d:\n", instr_lbl_n);
            }
            return cmp_ok;

        case EXPR_O_WHILE:
            (pos->lbl_id)++;
            if(req_val)
                ASM_OUT("push 0\n");
            regsDescendLvl(file, pos, regs, 0, true);

            ASM_OUT("while_%d_beg:\n", instr_lbl_n);
            if(req_val)
                ASM_OUT("pop rnn\n");
            sprintf(t_lbl, "while_%d_end", instr_lbl_n);
            CHECK(compileConditionJump(F_ARGS(expr->left),false , true, t_lbl));

            CHECK( compileCodeBlock(F_ARGS(expr->right), req_val) )
            regsDescendLvl(file, pos, regs, 0, true);
            ASM_OUT("jmp :while_%d_beg\n", instr_lbl_n);
            ASM_OUT("while_%d_end:\n", instr_lbl_n);

            regsDescendLvl(file, pos, regs, 0, true);
            return cmp_ok;

        case EXPR_O_EQRTL:
            return compileSetVar(F_ARGS(expr), req_val, false);

        case EXPR_O_EQLTR:
            return compileSetVar(F_ARGS(expr), req_val, false);

        case EXPR_O_VDEF:
            if (req_val){
                compilationError("var definition can not return a value");
                return false;
            }
            CHECK(compileVarDef(F_ARGS(expr->right), false));
            return cmp_ok;

        case EXPR_O_FDEF:
            if (req_val){
                compilationError("function definition can not return a value");
                return false;
            }
            CHECK(compileFuncDef(F_ARGS(expr), false));
            return cmp_ok;

        case EXPR_O_VFDEF:
            if (req_val){
                compilationError("v-function definition can not return a value");
                return false;
            }
            CHECK(compileFuncDef(F_ARGS(expr), false));
            return cmp_ok;

        case EXPR_O_ARIND:
            if (!req_val)
                return cmp_ok;
            if (!expr->left || expr->left->data.type != EXPR_VAR){
                compilationError("Array index read only works on vars");
            }
            CHECK(compileCodeBlock(F_ARGS(expr->right), true));
            var = varTableGet(objs->vars, expr->left->data.name);
            if(!var){
                compilationError("Var \"%s\" not found", expr->data.name);
                return false;
            }
            t = getRegWithVar(file, pos, regs, var);
            if(t != -1){
                unloadVarFromReg(file, pos, regs, t, true);
            }
            readFromVar(file, pos, var->fdepth, var->value, true);
            return cmp_ok;

        default:
            compilationError("Unknown op for code");
            return false;
        }

    }
    compilationError("Unknown elem for code");
    return false;
}
#pragma GCC diagnostic pop


bool compileProgram(FILE* file, BinTreeNode* code){
    ProgramNameTable objs = {};
    ProgramPosData pos = {};
    programNameTableCtor(&objs);
    programPosDataCtor(&pos);

    ASM_OUT("push 0\n");
    ASM_OUT("pop rbp\n");
    ASM_OUT("push 0\n");
    ASM_OUT("pop [%d]\n", RBP_BASE_POS);
    RegInfo regs[14] = {};

    bool ret = compileCodeBlock(file, code, &objs, &pos, regs, code, false);
    if (!ret){
        error_log("Compiltion failed");
    }


    programPosDataDtor(&pos);
    programNameTableDtor(&objs);
    return ret;
}
