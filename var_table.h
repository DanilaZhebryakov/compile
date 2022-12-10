#ifndef VAR_TABLE_H_INCLUDED
#define VAR_TABLE_H_INCLUDED

#include "lib/UStack.h"

#define INC_VAR_TABLE(TableName , tableName, tableType) \
    struct TableName##Entry{    \
        tableType value;    \
        char*  name;    \
        int depth;  \
        int fdepth;  \
    };  \
    typedef UStack TableName##Table;    \
    \
    void tableName##TableCtor(TableName##Table* stk);                               \
    void tableName##TableDtor(TableName##Table* stk);                               \
    bool tableName##TablePut(TableName##Table* stk, TableName##Entry var);                  \
    TableName##Entry* tableName##TableGet(TableName##Table* stk, char* s_name);             \
    TableName##Entry* tableName##TableGetLast(TableName##Table* stk);                       \
    int tableName##TableDescendLvl(TableName##Table* stk, int lvl);


#define DEF_VAR_TABLE(TableName , tableName, tableType) \
\
void tableName##TableCtor(TableName##Table* stk){\
    ustackCtor(stk, sizeof(TableName##Entry));\
}\
\
void tableName##TableDtor(TableName##Table* stk){\
    ustackDtor(stk);\
}\
\
bool tableName##TablePut(TableName##Table* stk, TableName##Entry var){\
    return ustackPush(stk, &var) == VAR_NOERROR;\
}\
\
TableName##Entry* tableName##TableGet(TableName##Table* stk, char* s_name){\
    for (TableName##Entry* i = (TableName##Entry*)stk->data + stk->size - 1; i >= ((TableName##Entry*)stk->data); i--){\
        if(strcmp(i->name, s_name) == 0){\
            return i;\
        }\
    }\
    return nullptr;\
}\
\
TableName##Entry* tableName##TableGetLast(TableName##Table* stk){\
    return ((TableName##Entry*)stk->data) + stk->size - 1;\
}\
\
int tableName##TableDescendLvl(TableName##Table* stk, int lvl){\
    int ret = 0;\
    while(stk->size > 0 && (((TableName##Entry*)stk->data) + stk->size - 1)->depth >= lvl && (ustackPop(stk, nullptr) == VAR_NOERROR)) {ret++;}\
    return ret;\
}\

#endif // VAR_TABLE_H_INCLUDED
