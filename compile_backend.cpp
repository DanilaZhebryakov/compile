#include "lib/bintree.h"
#include "expr/expr_elem.h"
#include "program/program_structure.h"

#define F_ARGS(...) file, __VA_ARGS__ , objs, pos, regs
#define F_DEF_ARGS FILE* file, BinTreeNode* expr , ProgramNameTable* objs, ProgramPosData* pos, RegInfo* regs, bool req_val

#define ASM_OUT(...) \
fprintf(file, __VA_ARGS__)

static const int BASE_BUFF_SIZE = 1000;
static const int RBP_BASE_POS = 99;

#define REG_ADD_ARGS FILE* file , ProgramPosData* pos,
#define REG_ADD_ARGS_CALL file, pos,
#include "program/reg_info.h"

static void writeToVar(FILE* file, ProgramPosData* pos, int v_flvl, int addr, bool arr = false){
    if(v_flvl == pos->flvl){
        if(arr){
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
        if(arr){
            ASM_OUT("add\n");
        }
        ASM_OUT("pop rax\n");
        ASM_OUT("pop [rax + %d]\n", addr);
    }
}
static void readFromVar(FILE* file, ProgramPosData* pos, int v_flvl, int addr, bool arr = false){
    if(v_flvl == pos->flvl){
        if(arr){
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
        if(arr){
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
        return false;

static bool compileCode(F_DEF_ARGS);

static bool compileCodeBlock(F_DEF_ARGS){
    if (!expr)
        return !req_val;
    if (expr->data.type != EXPR_OP || expr->data.op != EXPR_O_ENDL){
        return compileCode(F_ARGS(expr), req_val);
    }
    COMPILE_CODE_BLOCK(req_val,
            if(!compileCode(F_ARGS(expr), false))
                return false;
    )
    return true;
}

static int compileArgList(F_DEF_ARGS){
    if (!expr)
        return 0;
    if  (expr->data.type != EXPR_OP || expr->data.op != EXPR_O_ENDL){
        if (!compileCodeBlock(F_ARGS(expr), true))
            return -1;
        return 1;
    }
    if (!compileCodeBlock(F_ARGS(expr->left), true))
        return -1;
    return compileArgList(F_ARGS(expr->right), true) + 1;
}

static bool compileMathOp(F_DEF_ARGS){
    if (!req_val)
        return true;

    if (!compileCodeBlock(F_ARGS(expr->right), true))
        return false;

    if (!isExprOpUnary(expr->data.op)){
        pos->stack_size++;
        if (!compileCodeBlock(F_ARGS(expr->left), true))
            return false;
        pos->stack_size--;
    }

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
        default:
            error_log("Bad math op: ");
            printExprElem(stdout, expr->data);
            return false;
    }
    return true;
}

static bool compileSetDst(F_DEF_ARGS, bool create_vars = false, bool arr = false){

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
            CHECK( compileSetDst(F_ARGS(expr->left), false  , create_vars, arr) )
            pos->stack_size--;
            if(arr){
                pos->stack_size--;
            }
            CHECK( compileSetDst(F_ARGS(expr->right), req_val, create_vars, arr) )
            return true;
        }
        if (expr->data.op == EXPR_O_IF){
            CHECK( compileCodeBlock(F_ARGS(expr->left), true) )
            ASM_OUT("push 0\n");
            int if_lbl_id = pos->lbl_id;
            (pos->lbl_id)++;
            regsDescendLvl(file, pos, regs, 0, true);
            if (expr->right->data.type == EXPR_OP && expr->right->data.op == EXPR_O_SEP){

                ASM_OUT("jne :a_if_else_%d\n", if_lbl_id);
                CHECK( compileSetDst(F_ARGS(expr->right->left ), req_val, create_vars, arr) )
                ASM_OUT("jmp :a_if_end_%d\n" , if_lbl_id);
                regsDescendLvl(file, pos, regs, 0, true);
                ASM_OUT("a_if_else_%d:\n", if_lbl_id);
                CHECK( compileSetDst(F_ARGS(expr->right->right), req_val, create_vars, arr) )
                ASM_OUT("a_if_end_%d:\n" , if_lbl_id);
            }
            else {
                ASM_OUT("jne :a_if_else_%d\n", if_lbl_id);
                CHECK( compileSetDst(F_ARGS(expr->right), req_val, create_vars, arr) )
                ASM_OUT("jmp :a_if_end_%d\n", if_lbl_id);

                ASM_OUT("a_if_else_%d:\n", if_lbl_id);
                if(!req_val){
                    ASM_OUT("pop rnn\n");
                    if(arr)
                        ASM_OUT("pop rnn\n");
                }
                ASM_OUT("a_if_end_%d:\n", if_lbl_id);
            }
            regsDescendLvl(file, pos, regs, 0, true);
            return true;
        }
        //return false;
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
            return true;
        case EXPR_KW_HALT:
            ASM_OUT("halt\n");
            return true;
        case EXPR_KW_NULL:
            ASM_OUT("pop rnn\n");
            if (req_val)
                ASM_OUT("push 0\n");
            return true;
        default:
            error_log("Unknown kvar for write: %s\n", getExprKWName(expr->data.kword));
            return false;
        }
        return true;
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
            if (req_val){
                ASM_OUT("dup\n");
                pos->stack_size++;
            }
            if (create_vars){
                programCreateVar(objs, pos, expr->data.name);
            }
            VarEntry* var = varTableGet(objs->vars, expr->data.name);
            if (!var){
                error_log("Assignment var %s not found\n", expr->data.name);
                return false;
            }
            if (arr){
                writeToVar(file, pos, var->fdepth, var->value, true);
            }
            else{
                int reg_n = findRegForVar(file, pos, regs, var, pos->lvl, false);
                ASM_OUT("pop r%d\n", reg_n + REG_USE_FIRST);
            }
        }
        return true;
    }
    error_log("Invalid elem for write:");
    printExprElem(_logfile, expr->data);
    printExprElem(stderr  , expr->data);
    printf_log("\n");
    return false;
}

