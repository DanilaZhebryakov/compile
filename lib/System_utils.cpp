#include <string.h>

#ifdef _WIN32
    #include "System_utils_win_src.h"
#endif

#ifdef __unix__
    #include "System_utils_lin_src.h"
#endif

#ifdef __MACH__
    #include "System_utils_lin_src.h"
#endif

#ifndef SYS_UTILS_IMPL_INCLUDED
    #include "System_utils_blank_src.h"
#endif
