#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum : int8_t {
	SQUARE,
	LINE,
	S_ZHAPE,
	Z_SHAPE,
	J_SHAPE,
	L_SHAPE,
	T_SHAPE,
	NUM_BLOCK_TYPES
} PieceType;

typedef enum: int8_t {
	R_NORMAL,
	R_90_DEG,
	R_180_DEG,
	R_270_DEG,
	NUM_ROTATIONS
} Rotation;

typedef enum: int8_t {
	POS_INVALID = -1,
	POS_FLOATING = 0,
	POS_GROUND = 1
} PosState;

typedef struct {
	PieceType type;
	Rotation rotation;
	int8_t x;
	int8_t y;
} Piece;

#define HEIGHT 20
#define WIDTH 10

#define EMPTY '.'
#define BLOCK '\xB2'
#define NONE -1

typedef struct {
	char grid[HEIGHT][WIDTH];
	Piece curPiece;
	PieceType nextPieceType;
	PieceType holdType;
	bool canHold;
	bool onGround;
	bool locked;
	bool over;
	unsigned int score;
	int fallTime;
	int lockDelay;
	int frameDelay;
} Game;

typedef enum: int8_t {
	DOWN,
	LEFT,
	RIGHT,
	SPACE
} MoveDirection;

typedef enum : int8_t {
	CLOCKWISE,
	COUNTERCLOCKWISE
} RotationDirection;

extern uint16_t pieceGrids[NUM_BLOCK_TYPES][NUM_ROTATIONS];

void initGame(Game* game);

void spawnPiece(PieceType type, Game* game);

void getNextPieceType(Game* game);

void spawnNextPiece(Game* game);

bool putPieceOnGrid(Game* game);

void clearPieceFromGrid(Game* game);

PosState getPosState(int8_t pieceX, int8_t pieceY, Rotation rotation, Game* game);

PosState rotatePiece(RotationDirection dir, Game* game);

PosState movePiece(MoveDirection dir, Game* game);

void printGrid(Game* game);

bool outOfBounds(int8_t x, int8_t y);

uint8_t checkLines(Game* game);

uint8_t popCount(uint8_t x);

void markLines(uint8_t lines, Game* game);

void clearLines(uint8_t lines, Game* game);

void gravity(uint8_t lines, Game* game);

void holdPiece(Game* game);

void lockPiece(Game* game);

void refreshScreen(Game* game);
