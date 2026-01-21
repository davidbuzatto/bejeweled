/**
 * @file GameWorld.h
 * @author Prof. Dr. David Buzatto
 * @brief GameWorld implementation.
 * 
 * @copyright Copyright (c) 2026
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "GameWorld.h"
#include "ResourceManager.h"
#include "Piece.h"

#include "raylib/raylib.h"
//#include "raylib/raymath.h"
//#define RAYGUI_IMPLEMENTATION    // to use raygui, comment these three lines.
//#include "raylib/raygui.h"       // other compilation units must only include
//#undef RAYGUI_IMPLEMENTATION     // raygui.h

static int selectedCol;
static int selectedRow;

static Piece *selectedPiece = NULL;
static Vector2 pressOffset;
static Vector2 pressPos;
static Vector2 mousePos;

static Piece *leftNeighbor;
static Piece *rightNeighbor;
static Piece *topNeighbor;
static Piece *downNeighbor;
static Piece *beingSwapped;

static bool checkValidityAndCommitChanges( GameWorld *gw );

/**
 * @brief Creates a dinamically allocated GameWorld struct instance.
 */
GameWorld* createGameWorld( void ) {

    GameWorld *gw = (GameWorld*) calloc( 1, sizeof( GameWorld ) );
    gw->background = (Color){ 80, 49, 47, 255 };
    gw->detail = (Color){ 75, 45, 47, 255 };
    gw->pieceSize = GetScreenWidth() / GRID_WIDTH;
    gw->pieceMargin = 1;

    for ( int i = 0; i < GRID_HEIGHT; i++ ) {
        for ( int j = 0; j < GRID_WIDTH; j++ ) {
            gw->grid[i][j] = (Piece) {
                .type = GetRandomValue( 1, 7 ),
                .pos = { j * gw->pieceSize, i * gw->pieceSize },
                .dim = { gw->pieceSize, gw->pieceSize },
                .selected = false
            };
        }
    }

    return gw;

}

/**
 * @brief Destroys a GameWindow object and its dependecies.
 */
void destroyGameWorld( GameWorld *gw ) {
    free( gw );
}

/**
 * @brief Reads user input and updates the state of the game.
 */
