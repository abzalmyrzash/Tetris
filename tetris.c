#include "tetris.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include "console.h"

uint16_t shapeGrids[NUM_PIECE_TYPES][NUM_ROTATIONS] = 
{
	// O_SHAPE (SQUARE)
	{
		0b0000011001100000,
		0b0000011001100000,
		0b0000011001100000,
		0b0000011001100000,
	},
	// I_SHAPE (LINE)
	{
		0b0000111100000000,
		0b0100010001000100,
		0b0000000011110000,
		0b0010001000100010,
	},
	// S_SHAPE
	{
		0b0000011000110000,
		0b0000001001100100,
		0b0000000001100011,
		0b0000000100110010
	},
	// Z_SHAPE
	{
		0b0000001101100000,
		0b0000010001100010,
		0b0000000000110110,
		0b0000001000110001
	},
	// L_SHAPE
	{
		0b0000010001110000,
		0b0000001000100110,
		0b0000000001110001,
		0b0000001100100010
	},
	// J_SHAPE
	{
		0b0000000101110000,
		0b0000011000100010,
		0b0000000001110100,
		0b0000001000100011
	},
	// T_SHAPE
	{
		0b0000001001110000,
		0b0000001001100010,
		0b0000000001110010,
		0b0000001000110010,
	}
};

void initGame(Game* game)
{
	for (int y = HEIGHT - 1; y >= 0; y--) {
		for (int x = 0; x < WIDTH; x++) {
			game->grid[y][x] = '.';
		}
	}
	game->nextShape = rand() % NUM_PIECE_TYPES;
	game->holdShape = NONE;
	game->canHold = true;
	game->locked = false;
	game->over = false;
	game->score = 0;
	game->fallTicks = 20;
	game->lockDelayTicks = 100;
	game->tickMicrosecs = 16667;
	game->ticksSinceLastFall = 0;
	game->fps = 0;
	
	spawnNextPiece(game);
}

void spawnPiece(Shape shape, Game* game)
{
	game->curPiece.shape = shape;
	game->curPiece.orient = R_0_DEG;
	game->curPiece.x = 3;
	game->curPiece.y = HEIGHT - 3;

	PosState pos = getCurrentPosState(game);
	if (pos == POS_INVALID) game->over = true;
	else if (pos == POS_GROUND) game->onGround = true;
	else game->onGround = false;
}

void getNextShape(Game* game)
{
	game->nextShape = rand() % NUM_PIECE_TYPES;
}

void spawnNextPiece(Game* game)
{
	spawnPiece(game->nextShape, game);
	getNextShape(game);
}

bool outOfBounds(int8_t x, int8_t y)
{
	return x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT;
}

PosState getPosState(int8_t pieceX, int8_t pieceY, Orientation orient, Game* game)
{
	int8_t x, y;
	uint16_t shapeGrid = shapeGrids[game->curPiece.shape][orient];
	int result = POS_FLOATING;
	for (int i = 0; i < 4; i++) {
		y = pieceY + i;
		for (int j = 0; j < 4; j++) {
			if (shapeGrid & 1) {
				x = pieceX + j;
				if (outOfBounds(x, y)) return POS_INVALID;
				if (game->grid[y][x] == LOCKED_BLOCK) return POS_INVALID;
				if (y == 0) result = POS_GROUND;
				if (game->grid[y - 1][x] == LOCKED_BLOCK) result = POS_GROUND;
			}
			shapeGrid >>= 1;
		}
	}
	return result;
}

PosState getCurrentPosState(Game* game)
{
	return getPosState(game->curPiece.x, game->curPiece.y, game->curPiece.orient, game);
}

