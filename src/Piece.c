#include "raylib/raylib.h"

#include "Types.h"
#include "Piece.h"
#include "ResourceManager.h"

static Rectangle pieceRect[] = {
    { 0, 0, 0, 0 },
    { 26, 21, 206, 206 },
    { 270, 6, 216, 229 },
    { 516, 7, 234, 234 },
    { 773, 25, 202, 202 },
    { 133, 248, 232, 221 },
    { 403, 241, 223, 231 },
    { 666, 255, 205, 205 },
};

void drawPiece( Piece *p, int padding ) {
    
    float escaleW = ( p->dim.x - padding * 2 ) / pieceRect[p->type].width;
    float escaleH = ( p->dim.y - padding * 2 ) / pieceRect[p->type].height;

    DrawTexturePro( 
        rm.pieces, 
        pieceRect[p->type], 
        (Rectangle) { 
            p->pos.x + padding,
            p->pos.y + padding,
            pieceRect[p->type].width * escaleW, 
            pieceRect[p->type].height * escaleH
        },
        (Vector2) { 0, 0 }, 
        0, 
        WHITE
    );

    /*if ( p->selected ) {
        DrawRectangleLines( p->pos.x, p->pos.y, p->dim.x, p->dim.y, BLACK );
    }*/

    if ( p->checked ) {
        DrawCircle( p->pos.x + 10, p->pos.y + 10, 5, WHITE );
    }

}