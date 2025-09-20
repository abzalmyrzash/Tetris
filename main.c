#include <stdio.h>
#include "tetris.h"
#include <stdlib.h>
#include <time.h>
#include "console.h"

#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77

#define CLOCKS_PER_MSEC (CLOCKS_PER_SEC / 1000)

int main() {
	srand(time(NULL));
	CONSOLE_FONT_INFOEX originalFont;
	saveConsoleFont(&originalFont);
	setConsoleFont(L"Consolas", 12, 12);
	setCursorVisibility(false);

	Game game;
	initGame(&game);

	int moveResult;
	system("cls");

	refreshScreen(&game);
	clock_t lastFall = clock();

	while (!game.over) {
		clearPieceFromGrid(&game);
		
		moveResult = 0;

		int timeSinceLastFall = (clock() - lastFall) / CLOCKS_PER_MSEC;

		if (timeSinceLastFall >= game.fallTime && !game.onGround) {
			lastFall = clock();
			moveResult = movePiece(DOWN, &game);
		}

		else if (timeSinceLastFall >= game.lockDelay && game.onGround) {
			lastFall = clock();
			lockPiece(&game);
			continue;
		}

		else {
			while (kbhit()) {
				uint8_t c = getch();

				if (c == ' ') {
					if (game.onGround) {
						lockPiece(&game);
						continue;
					}
					else moveResult = movePiece(SPACE, &game);
				}

				// Escape key
				else if (c == 27) {
					goto quit;
				}

				else if (c == 'z' || c == 'Z') {
					moveResult = rotatePiece(COUNTERCLOCKWISE, &game);
				}

				else if (c == 'c' || c == 'C') {
					holdPiece(&game);
				}

				// arrow keys are two characters,
				// where the first is one of these two values
				// the second is one of the 4 values in the switch statement 
				else if (c == 0 || c == 224) { 
					switch(getch())
					{
					case KEY_UP:
						moveResult = rotatePiece(CLOCKWISE, &game);
						break;
					case KEY_DOWN:
						moveResult = movePiece(DOWN, &game);
						break;
					case KEY_LEFT:
						moveResult = movePiece(LEFT, &game);
						break;
					case KEY_RIGHT:
						moveResult = movePiece(RIGHT, &game);
						break;
					default:
						break;
					}
				}
			}
		}

		putPieceOnGrid(&game);
		refreshScreen(&game);
		Sleep(game.frameDelay);
	}
	
	while (getch() != 27);

	quit:
	system("cls");
	restoreConsoleFont(&originalFont);
	setCursorVisibility(true);
	return 0;
}
