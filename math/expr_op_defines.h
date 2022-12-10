#ifndef EXPR_OP_DEFINES_H_INCLUDED
#define EXPR_OP_DEFINES_H_INCLUDED


#endif // EXPR_OP_DEFINES_H_INCLUDED

// #define EXPR_OP_DEF(_enum, _enumval, _name, _pri, _ret, _diff)

EXPR_OP_DEF(EXPR_MO_ADD  ,1  | 0x40, "+"   , 1, a + b        )
EXPR_OP_DEF(EXPR_MO_SUB  ,2  | 0x40, "-"   , 1, a - b        )
EXPR_OP_DEF(EXPR_MO_UMIN ,2  | 0xC0, "-"   , 5, -b           )
EXPR_OP_DEF(EXPR_MO_MUL  ,3  | 0x40, "*"   , 2, a * b        )
EXPR_OP_DEF(EXPR_MO_DIV  ,4  | 0x40, "/"   , 2, a / b        )
EXPR_OP_DEF(EXPR_MO_POW  ,5  | 0x40, "^"   , 4, pow(a,b)     )
EXPR_OP_DEF(EXPR_MO_LOG  ,6  | 0x40, "log" , 3, log(b)/log(a))
EXPR_OP_DEF(EXPR_MO_EXP  ,8  | 0xC0, "exp" , 3, exp(b)       )
EXPR_OP_DEF(EXPR_MO_LN   ,9  | 0xC0, "ln"  , 3, log(b)       )
EXPR_OP_DEF(EXPR_MO_SIN  ,10 | 0xC0, "sin" , 3, sin(b)       )
EXPR_OP_DEF(EXPR_MO_COS  ,11 | 0xC0, "cos" , 3, cos(b)       )
EXPR_OP_DEF(EXPR_MO_TG   ,12 | 0xC0, "tan" , 3, tan(b)       )
EXPR_OP_DEF(EXPR_MO_ASIN ,13 | 0xC0, "asin", 3, asin(b)      )
EXPR_OP_DEF(EXPR_MO_ACOS ,14 | 0xC0, "acos", 3, acos(b)      )
EXPR_OP_DEF(EXPR_MO_ATG  ,15 | 0xC0, "atan", 3, atan(b)      )
EXPR_OP_DEF(EXPR_MO_SH   ,16 | 0xC0, "sinh", 3, sinh(a)      )
EXPR_OP_DEF(EXPR_MO_CH   ,17 | 0xC0, "cosh", 3, cosh(a)      )
EXPR_OP_DEF(EXPR_MO_TH   ,18 | 0xC0, "tanh", 3, tanh(a)      )
EXPR_OP_DEF(EXPR_MO_SQRT ,19 | 0xC0,  u8"√", 3, sqrt(a)      )
EXPR_OP_DEF(EXPR_MO_o    ,20 | 0xC0, "o"   , 3, 0            )
EXPR_OP_DEF(EXPR_MO_d    ,21 | 0xC0, "d"   , 3, 0            )
// -3 can recieve ✘ - blocks
// -2 can recieve whole lines of code
// -1 can recieve whole math expresions with =
//  0 can recieve math expression sides

EXPR_OP_DEF(EXPR_O_COMMA ,7         , ","     , 0, NAN          )
EXPR_OP_DEF(EXPR_O_IF    ,22        , u8"⁉"   , -3, NAN          )
EXPR_OP_DEF(EXPR_O_SEP   ,23        , u8"✘"   , -2, NAN          )
EXPR_OP_DEF(EXPR_O_WHILE ,24        , u8"🔄"   , -3, NAN          )
EXPR_OP_DEF(EXPR_O_FOR   ,25        , u8"🔁"   , -3, NAN          )
EXPR_OP_DEF(EXPR_O_VDEF  ,26  | 0x80, u8"★"   , -1, NAN          )
EXPR_OP_DEF(EXPR_O_FDEF  ,27        , u8"✷"   , -3, NAN          )
EXPR_OP_DEF(EXPR_O_VFDEF ,28        , u8"❂"   , -3, NAN          )
EXPR_OP_DEF(EXPR_O_CDEF  ,29  | 0x80, u8"☆"   , -1, NAN          )
EXPR_OP_DEF(EXPR_O_EQRTL ,32       , u8"←"    , 0 , NAN          )
EXPR_OP_DEF(EXPR_O_EQLTR ,33       , u8"→"    , 0 , NAN          )
EXPR_OP_DEF(EXPR_O_ENDL  ,34       , u8";"    , -4, NAN          )