void updateGameWorld( GameWorld *gw, float delta ) {

    if ( selectedPiece == NULL ) {

        if ( IsMouseButtonPressed( MOUSE_BUTTON_LEFT ) ) {

            pressPos = GetMousePosition();
            mousePos = pressPos;

            selectedCol = pressPos.x / gw->pieceSize;
            selectedRow = pressPos.y / gw->pieceSize;

            selectedPiece = &gw->grid[selectedRow][selectedCol];
            selectedPiece->selected = true;

            pressOffset.x = pressPos.x - selectedPiece->pos.x;
            pressOffset.y = pressPos.y - selectedPiece->pos.y;

            int leftCol = selectedCol - 1;
            int rightCol = selectedCol + 1;
            int topRow = selectedRow - 1;
            int downRow = selectedRow + 1;

            leftNeighbor = leftCol >= 0 ? &gw->grid[selectedRow][leftCol] : NULL;
            rightNeighbor = rightCol < GRID_WIDTH ? &gw->grid[selectedRow][rightCol] : NULL;
            topNeighbor = topRow >= 0 ? &gw->grid[topRow][selectedCol] : NULL;
            downNeighbor = downRow < GRID_HEIGHT ? &gw->grid[downRow][selectedCol] : NULL;

            if ( leftNeighbor != NULL ) {
                leftNeighbor->selected = true;
                leftNeighbor->pos.x = ( selectedCol - 1 ) * selectedPiece->dim.x;
                leftNeighbor->pos.y = selectedRow * selectedPiece->dim.x;
            }

            if ( rightNeighbor != NULL ) {
                rightNeighbor->selected = true;
                rightNeighbor->pos.x = ( selectedCol + 1 ) * selectedPiece->dim.x;
                rightNeighbor->pos.y = selectedRow * selectedPiece->dim.x;
            }

            if ( topNeighbor != NULL ) {
                topNeighbor->selected = true;
                topNeighbor->pos.x = selectedCol * selectedPiece->dim.y;
                topNeighbor->pos.y = ( selectedRow - 1 ) * selectedPiece->dim.y;
            }

            if ( downNeighbor != NULL ) {
                downNeighbor->selected = true;
                downNeighbor->pos.x = selectedCol * selectedPiece->dim.y;
                downNeighbor->pos.y = ( selectedRow + 1 ) * selectedPiece->dim.y;
            }

        }

    } else {

        mousePos = GetMousePosition();

        selectedPiece->pos.x = mousePos.x - pressOffset.x;
        selectedPiece->pos.y = mousePos.y - pressOffset.y;

        float xDiff = mousePos.x - pressPos.x;
        float yDiff = mousePos.y - pressPos.y;

        if ( fabs( xDiff ) >= fabs( yDiff ) ) {
            selectedPiece->pos.y = selectedRow * selectedPiece->dim.y;
        } else {
            selectedPiece->pos.x = selectedCol * selectedPiece->dim.x;
        }

        if ( leftNeighbor != NULL ) {
            if ( selectedPiece->pos.x < ( selectedCol - 1 ) * selectedPiece->dim.x ) {
                selectedPiece->pos.x = ( selectedCol - 1 ) * selectedPiece->dim.x;
            }
        } else {
            if ( selectedPiece->pos.x < 0 ) {
                selectedPiece->pos.x = 0;
            }
        }

        if ( rightNeighbor != NULL ) {
            if ( selectedPiece->pos.x > ( selectedCol + 1 ) * selectedPiece->dim.x ) {
                selectedPiece->pos.x = ( selectedCol + 1 ) * selectedPiece->dim.x;
            }
        } else {
            if ( selectedPiece->pos.x + selectedPiece->dim.x > GetScreenWidth() ) {
                selectedPiece->pos.x = GetScreenWidth() - selectedPiece->dim.x;
            }
        }

        if ( topNeighbor != NULL ) {
            if ( selectedPiece->pos.y < ( selectedRow - 1 ) * selectedPiece->dim.y ) {
                selectedPiece->pos.y = ( selectedRow - 1 ) * selectedPiece->dim.y;
            }
        } else {
            if ( selectedPiece->pos.y < 0 ) {
                selectedPiece->pos.y = 0;
            }
        }

        if ( downNeighbor != NULL ) {
            if ( selectedPiece->pos.y > ( selectedRow + 1 ) * selectedPiece->dim.y ) {
                selectedPiece->pos.y = ( selectedRow + 1 ) * selectedPiece->dim.y;
            }
        } else {
            if ( selectedPiece->pos.y + selectedPiece->dim.y > GetScreenHeight() ) {
                selectedPiece->pos.y = GetScreenHeight() - selectedPiece->dim.y;
            }
        }

        if ( leftNeighbor != NULL ) {
            leftNeighbor->pos.x = ( selectedCol - 1 ) * selectedPiece->dim.x;
            leftNeighbor->pos.y = selectedRow * selectedPiece->dim.x;
        }

        if ( rightNeighbor != NULL ) {
            rightNeighbor->pos.x = ( selectedCol + 1 ) * selectedPiece->dim.x;
            rightNeighbor->pos.y = selectedRow * selectedPiece->dim.x;
        }

        if ( topNeighbor != NULL ) {
            topNeighbor->pos.x = selectedCol * selectedPiece->dim.y;
            topNeighbor->pos.y = ( selectedRow - 1 ) * selectedPiece->dim.y;
        }

        if ( downNeighbor != NULL ) {
            downNeighbor->pos.x = selectedCol * selectedPiece->dim.y;
            downNeighbor->pos.y = ( selectedRow + 1 ) * selectedPiece->dim.y;
        }

        if ( fabs( xDiff ) >= fabs( yDiff ) ) {
            float xOffset = selectedPiece->pos.x - selectedCol * selectedPiece->dim.x;
            if ( xDiff < 0 ) {
                if ( leftNeighbor != NULL ) {
                    leftNeighbor->pos.x = ( selectedCol - 1 ) * selectedPiece->dim.x - xOffset;
                    beingSwapped = leftNeighbor;
                }
            } else if ( xDiff > 0 ) {
                if ( rightNeighbor != NULL ) {
                    rightNeighbor->pos.x = ( selectedCol + 1 ) * selectedPiece->dim.x - xOffset;
                    beingSwapped = rightNeighbor;
                }
            } else {
                beingSwapped = NULL;
            }
        } else {
            float yOffset = selectedPiece->pos.y - selectedRow * selectedPiece->dim.y;
            if ( yDiff < 0 ) {
                if ( topNeighbor != NULL ) {
                    topNeighbor->pos.y = ( selectedRow - 1 ) * selectedPiece->dim.y - yOffset;
                    beingSwapped = topNeighbor;
                }
            } else if ( yDiff > 0 ) {
                if ( downNeighbor != NULL ) {
                    downNeighbor->pos.y = ( selectedRow + 1 ) * selectedPiece->dim.y - yOffset;
                    beingSwapped = downNeighbor;
                }
            } else {
                beingSwapped = NULL;
            }
        }


    }

    if ( IsMouseButtonReleased( MOUSE_BUTTON_LEFT ) ) {

        if ( selectedPiece != NULL ) {
            selectedPiece->selected = false;
            selectedPiece->pos.x = selectedCol * selectedPiece->dim.x;
            selectedPiece->pos.y = selectedRow * selectedPiece->dim.y;
        }

        if ( leftNeighbor != NULL ) {
            leftNeighbor->selected = false;
            leftNeighbor->pos.x = ( selectedCol - 1 ) * selectedPiece->dim.x;
            leftNeighbor->pos.y = selectedRow * selectedPiece->dim.x;
        }

        if ( rightNeighbor != NULL ) {
            rightNeighbor->selected = false;
            rightNeighbor->pos.x = ( selectedCol + 1 ) * selectedPiece->dim.x;
            rightNeighbor->pos.y = selectedRow * selectedPiece->dim.x;
        }

        if ( topNeighbor != NULL ) {
            topNeighbor->selected = false;
            topNeighbor->pos.x = selectedCol * selectedPiece->dim.y;
            topNeighbor->pos.y = ( selectedRow - 1 ) * selectedPiece->dim.y;
        }

        if ( downNeighbor != NULL ) {
            downNeighbor->selected = false;
            downNeighbor->pos.x = selectedCol * selectedPiece->dim.y;
            downNeighbor->pos.y = ( selectedRow + 1 ) * selectedPiece->dim.y;
        }

        if ( beingSwapped != NULL ) {

            int r2 = 0;
            int c2 = 0;

            if ( beingSwapped == leftNeighbor ) {
                r2 = selectedRow;
                c2 = selectedCol - 1;
            } else if ( beingSwapped == rightNeighbor ) {
                r2 = selectedRow;
                c2 = selectedCol + 1;
            } else if ( beingSwapped == topNeighbor ) {
                r2 = selectedRow - 1;
                c2 = selectedCol;
            } else if ( beingSwapped == downNeighbor ) {
                r2 = selectedRow + 1;
                c2 = selectedCol;
            }

            Vector2 p1 = gw->grid[selectedRow][selectedCol].pos;
            Vector2 p2 = gw->grid[r2][c2].pos;

            gw->grid[selectedRow][selectedCol].pos = p2;
            gw->grid[r2][c2].pos = p1;

            Piece p = gw->grid[selectedRow][selectedCol];
            gw->grid[selectedRow][selectedCol] = gw->grid[r2][c2];
            gw->grid[r2][c2] = p;

            if ( !checkValidityAndCommitChanges( gw ) ) {

                // rollback changes if not valid
                p1 = gw->grid[selectedRow][selectedCol].pos;
                p2 = gw->grid[r2][c2].pos;

                gw->grid[selectedRow][selectedCol].pos = p2;
                gw->grid[r2][c2].pos = p1;

                p = gw->grid[selectedRow][selectedCol];
                gw->grid[selectedRow][selectedCol] = gw->grid[r2][c2];
                gw->grid[r2][c2] = p;

            }
            
        }

        selectedPiece = NULL;
        leftNeighbor = NULL;
        rightNeighbor = NULL;
        topNeighbor = NULL;
        downNeighbor = NULL;
        beingSwapped = NULL;

    }

}

