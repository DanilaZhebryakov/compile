#include "expr_op_flags.h"
// #define EXPR_OP_DEF(_enum, _enumval, _name, _pri, _ret, _diff, _std)

EXPR_OP_DEF(EXPR_MO_ADD  ,1  | EF_MATH          , "+"   , 2, a + b        , "ADD")
EXPR_OP_DEF(EXPR_MO_SUB  ,2  | EF_MATH          , "-"   , 2, a - b        , "SUB")
EXPR_OP_DEF(EXPR_MO_UMIN ,2  | EF_MATH | EF_UNAR, "-"   , 6, -b           , "UMIN")
EXPR_OP_DEF(EXPR_MO_MUL  ,3  | EF_MATH          , "*"   , 3, a * b        , "MUL")
EXPR_OP_DEF(EXPR_MO_DIV  ,4  | EF_MATH          , "/"   , 3, a / b        , "DIV")
EXPR_OP_DEF(EXPR_MO_POW  ,5  | EF_MATH          , "^"   , 5, pow(a,b)     , "POW")
EXPR_OP_DEF(EXPR_MO_LOG  ,6  | EF_MATH          , "log" , 4, log(b)/log(a), "LOG")
EXPR_OP_DEF(EXPR_MO_EXP  ,8  | EF_MATH | EF_UNAR, "exp" , 4, exp(b)       , "EXP")
EXPR_OP_DEF(EXPR_MO_LN   ,9  | EF_MATH | EF_UNAR, "ln"  , 4, log(b)       , "LN" )
EXPR_OP_DEF(EXPR_MO_SIN  ,10 | EF_MATH | EF_UNAR, "sin" , 4, sin(b)       , "SIN")
EXPR_OP_DEF(EXPR_MO_COS  ,11 | EF_MATH | EF_UNAR, "cos" , 4, cos(b)       , "COS")
EXPR_OP_DEF(EXPR_MO_TG   ,12 | EF_MATH | EF_UNAR, "tan" , 4, tan(b)       , "TAN")
EXPR_OP_DEF(EXPR_MO_ASIN ,13 | EF_MATH | EF_UNAR, "asin", 4, asin(b)      , "ASIN")
EXPR_OP_DEF(EXPR_MO_ACOS ,14 | EF_MATH | EF_UNAR, "acos", 4, acos(b)      , "ACOS")
EXPR_OP_DEF(EXPR_MO_ATG  ,15 | EF_MATH | EF_UNAR, "atan", 4, atan(b)      , "ATAN")
EXPR_OP_DEF(EXPR_MO_SH   ,16 | EF_MATH | EF_UNAR, "sinh", 4, sinh(a)      , "SINH")
EXPR_OP_DEF(EXPR_MO_CH   ,17 | EF_MATH | EF_UNAR, "cosh", 4, cosh(a)      , "COSH")
EXPR_OP_DEF(EXPR_MO_TH   ,18 | EF_MATH | EF_UNAR, "tanh", 4, tanh(a)      , "TANH")
EXPR_OP_DEF(EXPR_MO_SQRT ,19 | EF_MATH | EF_UNAR,  u8"√", 4, sqrt(a)      , "SQRT")
EXPR_OP_DEF(EXPR_MO_o    ,20 | EF_MATH | EF_UNAR, u8"𝐨" , 4, 0            , "!!!!")
EXPR_OP_DEF(EXPR_MO_d    ,21 | EF_MATH | EF_UNAR, u8"𝐝" , 4, 0            , "!!!!")
EXPR_OP_DEF(EXPR_MO_TANP ,22 | EF_MATH | EF_UNAR, "∟"   , 4, tan(b)*1000  , "TANP")


EXPR_OP_DEF(EXPR_MO_BOOL ,23 | EF_MATH | EF_UNAR, "⋄"    , 1, a !=0        , "BOOL")
EXPR_OP_DEF(EXPR_MO_CGT  ,24 | EF_MATH          , ">"    , 1, a > b        , "CGT")
EXPR_OP_DEF(EXPR_MO_CLT  ,25 | EF_MATH          , "<"    , 1, a < b        , "CLT")
EXPR_OP_DEF(EXPR_MO_CEQ  ,26 | EF_MATH          , "="    , 1, a ==b        , "CEQ")
EXPR_OP_DEF(EXPR_MO_CNE  ,27 | EF_MATH          , "!="   , 1, a !=b        , "CNE")
EXPR_OP_DEF(EXPR_MO_CGE  ,28 | EF_MATH          , ">="   , 1, a >=b        , "CGE")
EXPR_OP_DEF(EXPR_MO_CLE  ,29 | EF_MATH          , "<="   , 1, a <=b        , "CLE")

