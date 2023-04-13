#include <locale.h>
#include <wchar.h>
#include <windows.h>

#define CONSOLE_UTILS_IMPL_INCLUDED


bool setConsoleColor(FILE* console, consoleColor text_color, consoleColor background_color){

    if(text_color &= COLOR_DEFAULT)
        text_color = COLOR_DEFAULTT;
    if(background_color &= COLOR_DEFAULT)
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

    return 0;
}

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


void initConsole(){
    const unsigned int codepage_id = 1251; // ANSI
    SetConsoleCP      (codepage_id);
    SetConsoleOutputCP(codepage_id);
    setlocale (LC_ALL,     "RU");
    setlocale (LC_NUMERIC, "C" );

    setConsoleFont(STD_INPUT_HANDLE );
    setConsoleFont(STD_OUTPUT_HANDLE);
    setConsoleFont(STD_ERROR_HANDLE );
}

bool moveCursor(FILE* console, int dx, int dy){
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
}