/**
 * @brief Draws the state of the game.
 */
void drawGameWorld( GameWorld *gw ) {

    BeginDrawing();
    ClearBackground( gw->background );

    for ( int i = 0; i < GRID_HEIGHT; i++ ) {
        for ( int j = 0; j < GRID_WIDTH; j++ ) {
            if ( ( i + j ) % 2 != 0 ) {
                DrawRectangleRounded( 
                    (Rectangle) {
                        j*gw->pieceSize + gw->pieceMargin, 
                        i*gw->pieceSize + gw->pieceMargin, 
                        gw->pieceSize - gw->pieceMargin * 2, 
                        gw->pieceSize - gw->pieceMargin * 2
                    },
                    0.2,
                    10,
                    gw->detail
                );
            }
        }
    }

    for ( int i = 0; i < GRID_HEIGHT; i++ ) {
        for ( int j = 0; j < GRID_WIDTH; j++ ) {
            Piece *p = &gw->grid[i][j];
            if ( p != selectedPiece ) {
                drawPiece( p, 6 );
            }
        }
    }

    if ( selectedPiece != NULL ) {
        drawPiece( selectedPiece, 6 );
    }

    EndDrawing();

}

static bool checkValidityAndCommitChanges( GameWorld *gw ) {

    /*
     * 1) checar se o tipo das peças são diferentes;
     * 2) disparar o algoritmo para tanto a peça selecionada quanto a movida;
     * 3) verificar cada um dos casos:
     *    a) em linha, vertical e horizontal, 3 ou mais joias;
     *    b) em T, 4 ou mais joias;
     *    c) em L, 5 ou mais joias;
     *    d) em T, 4 ou mais joias;
     *    e) em cruz, 5 joias;
     *    f) quadrado 2x2
     */

    /* 
     * se houver match:
     * 1) marcar peças para remoção;
     * 2) remover as peças;
     * 3) fazer as peças caírem;
     * 4) gerar novas peças
     * 5) realizar a verificação para cada peça, pois podem ser criados
     *    novos matches ao preencher o tabuleiro.
     */


    return true;

}