EXPR_OP_DEF(EXPR_MO_BOR   ,30 | EF_MATH         , "|"   , 2, (int)a | (int)b , "BOR" )
EXPR_OP_DEF(EXPR_MO_BAND  ,31 | EF_MATH         , "&"   , 2, (int)a & (int)b , "BAND")
EXPR_OP_DEF(EXPR_MO_BXOR  ,32 | EF_MATH         , "⊕"   , 2, (int)a ^ (int)b , "BXOR")
EXPR_OP_DEF(EXPR_MO_BSL   ,33 | EF_MATH         , "<<"  , 2, (int)a <<(int)b , "BSL" )
EXPR_OP_DEF(EXPR_MO_BSR   ,34 | EF_MATH         , ">>"  , 2, (int)a >>(int)b , "BSR" )

// -3 can recieve ✘ - blocks
// -2 can recieve whole lines of code
// -1 can recieve whole math expresions with =
//  0 can recieve math expression sides

EXPR_OP_DEF(EXPR_O_COMMA  ,1                      , ","      , 0 , NAN    , "PARAM" )
EXPR_OP_DEF(EXPR_O_RCOMMA ,1 | EF_REV             , "⸲"      , 0 , NAN    , "RPARAM" )


EXPR_OP_DEF(EXPR_O_IF    ,2                      , u8"⁉"    , -3, NAN    , "IF"    )
EXPR_OP_DEF(EXPR_O_ELSE  ,3                      , u8"✘"    , -2, NAN    , "ELSE"   )
EXPR_OP_DEF(EXPR_O_SEP   ,4                      , u8"⁝"    , -2, NAN     , "SEP"   )
EXPR_OP_DEF(EXPR_O_WHILE ,5                      , u8"🔄"    , -3, NAN    , "WHILE" )
EXPR_OP_DEF(EXPR_O_FOR   ,6                      , u8"🔁"    , -3, NAN    , "FOR"   )
EXPR_OP_DEF(EXPR_O_VDEF  ,7  | EF_UNAR           , u8"★"    , -1, NAN    , "VAR"   )
EXPR_OP_DEF(EXPR_O_FDEF  ,8                      , u8"✷"    , -3, NAN    , "FUNC"  )
EXPR_OP_DEF(EXPR_O_VFDEF ,9                      , u8"❂"    , -3, NAN    , "VFUNC" )
EXPR_OP_DEF(EXPR_O_CDEF  ,10                     , u8"☆"    , -1, NAN    , "CONST" )
EXPR_OP_DEF(EXPR_O_ENDL  ,11                     , u8";"    , -4, NAN    , "ST"    )
EXPR_OP_DEF(EXPR_O_RENDL ,11 | EF_REV            , u8"⸵"    , -4, NAN    , "RST"   )
EXPR_OP_DEF(EXPR_O_ARIND ,12                     , u8"@"    , 6 , NAN    , "AT"    )
EXPR_OP_DEF(EXPR_O_ARDEF ,13                     , u8"!@"   , 6 , NAN    , "OF"    )

EXPR_OP_DEF(EXPR_O_CER   ,14                     , u8"⸢"    , -4 , NAN    , "CER"    )
EXPR_OP_DEF(EXPR_O_CEL   ,15                     , u8"⸣"    , -4 , NAN    , "CEL"    )
EXPR_OP_DEF(EXPR_O_RCER  ,14 | EF_REV            , u8"⸥"    , -4 , NAN    , "RCER"    )
EXPR_OP_DEF(EXPR_O_RCEL  ,15 | EF_REV            , u8"⸤"    , -4 , NAN    , "RCEL"    )

EXPR_OP_DEF(EXPR_O_LOAD  ,14 | EF_UNAR           , u8"⟱"    , -2, NAN    , "LOAD"  ) /*for future use with multi-file compilation*/
EXPR_OP_DEF(EXPR_O_VIEW  ,15 | EF_UNAR           , u8"⏿"    , -2, NAN    , "VIEW"  ) // 'table transfer' (function/variable prototypes)
EXPR_OP_DEF(EXPR_O_PUBL  ,16 | EF_UNAR           , u8"☭"    , -2, NAN    , "PUBLIC" )
EXPR_OP_DEF(EXPR_O_TABL  ,17 | EF_UNAR           , u8"┬─┬"  , -2, NAN    , "TABLE"  ) //manual table editing. '⏿' compiles into this


EXPR_OP_DEF(EXPR_O_EQRTL ,1 | EF_EQL            , u8"←"    , 0 , NAN    , "EQ"    )
EXPR_OP_DEF(EXPR_O_EQLTR ,1 | EF_EQL | EF_REV   , u8"→"    , 0 , NAN    , "!!EQ"  )
EXPR_OP_DEF(EXPR_O_EQALTR ,2 | EF_EQL | EF_REV   , u8"⟴"    , 0 , NAN   , "!!EQADD"  )
EXPR_OP_DEF(EXPR_O_EQARTL ,2 | EF_EQL            , u8"⬲"    , 0 , NAN    , "EQADD"    )
EXPR_OP_DEF(EXPR_O_EQSPEC  ,3 | EF_EQL           , u8"!←!"    , 0 , NAN  , "!!EQSPEC"    )
//not meant for manual use. Generated automatically to resolve stuff like a → b ← c or a ← b → c (resolves into b !←! (a, c) or (a, c) !←! b )