static int compileSetArray(F_DEF_ARGS, BinTreeNode* dst, int ind){
    if (!expr)
        return 0;
    if (expr->data.type == EXPR_STLIT){
        int i = 0;
        while(expr->data.name[i] != '\0'){
            ASM_OUT("push '%c'\n", expr->data.name[i]);
            ASM_OUT("push %d\n"  , ind);
            compileSetDst(F_ARGS(dst), false, false, true);
            i++;
        }
        return i;
    }
    if  (expr->data.type == EXPR_OP || expr->data.op == EXPR_O_COMMA){
        int t = compileSetArray(F_ARGS(expr->left), true, dst, ind);
        if(t == -1){
            return -1;
        }
        ind += t;
        t = compileSetArray(F_ARGS(expr->right), true, dst, ind);
        if(t == -1){
            return -1;
        }
        ind += t;
        return ind;
    }

    if (!compileCodeBlock(F_ARGS(expr), true))
        return -1;
    ASM_OUT("push %d\n", ind);
    compileSetDst(F_ARGS(dst), false, false, true);
    return 1;
}

static bool compileSetVar(F_DEF_ARGS, bool create_vars = false){
    if (!expr){
        error_log("Empty var definition not allowed\n");
        return false;
    }
    if (expr->data.type != EXPR_OP){
        error_log("Bad elem ");
        printExprElem(stdout  , expr->data);
        printExprElem(_logfile, expr->data);
        printf_log("in var assignment\n");
        return false;
    }
    if (expr->data.op == EXPR_O_COMMA){
        if (!compileSetVar(F_ARGS(expr->left), false  , create_vars))
            return false;
        if (!compileSetVar(F_ARGS(expr->left), req_val, create_vars))
            return false;
        return true;
    }

    if (!(expr->left && expr->right)){
        error_log("Assignment should have both L and R values\n");
        return false;
    }

    BinTreeNode* src = expr->right;
    BinTreeNode* dst = expr->left ;
    if(expr->data.op == EXPR_O_EQLTR){
        src = expr->left;
        dst = expr->right;
    }

    if (src->data.type == EXPR_OP && src->data.op == EXPR_O_COMMA){
        if(req_val){
            error_log("assignment returning value not supported for arrays\n");
            return false;
        }
        ASM_OUT("#aset:\n");
        if(compileSetArray(F_ARGS(src), false, dst, 0) == -1){
            error_log("array error\n");
            return false;
        }
        ASM_OUT("#Easet:\n");
        return true;
    }
    else{
        ASM_OUT("#src:\n");
        if(!compileCodeBlock(F_ARGS(src), true)){
            return false;
        }
        ASM_OUT("#Esrc:\n");
        return compileSetDst(F_ARGS(dst), req_val, create_vars);
    }

}

