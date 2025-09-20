#include "tetris.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "console.h"

uint16_t pieceGrids[NUM_BLOCK_TYPES][NUM_ROTATIONS] = 
{
	// SQUARE
	{
		0b0000011001100000,
		0b0000011001100000,
		0b0000011001100000,
		0b0000011001100000,
	},
	// LINE
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
	for (int y = 40; y >= 0; y--) {
		for (int x = 0 ; x < 10; x++) {
			game->grid[y][x] = '.';
		}
	}
	game->nextPieceType = rand() % NUM_BLOCK_TYPES;
	game->holdType = NONE;
	game->canHold = true;
	game->locked = false;
	game->over = false;
	game->score = 0;
	game->fallTime = 200;
	game->lockDelay = 1000;
	game->frameDelay = 10;
	
	spawnNextPiece(game);
	putPieceOnGrid(game);
}

void spawnPiece(PieceType type, Game* game)
{
	game->curPiece.type = type;
	game->curPiece.rotation = R_NORMAL;
	game->curPiece.x = 3;
	game->curPiece.y = HEIGHT - 3;

	PosState pos = getPosState(3, HEIGHT - 3, R_NORMAL, game);
	if (pos == POS_INVALID) game->over = true;
	else if (pos == POS_GROUND) game->onGround = true;
	else game->onGround = false;
}

void getNextPieceType(Game* game)
{
	game->nextPieceType = rand() % NUM_BLOCK_TYPES;
}

void spawnNextPiece(Game* game)
{
	spawnPiece(game->nextPieceType, game);
	getNextPieceType(game);
}

bool outOfBounds(int8_t x, int8_t y)
{
	return x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT;
}

PosState getPosState(int8_t pieceX, int8_t pieceY, Rotation rotation, Game* game)
{
	int8_t x, y;
	uint16_t pieceGrid = pieceGrids[game->curPiece.type][rotation];
	int result;
	for (int i = 0; i < 4; i++) {
		y = pieceY + i;
		for (int j = 0; j < 4; j++) {
			if (pieceGrid & 1) {
				x = pieceX + j;
				if (outOfBounds(x, y)) return POS_INVALID;
				if (game->grid[y][x] != EMPTY) return POS_INVALID;
				if (y == 0) result = POS_GROUND;
				if (game->grid[y - 1][x] != EMPTY) result = POS_GROUND;
			}
			pieceGrid >>= 1;
		}
	}
	return result;
}

PosState rotatePiece(RotationDirection dir, Game* game)
{
	Rotation rotation = game->curPiece.rotation;
	if (dir == CLOCKWISE) {
		rotation++;
		if (rotation == 4) {
			rotation = R_NORMAL;
		}
	} else {
		rotation--;
		if (rotation < 0) {
			rotation = R_270_DEG;
		}
	}

	PosState result = getPosState(game->curPiece.x, game->curPiece.y, rotation, game);
	if (result == POS_INVALID) return result;
	else {
		game->curPiece.rotation = rotation;
		if (result == 1) game->onGround = true;
		else game->onGround = false;
		return result;
	}
}

PosState movePiece(MoveDirection dir, Game* game)
{
	int8_t pieceX, pieceY;
	uint16_t pieceGrid;
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

	result = getPosState(pieceX, pieceY, game->curPiece.rotation, game);
	if (result == POS_INVALID) return result;
	else {
		game->curPiece.x = pieceX;
		game->curPiece.y = pieceY;
		if (dir == SPACE && result == 0) {
			goto loop;
		}

		if (result == 1) game->onGround = true;
		else game->onGround = false;
		return result;
	}

	if (result == 1) game->onGround = true;
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

bool putPieceOnGrid(Game* game)
{
	uint16_t pieceGrid = pieceGrids[game->curPiece.type][game->curPiece.rotation];
	int8_t pieceY = game->curPiece.y;
	int8_t pieceX = game->curPiece.x;
	for (int8_t i = 0; i < 4; i++) {
		int8_t y = pieceY + i;
		for (int8_t j = 0; j < 4; j++) {
			int8_t x = pieceX + j;
			if (pieceGrid & 1) {
				if (game->grid[y][x] != EMPTY) return false;
				game->grid[y][x] = '\xB2';
			}
			pieceGrid >>= 1;
		}
	}
	return true;
}

void clearPieceFromGrid(Game* game)
{
	uint16_t pieceGrid = pieceGrids[game->curPiece.type][game->curPiece.rotation];
	int8_t pieceY = game->curPiece.y;
	int8_t pieceX = game->curPiece.x;
	for (int8_t i = 0; i < 4; i++) {
		int8_t y = pieceY + i;
		for (int8_t j = 0; j < 4; j++) {
			int8_t x = pieceX + j;
			if (pieceGrid & 1) {
				game->grid[y][x] = EMPTY;
			}
			pieceGrid >>= 1;
		}
	}
}

void printGrid(Game* game)
{
	if (!game->locked) putPieceOnGrid(game);
	for (int y = HEIGHT - 1; y >= 0; y--) {
		for (int x = 0; x < WIDTH; x++) {
			printf("%c", game->grid[y][x]);
		}
		printf("\n");
	}
	if (!game->locked) clearPieceFromGrid(game);
}

void holdPiece(Game* game)
{
	if (game->canHold == false) return;

	if (game->holdType == NONE) {
		game->holdType = game->curPiece.type;
		spawnNextPiece(game);
	}
	else {
		PieceType prevType = game->curPiece.type;
		spawnPiece(game->holdType, game);
		game->holdType = prevType;
	}
	game->canHold = false;
}

void lockPiece(Game* game)
{
	game->locked = true;
	putPieceOnGrid(game);
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

	refreshScreen(game);
	Sleep(game->frameDelay);
}

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
}
