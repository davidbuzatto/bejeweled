#pragma once

#include <stdbool.h>

typedef enum PieceType {
    PIECE_NULL,
    PIECE_RED,
    PIECE_ORANGE,
    PIECE_YELLOW,
    PIECE_GREEN,
    PIECE_BLUE,
    PIECE_PINK,
    PIECE_WHITE
} PieceType;

typedef enum GameState {
    GAME_STATE_PLAYING,
    GAME_STATE_DROPPING_NEW_PIECES
} GameState;

typedef struct Piece {
    PieceType type;
    Vector2 pos;
    Vector2 dim;
    bool selected;
    bool checked;
    bool debug;
} Piece;

typedef struct FallingPiece {
    Piece *piece;
    float targetY;
} FallingPiece;

typedef struct Position {
    int row;
    int col;
} Position;