static bool compileVarDef(F_DEF_ARGS){
    if (!expr){
        return true;
    }
    if (expr->data.type == EXPR_VAR){
        programCreateVar(objs, pos, expr->data.name);
        int regn = getFreeReg(file, pos, regs);
        if (regn != -1){// if there is free reg -> 'load', is not -> do nothing
            loadVarToReg(file, pos, regs, regn, varTableGetLast(objs->vars), false);
            regs[regn].load_prog_lvl = pos->lvl;
            regs[regn].load_mem_n = 1;
        }
        return true;
    }
    if (expr->data.type == EXPR_OP){
        if (isAssignOp(expr->data.op)){
            return compileSetVar(F_ARGS(expr), false, true);
        }
        if (expr->data.op == EXPR_O_COMMA){
            CHECK(compileVarDef(F_ARGS(expr->left) , false));
            CHECK(compileVarDef(F_ARGS(expr->right), false));
            return true;
        }
    }

    error_log("Invalid elem for var def:");
    printExprElem(_logfile, expr->data);
    printExprElem(stderr  , expr->data);
    printf_log("\n");
    return true;
}

static bool compileFuncDef(F_DEF_ARGS){
    bool var_func = expr->data.op == EXPR_O_VFDEF;

    if(!(expr->left && expr->left->data.type == EXPR_VAR))
        return false;

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
                if (!compileCode(F_ARGS(expr->right), true))
                    return false;
            )

        }
        else{
            error_log("normal function needs to be defined with <sep> even with no args\n");
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
            if (!compileCode(F_ARGS(expr->right->left), false))
                return false;
            if (!compileCode(F_ARGS(expr->right->right), false))
                return false;
        )
    }
    pos->rbp_offset = old_offset;
    ASM_OUT("swap\n");// function code, being RCB puts a result on a stack. Return adress is under it
    ASM_OUT("ret\n");
    ASM_OUT("%s*skip:\n", name_buff);
    pos->flvl--;
    return true;
}

