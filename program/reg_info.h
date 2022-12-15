#ifndef REG_INFO_H_INCLUDED
#define REG_INFO_H_INCLUDED

const int REG_USE_CNT = 10;
const int REG_USE_FIRST = 2;

struct RegInfo{
    int mem_addr;
    int load_prog_lvl;
    int load_mem_n;
    VarEntry* var;
};

static void unloadVarFromReg(REG_ADD_ARGS RegInfo* regs, int reg_n, bool write);
static void loadVarToReg(REG_ADD_ARGS RegInfo* regs, int reg_n, VarEntry* var, bool read);

static void printRegInfo(REG_ADD_ARGS RegInfo* regs){
    for(int i = 0; i < REG_USE_CNT; i++){
        if(regs[i].var)
        fprintf(file, "#reg %d contains var %s (@%d)\n", i + REG_USE_FIRST, regs[i].var->name, regs[i].var->value);
    }
}

static int getRegWithVar(REG_ADD_ARGS RegInfo* regs, VarEntry* var){
    for(int i = 0; i < REG_USE_CNT; i++){
        if(regs[i].var == var){
            return i;
        }
    }
    return -1;
}

static int findRegForVar(REG_ADD_ARGS RegInfo* regs, VarEntry* var, int lvl, bool read){
    int min_n = 1000000000, max_n = 0, min_reg = 0;
    for(int i = 0; i < REG_USE_CNT; i++){
        if(regs[i].load_mem_n > max_n)
            max_n = regs[i].load_mem_n;
        if(regs[i].load_mem_n < min_n){
            min_n = regs[i].load_mem_n;
            min_reg = i;
        }
    }
    if(regs[min_reg].load_mem_n != 0){
        unloadVarFromReg(REG_ADD_ARGS_CALL regs, min_reg, true);
    }
    loadVarToReg(REG_ADD_ARGS_CALL regs, min_reg, var, read);
    regs[min_reg].load_prog_lvl = lvl;
    regs[min_reg].load_mem_n = max_n + 1;
    return min_reg;
}

static void regsDescendLvl(REG_ADD_ARGS RegInfo* regs, int lvl, bool save_all){
    fprintf(file, "#save\n");
    printRegInfo(file, pos, regs);
    for(int i = 0; i < REG_USE_CNT; i++){
        if(regs[i].load_prog_lvl >= lvl && regs[i].var != nullptr){
            unloadVarFromReg(REG_ADD_ARGS_CALL regs, i, regs[i].var->depth < lvl || save_all );
        }
    }
}


#endif // REG_INFO_H_INCLUDED
