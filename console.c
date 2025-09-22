#include "console.h"

void setCursor(short x, short y) {
    COORD coord = {x, y};
    SetConsoleCursorPosition(hConsole, coord);
}

void getCursor(short *x, short *y) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        *x = csbi.dwCursorPosition.X;
        *y = csbi.dwCursorPosition.Y;
    } else {
        *x = -1;
        *y = -1;
    }
}

void setConsoleFont(const wchar_t* fontName, SHORT width, SHORT height) {
    CONSOLE_FONT_INFOEX cfi = {0};
    cfi.cbSize = sizeof(cfi);
    cfi.dwFontSize.X = width;
    cfi.dwFontSize.Y = height;
    wcscpy_s(cfi.FaceName, LF_FACESIZE, fontName);
    SetCurrentConsoleFontEx(hConsole, FALSE, &cfi);
}

void saveConsoleFont(CONSOLE_FONT_INFOEX *savedFont) {
    savedFont->cbSize = sizeof(CONSOLE_FONT_INFOEX);
    GetCurrentConsoleFontEx(hConsole, FALSE, savedFont);
}

void restoreConsoleFont(CONSOLE_FONT_INFOEX *savedFont) {
    SetCurrentConsoleFontEx(hConsole, FALSE, savedFont);
}

void setCursorVisibility(bool visible) {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);  // Get current cursor info

    cursorInfo.bVisible = visible;  // Hide cursor
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void setConsoleColor(WORD attributes) {
    SetConsoleTextAttribute(hConsole, attributes);
}

