#pragma once
#include <windows.h>
#include <conio.h>
#include <stdbool.h>

extern HANDLE hConsole;

void setCursor(short x, short y);

void getCursor(short *x, short *y);

void setConsoleFont(const wchar_t* fontName, SHORT width, SHORT height);

void saveConsoleFont(CONSOLE_FONT_INFOEX *savedFont);

void restoreConsoleFont(CONSOLE_FONT_INFOEX *savedFont);

void setCursorVisibility(bool visible);

void setConsoleColor(WORD attributes);