PosState rotatePiece(RotationDirection dir, Game* game)
{
	Shape shape = game->curPiece.shape;
	if (shape == O_SHAPE) return getCurrentPosState(game);

	Orientation orient = game->curPiece.orient;
	Rotation rotation = (dir << 2) | orient;
	
	if (dir == CLOCKWISE) {
		orient++;
		if (orient == 4) {
			orient = R_0_DEG;
		}
	} else {
		orient--;
		if (orient < 0) {
			orient = R_270_DEG;
		}
	}

	int x = game->curPiece.x, y = game->curPiece.y;

	PosState result = getPosState(x, y, orient, game);
	if (result != POS_INVALID) {
		goto success;
	}

	static int8_t wallKick_I[8][4][2] = {
		{ {-2,  0}, {+1,  0}, {-2, -1}, {+1, +2} }, // 0->R
		{ {-1,  0}, {+2,  0}, {-1, +2}, {+2, -1} }, // R->2
		{ {+2,  0}, {-1,  0}, {+2, +1}, {-1, -2} }, // 2->L
		{ {+1,  0}, {-2,  0}, {+1, -2}, {-2, +1} }, // L->0
		{ {-1,  0}, {+2,  0}, {-1, +2}, {+2, -1} }, // 0->L
		{ {-2,  0}, {+1,  0}, {-2, -1}, {+1, +2} }, // L->2
		{ {+1,  0}, {-2,  0}, {+1, -2}, {-2, +1} }, // 2->R
		{ {+2,  0}, {-1,  0}, {+2, +1}, {-1, -2} }  // R->0
	};

	static int8_t wallKick_not_I[8][4][2] = {
		{ {-1,  0}, {-1, +1}, { 0, -2}, {-1, -2} }, // 0->R
		{ {+1,  0}, {+1, -1}, { 0, +2}, {+1, +2} }, // R->2
		{ {+1,  0}, {+1, +1}, { 0, -2}, {+1, -2} }, // 2->L
		{ {-1,  0}, {-1, -1}, { 0, +2}, {-1, +2} }, // L->0
		{ { 0,  0}, {+1,  0}, {+1, +1}, { 0, -2} }, // 0->L
		{ {-1,  0}, {-1, -1}, { 0, +2}, {-1, +2} }, // L->2
		{ {-1,  0}, {-1, +1}, { 0, -2}, {-1, -2} }, // 2->R
		{ {+1,  0}, {+1, -1}, { 0, +2}, {+1, +2} }  // R->0
	};

	int8_t (*wallKick)[4][2];
	if (shape == I_SHAPE) wallKick = wallKick_I;
	else wallKick = wallKick_not_I;

	for (int i = 0; i < 4; i++) {
		x = game->curPiece.x + wallKick[rotation][i][0];
		y = game->curPiece.y + wallKick[rotation][i][1];
		result = getPosState(x, y, orient, game);
		if (result != POS_INVALID) {
			game->curPiece.x = x;
			game->curPiece.y = y;
			goto success;
		}
	}

	return POS_INVALID;
	
	success:
		game->curPiece.orient = orient;
		if (result == POS_GROUND) game->onGround = true;
		else game->onGround = false;
		return result;
}

PosState movePiece(MoveDirection dir, Game* game)
{
	int8_t pieceX, pieceY;
	uint16_t shapeGrid;
	int8_t x, y;
	PosState result;

loop:
	switch(dir)
	{
	case DOWN:
		pieceX = game->curPiece.x;
		pieceY = game->curPiece.y - 1;
		break;
	case LEFT:
		pieceX = game->curPiece.x - 1;
		pieceY = game->curPiece.y;
		break;
	case RIGHT:
		pieceX = game->curPiece.x + 1;
		pieceY = game->curPiece.y;
		break;
	case SPACE:
		pieceX = game->curPiece.x;
		pieceY = game->curPiece.y - 1;
	default:
		break;
	}
	
	x, y;
	result = 0;

	result = getPosState(pieceX, pieceY, game->curPiece.orient, game);
	if (result == POS_INVALID) return result;
	else {
		game->curPiece.x = pieceX;
		game->curPiece.y = pieceY;
		if (dir == SPACE && result == 0) {
			goto loop;
		}
	}

	if (result == POS_GROUND) game->onGround = true;
	else game->onGround = false;
	return result;
}

uint8_t checkLines(Game* game) {
	uint8_t res = 0;
	for (int i = 0; i < 4; i++) {
		if (outOfBounds(0, game->curPiece.y + i)) continue;
		bool line = true;
		for (int x = 0; x < WIDTH; x++) {
			if (game->grid[game->curPiece.y + i][x] == EMPTY) {
				line = false;
				break;
			}
		}
		if (line) {
			res |= (1 << i);
		}
	}
	return res;
}

uint8_t popCount(uint8_t x) {
	uint8_t cnt = 0;
	if (x) do {
		cnt++;
	} while (x &= x - 1);

	return cnt;
}

void markLines(uint8_t lines, Game* game) {
	for (int i = 0; i < 4; i++) {
		if (lines & 1) {
			int y = game->curPiece.y + i;
			for (int x = 0; x < WIDTH; x++) {
				game->grid[y][x] = '-';
			}
		}
		lines >>= 1;
	}
}

void clearLines(uint8_t lines, Game* game) {
	for (int i = 0; i < 4; i++) {
		if (lines & 1) {
			int y = game->curPiece.y + i;
			for (int x = 0; x < WIDTH; x++) {
				game->grid[y][x] = EMPTY;
			}
		}
		lines >>= 1;
	}
}

void gravity(uint8_t lines, Game* game) {
	for (int i = 0; i < 4; i++) {
		int consecutive = 0;
		while(lines & 1) {
			lines >>= 1;
			consecutive++;
		}
		lines >>= 1;
		if (consecutive == 0) continue;

		int y0 = game->curPiece.y + i;
		for (int y = y0 + consecutive; y < HEIGHT; y++) {
			for (int x = 0; x < WIDTH; x++) {
				game->grid[y - consecutive][x] = game->grid[y][x];
			}
		}
		i += consecutive;
	}
}

