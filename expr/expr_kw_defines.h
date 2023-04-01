
//#define EXPR_KW_DEF(_enum, _enumval, _name, _std)

EXPR_KW_DEF(EXPR_KW_RET    , 1, u8"🚪"  ,  "RET"      ) /* you usually exit using door, this brings you where you came from*/
EXPR_KW_DEF(EXPR_KW_EXIT   , 2, u8"🪟"  , "EXIT"      )/* you can also exit from a window. This will immediately bring you out, but is (usually) highly suggested against*/
EXPR_KW_DEF(EXPR_KW_PWR    , 3, u8"⏻" , "PWR"       ) /* it's never known what this does exactly, but it is, that writing 0 into there immediately powers off whatever device program runs on*/
EXPR_KW_DEF(EXPR_KW_TRAP   , 4, u8"🪤" , "TRAP"       )
EXPR_KW_DEF(EXPR_KW_NIO    , 5, u8"▲"  , "SCREEN_N" )
EXPR_KW_DEF(EXPR_KW_CIO    , 6, u8"ᐃ"  , "SCREEN_C" )
EXPR_KW_DEF(EXPR_KW_BAD    , 7, u8"🔫"  , "REVOLVER" ) /* the only way to learn the game is to play with better opponent. Like processor exceptions. */
EXPR_KW_DEF(EXPR_KW_NULL   , 8, u8"∅"  , "trash"    )  /* The Void */
