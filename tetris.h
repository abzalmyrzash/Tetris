#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <windows.h>

typedef enum : int8_t {
	O_SHAPE,
	I_SHAPE,
	S_ZHAPE,
	Z_SHAPE,
	J_SHAPE,
	L_SHAPE,
	T_SHAPE,
	NUM_PIECE_TYPES
} Shape;

typedef enum: int8_t {
	R_0_DEG,
	R_90_DEG,
	R_180_DEG,
	R_270_DEG,
	NUM_ROTATIONS
} Orientation;

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

typedef enum: int8_t {
	CLOCKWISE_FROM_0,
	CLOCKWISE_FROM_90,
	CLOCKWISE_FROM_180,
	CLOCKWISE_FROM_270,
	COUNTERCLOCKWISE_FROM_0,
	COUNTERCLOCKWISE_FROM_90,
	COUNTERCLOCKWISE_FROM_180,
	COUNTERCLOCKWISE_FROM_270
} Rotation;

typedef enum: int8_t {
	POS_INVALID = -1,
	POS_FLOATING = 0,
	POS_GROUND = 1
} PosState;

typedef struct {
	Shape shape;
	Orientation orient;
	int8_t x;
	int8_t y;
} Piece;

#define HEIGHT 22
#define VISIBLE_HEIGHT 20
#define WIDTH 10

#define EMPTY '.'
#define LOCKED_BLOCK '\xB2'
#define CUR_BLOCK 'O'
#define NONE -1

typedef struct {
	char grid[HEIGHT][WIDTH];
	Piece curPiece;
	Shape nextShape;
	Shape holdShape;
	bool canHold;
	bool onGround;
	bool locked;
	bool over;
	unsigned int score;
	uint8_t fallTicks;
	uint8_t lockDelayTicks;
	uint8_t ticksSinceLastFall;
	int tickMicrosecs;
	int fps;
} Game;

extern uint16_t shapeGrids[NUM_PIECE_TYPES][NUM_ROTATIONS];

void initGame(Game* game);

void spawnPiece(Shape shape, Game* game);

void getNextShape(Game* game);

void spawnNextPiece(Game* game);

void putPieceOnGrid(Game* game, char symbol);

void clearPieceFromGrid(Game* game);

PosState getPosState(int8_t pieceX, int8_t pieceY, Orientation orient, Game* game);

PosState getCurrentPosState(Game* game);

PosState rotatePiece(RotationDirection dir, Game* game);

PosState movePiece(MoveDirection dir, Game* game);

void printGrid(Game* game, short x0, short y0);

bool outOfBounds(int8_t x, int8_t y);

uint8_t checkLines(Game* game);

uint8_t popCount(uint8_t x);

void markLines(uint8_t lines, Game* game);

void clearLines(uint8_t lines, Game* game);

void gravity(uint8_t lines, Game* game);

void holdPiece(Game* game);

void lockPiece(Game* game);

void refreshScreen(Game* game);
