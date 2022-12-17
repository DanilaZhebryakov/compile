#ifndef EXPR_OP_DEFINES_H_INCLUDED
#define EXPR_OP_DEFINES_H_INCLUDED


#endif // EXPR_OP_DEFINES_H_INCLUDED

// #define EXPR_OP_DEF(_enum, _enumval, _name, _pri, _ret, _diff, _std)

EXPR_OP_DEF(EXPR_MO_ADD  ,1  | 0x40, "+"   , 2, a + b        , "ADD")
EXPR_OP_DEF(EXPR_MO_SUB  ,2  | 0x40, "-"   , 2, a - b        , "SUB")
EXPR_OP_DEF(EXPR_MO_UMIN ,2  | 0xC0, "-"   , 6, -b           , "UMIN")
EXPR_OP_DEF(EXPR_MO_MUL  ,3  | 0x40, "*"   , 3, a * b        , "MUL")
EXPR_OP_DEF(EXPR_MO_DIV  ,4  | 0x40, "/"   , 3, a / b        , "DIV")
EXPR_OP_DEF(EXPR_MO_POW  ,5  | 0x40, "^"   , 5, pow(a,b)     , "POW")
EXPR_OP_DEF(EXPR_MO_LOG  ,6  | 0x40, "log" , 4, log(b)/log(a), "LOG")
EXPR_OP_DEF(EXPR_MO_EXP  ,8  | 0xC0, "exp" , 4, exp(b)       , "EXP")
EXPR_OP_DEF(EXPR_MO_LN   ,9  | 0xC0, "ln"  , 4, log(b)       , "LN" )
EXPR_OP_DEF(EXPR_MO_SIN  ,10 | 0xC0, "sin" , 4, sin(b)       , "SIN")
EXPR_OP_DEF(EXPR_MO_COS  ,11 | 0xC0, "cos" , 4, cos(b)       , "COS")
EXPR_OP_DEF(EXPR_MO_TG   ,12 | 0xC0, "tan" , 4, tan(b)       , "TAN")
EXPR_OP_DEF(EXPR_MO_ASIN ,13 | 0xC0, "asin", 4, asin(b)      , "ASIN")
EXPR_OP_DEF(EXPR_MO_ACOS ,14 | 0xC0, "acos", 4, acos(b)      , "ACOS")
EXPR_OP_DEF(EXPR_MO_ATG  ,15 | 0xC0, "atan", 4, atan(b)      , "ATAN")
EXPR_OP_DEF(EXPR_MO_SH   ,16 | 0xC0, "sinh", 4, sinh(a)      , "SINH")
EXPR_OP_DEF(EXPR_MO_CH   ,17 | 0xC0, "cosh", 4, cosh(a)      , "COSH")
EXPR_OP_DEF(EXPR_MO_TH   ,18 | 0xC0, "tanh", 4, tanh(a)      , "TANH")
EXPR_OP_DEF(EXPR_MO_SQRT ,19 | 0xC0,  u8"√", 4, sqrt(a)      , "SQRT")
EXPR_OP_DEF(EXPR_MO_o    ,20 | 0xC0, "o"   , 4, 0            , "!!!!")
EXPR_OP_DEF(EXPR_MO_d    ,21 | 0xC0, "d"   , 4, 0            , "!!!!")
EXPR_OP_DEF(EXPR_MO_TANP ,22 | 0xC0, "∟"   , 4, tan(b)*1000  , "TANP")


EXPR_OP_DEF(EXPR_MO_BOOL ,23 | 0xC0, "<+>"  , 1, a !=0        , "BOOL")
EXPR_OP_DEF(EXPR_MO_CGT  ,24 | 0x40, ">"    , 1, a > b        , "CGT")
EXPR_OP_DEF(EXPR_MO_CLT  ,25 | 0x40, "<"    , 1, a < b        , "CLT")
EXPR_OP_DEF(EXPR_MO_CEQ  ,26 | 0x40, "="    , 1, a ==b        , "CEQ")
EXPR_OP_DEF(EXPR_MO_CNE  ,27 | 0x40, "!="   , 1, a !=b        , "CNE")
EXPR_OP_DEF(EXPR_MO_CGE  ,28 | 0x40, ">="   , 1, a >=b        , "CGE")
EXPR_OP_DEF(EXPR_MO_CLE  ,29 | 0x40, "<="   , 1, a <=b        , "CLE")

EXPR_OP_DEF(EXPR_MO_BOR   ,30 | 0x40, "|"   , 2, (int)a | (int)b , "BOR" )
EXPR_OP_DEF(EXPR_MO_BAND  ,31 | 0x40, "&"   , 2, (int)a & (int)b , "BAND")
EXPR_OP_DEF(EXPR_MO_BXOR  ,32 | 0x40, "⊕"  , 2, (int)a ^ (int)b , "BXOR")
EXPR_OP_DEF(EXPR_MO_BSL   ,33 | 0x40, "<<"  , 2, (int)a <<(int)b , "BSL" )
EXPR_OP_DEF(EXPR_MO_BSR   ,34 | 0x40, ">>"  , 2, (int)a >>(int)b , "BSR" )

// -3 can recieve ✘ - blocks
// -2 can recieve whole lines of code
// -1 can recieve whole math expresions with =
//  0 can recieve math expression sides

EXPR_OP_DEF(EXPR_O_COMMA ,1        , ","     , 0 , NAN    , "PARAM" )
EXPR_OP_DEF(EXPR_O_IF    ,2        , u8"⁉"   , -3, NAN    , "IF"    )
EXPR_OP_DEF(EXPR_O_SEP   ,3        , u8"✘"   , -2, NAN    , "SEP"   )
EXPR_OP_DEF(EXPR_O_WHILE ,4        , u8"🔄"   , -3, NAN    , "WHILE" )
EXPR_OP_DEF(EXPR_O_FOR   ,5        , u8"🔁"   , -3, NAN    , "FOR"   )
EXPR_OP_DEF(EXPR_O_VDEF  ,6  | 0x80, u8"★"   , -1, NAN    , "VAR"   )
EXPR_OP_DEF(EXPR_O_FDEF  ,7        , u8"✷"   , -3, NAN    , "FUNC"  )
EXPR_OP_DEF(EXPR_O_VFDEF ,8        , u8"❂"   , -3, NAN    , "VFUNC" )
EXPR_OP_DEF(EXPR_O_CDEF  ,9        , u8"☆"   , -1, NAN    , "CONST" )
EXPR_OP_DEF(EXPR_O_EQRTL ,12       , u8"←"    , 0 , NAN    , "EQ"    )
EXPR_OP_DEF(EXPR_O_EQLTR ,13       , u8"→"    , 0 , NAN    , "!!EQ"  )
EXPR_OP_DEF(EXPR_O_ENDL  ,14       , u8";"    , -4, NAN    , "ST"    )
EXPR_OP_DEF(EXPR_O_ARIND ,15       , u8"@"    , 6 , NAN    , "AT"    )
EXPR_OP_DEF(EXPR_O_ARDEF ,16       , u8"!@"   , 6 , NAN    , "OF"    )
