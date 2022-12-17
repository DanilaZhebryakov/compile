#ifndef EXPR_OP_DEFINES_H_INCLUDED
#define EXPR_OP_DEFINES_H_INCLUDED


#endif // EXPR_OP_DEFINES_H_INCLUDED

// #define EXPR_OP_DEF(_enum, _enumval, _name, _pri, _ret, _diff, _std)

EXPR_OP_DEF(EXPR_MO_ADD  ,1  | 0x40, "+"   , 1, a + b        , "ADD")
EXPR_OP_DEF(EXPR_MO_SUB  ,2  | 0x40, "-"   , 1, a - b        , "SUB")
EXPR_OP_DEF(EXPR_MO_UMIN ,2  | 0xC0, "-"   , 5, -b           , "UMIN")
EXPR_OP_DEF(EXPR_MO_MUL  ,3  | 0x40, "*"   , 2, a * b        , "MUL")
EXPR_OP_DEF(EXPR_MO_DIV  ,4  | 0x40, "/"   , 2, a / b        , "DIV")
EXPR_OP_DEF(EXPR_MO_POW  ,5  | 0x40, "^"   , 4, pow(a,b)     , "POW")
EXPR_OP_DEF(EXPR_MO_LOG  ,6  | 0x40, "log" , 3, log(b)/log(a), "LOG")
EXPR_OP_DEF(EXPR_MO_EXP  ,8  | 0xC0, "exp" , 3, exp(b)       , "EXP")
EXPR_OP_DEF(EXPR_MO_LN   ,9  | 0xC0, "ln"  , 3, log(b)       , "LN" )
EXPR_OP_DEF(EXPR_MO_SIN  ,10 | 0xC0, "sin" , 3, sin(b)       , "SIN")
EXPR_OP_DEF(EXPR_MO_COS  ,11 | 0xC0, "cos" , 3, cos(b)       , "COS")
EXPR_OP_DEF(EXPR_MO_TG   ,12 | 0xC0, "tan" , 3, tan(b)       , "TAN")
EXPR_OP_DEF(EXPR_MO_ASIN ,13 | 0xC0, "asin", 3, asin(b)      , "ASIN")
EXPR_OP_DEF(EXPR_MO_ACOS ,14 | 0xC0, "acos", 3, acos(b)      , "ACOS")
EXPR_OP_DEF(EXPR_MO_ATG  ,15 | 0xC0, "atan", 3, atan(b)      , "ATAN")
EXPR_OP_DEF(EXPR_MO_SH   ,16 | 0xC0, "sinh", 3, sinh(a)      , "SINH")
EXPR_OP_DEF(EXPR_MO_CH   ,17 | 0xC0, "cosh", 3, cosh(a)      , "COSH")
EXPR_OP_DEF(EXPR_MO_TH   ,18 | 0xC0, "tanh", 3, tanh(a)      , "TANH")
EXPR_OP_DEF(EXPR_MO_SQRT ,19 | 0xC0,  u8"√", 3, sqrt(a)      , "SQRT")
EXPR_OP_DEF(EXPR_MO_o    ,20 | 0xC0, "o"   , 3, 0            , "!!!!")
EXPR_OP_DEF(EXPR_MO_d    ,21 | 0xC0, "d"   , 3, 0            , "!!!!")
EXPR_OP_DEF(EXPR_MO_TANP ,22 | 0xC0, "∟"   , 3, tan(b)*1000  , "TANP")

// -3 can recieve ✘ - blocks
// -2 can recieve whole lines of code
// -1 can recieve whole math expresions with =
//  0 can recieve math expression sides

EXPR_OP_DEF(EXPR_O_COMMA ,7         , ","     , 0 , NAN    , "PARAM" )
EXPR_OP_DEF(EXPR_O_IF    ,22        , u8"⁉"   , -3, NAN    , "IF"    )
EXPR_OP_DEF(EXPR_O_SEP   ,23        , u8"✘"   , -2, NAN    , "SEP"   )
EXPR_OP_DEF(EXPR_O_WHILE ,24        , u8"🔄"   , -3, NAN    , "WHILE" )
EXPR_OP_DEF(EXPR_O_FOR   ,25        , u8"🔁"   , -3, NAN    , "FOR"   )
EXPR_OP_DEF(EXPR_O_VDEF  ,26  | 0x80, u8"★"   , -1, NAN    , "VAR"   )
EXPR_OP_DEF(EXPR_O_FDEF  ,27        , u8"✷"   , -3, NAN    , "FUNC"  )
EXPR_OP_DEF(EXPR_O_VFDEF ,28        , u8"❂"   , -3, NAN    , "VFUNC" )
EXPR_OP_DEF(EXPR_O_CDEF  ,29        , u8"☆"   , -1, NAN    , "CONST" )
EXPR_OP_DEF(EXPR_O_EQRTL ,32       , u8"←"    , 0 , NAN    , "EQ"    )
EXPR_OP_DEF(EXPR_O_EQLTR ,33       , u8"→"    , 0 , NAN    , "!!EQ"  )
EXPR_OP_DEF(EXPR_O_ENDL  ,34       , u8";"    , -4, NAN    , "ST"    )
EXPR_OP_DEF(EXPR_O_ARIND ,35       , u8"@"    , 5 , NAN    , "AT"    )
EXPR_OP_DEF(EXPR_O_ARDEF ,36       , u8"!@"   , 5 , NAN    , "OF"    )