static bool compileCode(F_DEF_ARGS){
    if(!expr){
        if(req_val){
            error_log("Empty expression can not return a value\n");
        }
        return !req_val;
    }
    if(expr->data.type == EXPR_STAND){
        error_log("СТАНДАРТ--ГОВНО\n");
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
            return true;
        }
        if(req_val){
            VarEntry* var = varTableGet(objs->vars, expr->data.name);
            if(!var){
                error_log("Var \"%s\" not found\n", expr->data.name);
                return false;
            }
            compileVarRead(file, pos, regs, var);
        }
        return true;
    }
    if(expr->data.type == EXPR_KVAR){
        switch(expr->data.kword){
        case EXPR_KW_RET:
            error_log("Return with no value NYI\n");
            return false;
        case EXPR_KW_NIO:
            if(req_val)
                ASM_OUT("inp\n");
            return true;
        case EXPR_KW_CIO:
            if(req_val)
                ASM_OUT("inpch\n");
            return true;
        case EXPR_KW_BAD:
            ASM_OUT("bad\n");
            return true;
        case EXPR_KW_HALT:
            ASM_OUT("halt\n");
            return true;
        case EXPR_KW_NULL:
            ASM_OUT("push 0\n");
            return true;
        default:
            error_log("Unknown kvar for write: %s\n", getExprKWName(expr->data.kword));
            return false;
        }
    }
    if(expr->data.type == EXPR_FUNC){
        int nargs = compileArgList(F_ARGS(expr->right), true);
        if(nargs == -1){
            error_log("Function arg list error\n");
            return false;
        }
        FuncEntry* func = funcTableGet(objs->funcs, expr->data.name);
        if(!func){
            error_log("Function %s not found\n", expr->data.name);
            return false;
        }
        ASM_OUT("push :%s\n", func->value.lbl);
        ASM_OUT("pop rax\n");
        compileFuncCall(file, pos, regs, nargs, true, func->fdepth);
        if(!req_val)
            ASM_OUT("pop rnn\n");
        return true;
    }
    if(expr->data.type == EXPR_CONST){
        if(req_val){
            ASM_OUT("push %d\n", int(expr->data.val));
        }
        return true;
    }
    if(expr->data.type == EXPR_OP){
        //printf_log("%s " , exprOpName(expr->data.op));
        if(isMathOp(expr->data.op))
            return compileMathOp(F_ARGS(expr), true);
        int instr_lbl_n = pos->lbl_id;

        switch(expr->data.op){
        case EXPR_O_ENDL:
            CHECK( compileCode     (F_ARGS(expr->left ), false) )
            CHECK( compileCodeBlock(F_ARGS(expr->right), false) )
            return true;
        case EXPR_O_IF:
            if(!compileCodeBlock(F_ARGS(expr->left), true))
                return false;
            (pos->lbl_id)++;
            ASM_OUT("push 0\n");
            if(!expr->right){
                error_log("'if' should have both L and R values\n");
                return false;
            }

            regsDescendLvl(file, pos, regs, 0, true);
            if(expr->right->data.type == EXPR_OP && expr->left->data.op == EXPR_O_SEP){
                ASM_OUT("jne :if_else_%d\n", instr_lbl_n);
                CHECK( compileCodeBlock(F_ARGS(expr->right->left ), req_val) )
                ASM_OUT("jmp :if_end_%d\n" , instr_lbl_n);
                regsDescendLvl(file, pos, regs, 0, true);

                ASM_OUT("if_else_%d:\n", instr_lbl_n);
                CHECK( compileCodeBlock(F_ARGS(expr->right->right), req_val) )
                ASM_OUT("if_end_%d:\n" , instr_lbl_n);
            }
            else{
                if(req_val)
                    return false;
                ASM_OUT("jeq :if_%d\n", instr_lbl_n);
                CHECK( compileCodeBlock(F_ARGS(expr->right), false) )
                ASM_OUT("if_%d:\n", instr_lbl_n);
            }
            regsDescendLvl(file, pos, regs, 0, true);
            return true;
        case EXPR_O_WHILE:
            (pos->lbl_id)++;
            ASM_OUT("while_%d_beg:\n", instr_lbl_n);
            regsDescendLvl(file, pos, regs, 0, true);
            CHECK( compileCodeBlock(F_ARGS(expr->left), true) )
            ASM_OUT("push 0\njeq :while_%d_end\n", instr_lbl_n);

            CHECK( compileCodeBlock(F_ARGS(expr->right), false) )
            ASM_OUT("jmp :while_%d_beg\n", instr_lbl_n);
            ASM_OUT("while_%d_end:\n", instr_lbl_n);
            regsDescendLvl(file, pos, regs, 0, true);
            return true;
        case EXPR_O_EQRTL:
            return compileSetVar(F_ARGS(expr), req_val, false);
        case EXPR_O_EQLTR:
            return compileSetVar(F_ARGS(expr), req_val, false);
        case EXPR_O_VDEF:
            if(req_val){
                error_log("var definition can not return a value\n");
                return false;
            }
            return compileVarDef(F_ARGS(expr->right), false);
        case EXPR_O_FDEF:
            if(req_val){
                error_log("function definition can not return a value\n");
                return false;
            }
            return compileFuncDef(F_ARGS(expr), false);
        case EXPR_O_VFDEF:
            if(req_val){
                error_log("v-function definition can not return a value\n");
                return false;
            }
            return compileFuncDef(F_ARGS(expr), false);

        default:
            printf_log("Unknown op: ");
            printExprElem(stderr, expr->data);
            printExprElem(_logfile, expr->data);
            printf_log("\n");
            return false;
        }

    }
    error_log("Unknown elem for code:");
    printExprElem(stderr, expr->data);
    printExprElem(_logfile, expr->data);
    printf_log("\n");
    return false;
}

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

    bool ret = compileCodeBlock(file, code, &objs, &pos, regs, false);
    if (!ret){
        error_log("Compiltion failed");
    }


    programPosDataDtor(&pos);
    programNameTableDtor(&objs);
    return ret;
}
