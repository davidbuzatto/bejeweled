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
    PIECE_WHITE,
} PieceType;

typedef struct Piece {
    PieceType type;
    Vector2 pos;
    Vector2 dim;
    bool selected;
} Piece;