void putPieceOnGrid(Game* game, char symbol)
{
	uint16_t shapeGrid = shapeGrids[game->curPiece.shape][game->curPiece.orient];
	int8_t pieceY = game->curPiece.y;
	int8_t pieceX = game->curPiece.x;
	for (int8_t i = 0; i < 4; i++) {
		int8_t y = pieceY + i;
		for (int8_t j = 0; j < 4; j++) {
			int8_t x = pieceX + j;
			if (shapeGrid & 1) {
				game->grid[y][x] = symbol;
			}
			shapeGrid >>= 1;
		}
	}
}

void clearPieceFromGrid(Game* game)
{
	putPieceOnGrid(game, EMPTY);
}

void printGrid(Game* game, short x0, short y0)
{
	if (!game->locked) putPieceOnGrid(game, CUR_BLOCK);
	COORD coord = {x0, y0};
	DWORD written;
	char c;
	FILE* file = fopen("grid.txt", "w");
	for (int8_t y = VISIBLE_HEIGHT - 1; y >= 0; y--) {
		WriteConsoleOutputCharacter(hConsole, game->grid[y], WIDTH, coord, &written);
		coord.Y++;
	}
	fclose(file);
	if (!game->locked) clearPieceFromGrid(game);
}

void holdPiece(Game* game)
{
	if (game->canHold == false) return;

	if (game->holdShape == NONE) {
		game->holdShape = game->curPiece.shape;
		spawnNextPiece(game);
	}
	else {
		Shape prevShape = game->curPiece.shape;
		spawnPiece(game->holdShape, game);
		game->holdShape = prevShape;
	}
	game->canHold = false;
}

void lockPiece(Game* game)
{
	game->locked = true;
	putPieceOnGrid(game, LOCKED_BLOCK);
	uint8_t lines = checkLines(game);
	if (lines) {
		markLines(lines, game);
		refreshScreen(game);
		Sleep(200);
		clearLines(lines, game);
		refreshScreen(game);
		Sleep(200);
		gravity(lines, game);
		int cnt = popCount(lines);
		game->score += 100 * cnt * cnt;
	}
	game->locked = false;

	spawnNextPiece(game);
	game->canHold = true;
}

void refreshScreen(Game* game)
{
    COORD coord;

	uint16_t shapeGrid = shapeGrids[game->nextShape][0];
	DWORD written;
	char c;

	for (int i = 3; i >= 0; i--) {
		coord.Y = i;
		for (int j = 0; j < 4; j++) {
			coord.X = 3 + j;
			if (shapeGrid & 1) {
				c = LOCKED_BLOCK;
				// printf("%c", LOCKED_BLOCK);
			} else {
				c = EMPTY;
				//printf(".");
			}
			WriteConsoleOutputCharacter(hConsole, &c, 1, coord, &written);
			shapeGrid >>= 1;
		}
	}

	coord.Y = 4;
	coord.X = 20;
	WriteConsoleOutputCharacter(hConsole, "HOLD", 4, coord, &written);
	if (game->holdShape == NONE) shapeGrid = 0;
	else shapeGrid = shapeGrids[game->holdShape][0];

	for (int i = 3; i >= 0; i--) {
		coord.Y = i + 5;
		for (int j = 0; j < 4; j++) {
			coord.X = 20 + j;
			if (shapeGrid & 1) {
				//printf("%c", LOCKED_BLOCK);
				c = LOCKED_BLOCK;
			} else {
				//printf(".");
				c = EMPTY;
			}
			WriteConsoleOutputCharacter(hConsole, &c, 1, coord, &written);
			shapeGrid >>= 1;
		}
	}

	printGrid(game, 0, 5);

	coord.Y = VISIBLE_HEIGHT + 5;
	coord.X = 0;
	WriteConsoleOutputCharacter(hConsole, "SCORE:", 6, coord, &written);

	coord.X = 7;
	char score[10];
	static int scoreLen = 0;
	WriteConsoleOutputCharacter(hConsole, "          ", scoreLen, coord, &written);
	scoreLen = sprintf(score, "%d", game->score);
	WriteConsoleOutputCharacter(hConsole, score, scoreLen, coord, &written);

	coord.X = 20;
	WriteConsoleOutputCharacter(hConsole, "FPS:", 4, coord, &written);

	coord.X = 25;
	char fps[10];
	static int fpsLen = 0;
	WriteConsoleOutputCharacter(hConsole, "          ", fpsLen, coord, &written);
	fpsLen = sprintf(fps, "%d", game->fps);
	WriteConsoleOutputCharacter(hConsole, fps, fpsLen, coord, &written);

	if (game->over) {
		coord.X = 0;
		coord.Y++;
		WriteConsoleOutputCharacter(hConsole, "GAME OVER!", 10, coord, &written);
	}
}
