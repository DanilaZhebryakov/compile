#include <stdio.h>
#include <assert.h>
#include <string.h>

#ifdef WINDOWS
    #include <locale.h>
    #include <wchar.h>
    #include <windows.h>
#endif

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

bool setConsoleColor(FILE* console, consoleColor text_color, consoleColor background_color){
    #ifdef WINDOWS
    if(text_color == COLOR_DEFAULT)
        text_color = COLOR_DEFAULTT;
    if(background_color == COLOR_DEFAULT)
        background_color = COLOR_DEFAULTB;
    WORD textAttribute = 0;
    textAttribute |= (text_color       & COLOR_RED     ) ? FOREGROUND_RED       : 0;
    textAttribute |= (text_color       & COLOR_GREEN   ) ? FOREGROUND_GREEN     : 0;
    textAttribute |= (text_color       & COLOR_BLUE    ) ? FOREGROUND_BLUE      : 0;
    textAttribute |= (text_color       & COLOR_INTENSE ) ? FOREGROUND_INTENSITY : 0;

    textAttribute |= (background_color & COLOR_RED     ) ? BACKGROUND_RED       : 0;
    textAttribute |= (background_color & COLOR_GREEN   ) ? BACKGROUND_GREEN     : 0;
    textAttribute |= (background_color & COLOR_BLUE    ) ? BACKGROUND_BLUE      : 0;
    textAttribute |= (background_color & COLOR_INTENSE ) ? BACKGROUND_INTENSITY : 0;

    if(console == stdin)
        return SetConsoleTextAttribute(GetStdHandle(STD_INPUT_HANDLE ), textAttribute);
    if(console == stdout)
        return SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), textAttribute);
    if(console == stderr)
        return SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE ), textAttribute);
    #endif
    if(text_color == COLOR_DEFAULT && background_color == COLOR_DEFAULT){
        fprintf(console, "\e[0m");
        return 1;
    }
    fprintf(console, "\e[");
    if(!(text_color & COLOR_NOCHANGE)){
        fprintf(console, text_color & COLOR_INTENSE ? "9%c;" : "3%c;", consoleColorNumber(text_color));
    }
    if(!(background_color & COLOR_NOCHANGE)){
        fprintf(console, text_color & COLOR_INTENSE ? "4%c;" : "10%c;", consoleColorNumber(text_color));
    }
    fprintf(console, "m");
    return 1;
}
#ifdef WINDOWS
static bool setConsoleFont(DWORD console){
    
    CONSOLE_FONT_INFOEX font = {sizeof(font)};
    if (!GetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), false, &font)){
       return 0;
    }
    font.FontFamily = 0x36;
    font.dwFontSize.Y = 15;
    wcsncpy(font.FaceName, L"Consolas", LF_FACESIZE);
    return SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), false, &font);
}
#endif

void initConsole(){
    #ifdef WINDOWS
    const unsigned int codepage_id = 1251; // ANSI
    SetConsoleCP      (codepage_id);
    SetConsoleOutputCP(codepage_id);
    setlocale (LC_ALL,     "RU");
    setlocale (LC_NUMERIC, "C" );

    setConsoleFont(STD_INPUT_HANDLE );
    setConsoleFont(STD_OUTPUT_HANDLE);
    setConsoleFont(STD_ERROR_HANDLE );
    #endif
}

bool moveCursor(FILE* console, int dx, int dy){
    #ifdef WINDOWS
    HANDLE WINAPI console_handle = 0;
    if     (console == stdin){
        console_handle = GetStdHandle(STD_INPUT_HANDLE);
    }
    else if(console == stdout){
        console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    else if(console == stderr){
        console_handle = GetStdHandle(STD_ERROR_HANDLE);
    }
    else{
        return 0;
    }

    _CONSOLE_SCREEN_BUFFER_INFO screen_info = {};
    GetConsoleScreenBufferInfo(console_handle, &screen_info);
    screen_info.dwCursorPosition.X += dx;
    screen_info.dwCursorPosition.Y += dy;
    return SetConsoleCursorPosition  (console_handle, screen_info.dwCursorPosition);
    #else

    if(dy < 0){
        fprintf(console, "\e[%dA", -dy);
    }
    if(dy > 0){
        fprintf(console, "\e[%dB", dy);
    }
    if(dx > 0){
        fprintf(console, "\e[%dC", dx);
    }
    if(dx < 0){
        fprintf(console, "\e[%dD", -dx);
    }


    return 1;
    #endif
}


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

