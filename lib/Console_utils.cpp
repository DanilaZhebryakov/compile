#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "Console_utils.h"

unsigned int consoleColorAsHex(consoleColor col){
    unsigned int res = 0;
    unsigned int col_val = (col & COLOR_INTENSE) ? 0xFF : 0x7f;

    res |= (col & COLOR_RED  ) ? (col_val << 16)  : 0;
    res |= (col & COLOR_GREEN) ? (col_val << 8 )  : 0;
    res |= (col & COLOR_BLUE ) ? (col_val << 0 )  : 0;
    return res;
}

char consoleColorNumber(consoleColor col){
    char res = 0;
    res += (col & COLOR_RED  ) ? 1 : 0;
    res += (col & COLOR_GREEN) ? 2 : 0;
    res += (col & COLOR_BLUE ) ? 4 : 0;
    return res;
}

#ifdef _USE_ANSI_CONSOLE_ESCAPE
    #include "Console_utils_ansi_src.h"
#else
    #ifdef _WIN32
        #include "Console_utils_win_src.h"
    #endif
    #ifdef __unix__
        #include "Console_utils_ansi_src.h"
    #endif
    #ifdef __MACH__
        #include "Console_utils_ansi_src.h"
    #endif
#endif

#ifndef CONSOLE_UTILS_IMPL_INCLUDED
    #include "Console_utils_blank_src.h"
#endif


void createProgressBar(FILE* out, int total, int filled, const char* bar_string, consoleColor color_full, consoleColor color_empty){
    assert(strlen(bar_string) >= 5);
    fputc(bar_string[0], out);
    if (!(color_full & COLOR_NOCHANGE)){
        fflush(out);
        setConsoleColor(out, color_full, COLOR_BLACK);
    }
    for (int x = 0; x < filled; x++)
        fputc(bar_string[1], out);
    if (!(color_empty & COLOR_NOCHANGE)){
        fflush(out);
        setConsoleColor(out, color_empty, COLOR_BLACK);
    }


    if (filled != total)
        fputc(bar_string[2], out);
    for (int x = filled+1; x < total; x++)
        fputc(bar_string[3], out);

    fputc(bar_string[4], out);
    fflush(out);
}
void createNormalProgressBar(FILE* out, int total, int filled){
    createProgressBar(out, total, filled, "[=  ]", COLOR_GREEN, COLOR_DEFAULT);
}

void createSimpleProgressBar(FILE* out, int total, int filled){
    createProgressBar(out, total, filled, "[=  ]", COLOR_NOCHANGE, COLOR_NOCHANGE);
}
