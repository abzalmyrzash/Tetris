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

void refreshScreen(Game* game)
{
    HANDLE hOut;
    COORD Position;

    hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	setCursor(0, 0);

	uint16_t pieceGrid = pieceGrids[game->nextPieceType][0];

	for (int i = 3; i >= 0; i--) {
		setCursor(3, i);
		for (int j = 0; j < 4; j++) {
			if (pieceGrid & 1) {
				printf("%c", BLOCK);
			} else {
				printf(".");
			}
			pieceGrid >>= 1;
		}
	}

	setCursor (20, 4);
	printf("HOLD");
	if (game->holdType == NONE) pieceGrid = 0;
	else pieceGrid = pieceGrids[game->holdType][0];

	for (int i = 3; i >= 0; i--) {
		setCursor(20, i + 5);
		for (int j = 0; j < 4; j++) {
			if (pieceGrid & 1) {
				printf("%c", BLOCK);
			} else {
				printf(".");
			}
			pieceGrid >>= 1;
		}
	}

	setCursor(0, 5);

	printGrid(game);
	printf("SCORE: %d\n", game->score);

	if (game->over) {
		printf("GAME OVER!\n");
	}
	fflush(stdout);
}

int main() {
	srand(time(NULL));
	CONSOLE_FONT_INFOEX originalFont;
	saveConsoleFont(&originalFont);
	setConsoleFont(L"Consolas", 12, 12);
	setCursorVisibility(false);

	Game game;
	initGame(&game);

	int moveResult;
	int fallTime = 200;
	int delay = 10;
	system("cls");

	refreshScreen(&game);
	clock_t lastFall = clock();
	bool hitGround;

	while (!game.over) {
		// clear piece
		clearPieceFromGrid(&game);
		
		moveResult = 0;

		if ((clock() - lastFall) / CLOCKS_PER_MSEC >= fallTime) {
			lastFall = clock();
			moveResult = movePiece(DOWN, &game);
			if (moveResult == -1) moveResult = 2;
		}
		
		while (kbhit()) {
			uint8_t c = getch();

			if (c == ' ') {
				moveResult = movePiece(SPACE, &game);
			}

			else if (c == 27) {
				goto quit;
			}

			else if (c == 'z' || c == 'Z') {
				moveResult = rotatePiece(COUNTERCLOCKWISE, &game);
			}

			else if (c == 'c' || c == 'C') {
				holdPiece(&game);
			}

			else if (c == 0 || c == 224) {
				switch(getch())
				{
				case KEY_UP:
					moveResult = rotatePiece(CLOCKWISE, &game);
					break;
				case KEY_DOWN:
					moveResult = movePiece(DOWN, &game);
					break;
				case KEY_RIGHT:
					moveResult = movePiece(RIGHT, &game);
					break;
				case KEY_LEFT:
					moveResult = movePiece(LEFT, &game);
					break;
				default:
					break;
				}
			}
		}

		if (moveResult >= 1 || moveResult == -1) {
			putPieceOnGrid(&game);
			refreshScreen(&game);
			Sleep(delay);

			lastFall = clock();
			if (moveResult != 2) continue;

			uint8_t lines = checkLines(&game);
			if (lines) {
				markLines(lines, &game);
				refreshScreen(&game);
				Sleep(200);
				clearLines(lines, &game);
				Sleep(200);
				gravity(lines, &game);
				int cnt = popCount(lines);
				game.score += 100 * cnt * cnt;
			}

			spawnNextPiece(&game);
			hitGround = false;
		}

		if (!putPieceOnGrid(&game)) game.over = true;
		refreshScreen(&game);

		Sleep(delay);
	}
	
	while (getch() != 27);

	quit:
	system("cls");
	restoreConsoleFont(&originalFont);
	setCursorVisibility(true);
}
