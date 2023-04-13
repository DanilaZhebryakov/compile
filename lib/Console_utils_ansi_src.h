#define CONSOLE_UTILS_IMPL_INCLUDED

bool setConsoleColor(FILE* console, consoleColor text_color, consoleColor background_color){
    if ((text_color & COLOR_DEFAULT) && (background_color & COLOR_DEFAULT)){
        fprintf(console, "\e[0m");
        return 1;
    }
    if (text_color & COLOR_DEFAULT) 
        text_color = COLOR_DEFAULTT;          

    fprintf(console, "\e[");
    if (!(text_color & COLOR_NOCHANGE)){
        fprintf(console, text_color & COLOR_INTENSE ? "9%c" : "3%c", consoleColorNumber(text_color) + '0');
        if (!(background_color & COLOR_NOCHANGE)){
            fprintf(console, ";");
        }
    }
    if (!(background_color & COLOR_NOCHANGE)){
        if (background_color & COLOR_DEFAULT)
            fprintf(console, "49");
        else
            fprintf(console, background_color & COLOR_INTENSE ? "10%c" : "4%c", consoleColorNumber(background_color) + '0');
    }
    fprintf(console, "m");
    return 1;
}

void initConsole(){}

bool moveCursor(FILE* console, int dx, int dy){
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
}