#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "tetris.h"
#include "console.h"
#include "time_util.h"

#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77

#define CLOCKS_PER_MSEC (CLOCKS_PER_SEC / 1000)

HANDLE hConsole;


int main() {
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE timer;
	if(!(timer = CreateWaitableTimer(NULL, TRUE, NULL))) {
		return -1;
	}
	
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

	HANDLE sleepTimer;
    if(!(sleepTimer = CreateWaitableTimer(NULL, TRUE, NULL))) {
		return -1;
	}

	ZenTimer_Init();
	const int64_t zen_ticks_per_second = getZenTicksPerSecond();
	const int64_t zen_ticks_per_microsecond = getZenTicksPerMicrosecond();
	zen_timer_t frameTimer;
	zen_timer_t timer1s = ZenTimer_Start();
	int64_t frameTime = 0;
	int64_t time1s = 0;
	int fps = 0;

	while (!game.over) {
		frameTimer = ZenTimer_Start();

		moveResult = 0;

		if (game.ticksSinceLastFall >= game.fallTicks && !game.onGround) {
			game.ticksSinceLastFall = 0;
			moveResult = movePiece(DOWN, &game);
		}

		else if (game.ticksSinceLastFall >= game.lockDelayTicks && game.onGround) {
			lockPiece(&game);
			game.ticksSinceLastFall = 0;
		}

		else {
			while (kbhit()) {
				uint8_t c = getch();

				if (c == ' ') {
					if (game.onGround) {
						lockPiece(&game);
						game.ticksSinceLastFall = 0;
					}
					else {
						moveResult = movePiece(SPACE, &game);
						game.ticksSinceLastFall = 0;
					}
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
					game.ticksSinceLastFall = 0;
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
						if (moveResult != POS_INVALID) {
							game.ticksSinceLastFall = 0;
						}
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

		refreshScreen(&game);

		time1s = ZenTimer_End(&timer1s);
		fps++;
		if (time1s >= 1e6) {
			timer1s.start.QuadPart += zen_ticks_per_second;
			game.fps = fps;
			fps = 0;
		}

		frameTime += ZenTimer_End(&frameTimer);
		if (frameTime < game.tickMicrosecs) {
			frameTimer = ZenTimer_Start();
			nanoSleep((game.tickMicrosecs - frameTime) * 10, timer); // 1 unit = 100 nanoseconds
			frameTime += ZenTimer_End(&frameTimer);
		}
		frameTime -= game.tickMicrosecs;

		game.ticksSinceLastFall++;
	}
	
	while (getch() != 27);

	quit:
	system("cls");
	restoreConsoleFont(&originalFont);
	setCursorVisibility(true);
    CloseHandle(sleepTimer);
	return 0;
}
