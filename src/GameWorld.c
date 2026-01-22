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

#define LIST_CAPACITY 100

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

static Position positionlist[LIST_CAPACITY];
static int positionListSize = 0;

static FallingPiece animationList[LIST_CAPACITY];
static int animationListSize = 0;
static const float BASE_FALL_SPEED = 100;
static float fallSpeed = 0;
static const float GRAVITY = 2000;

static int linear[] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    4, 1, 1, 1, 1, 1, 4, 1,
    4, 1, 1, 1, 1, 4, 3, 1,
    3, 4, 3, 4, 3, 1, 4, 1,
    4, 1, 4, 3, 4, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 4, 1, 1, 1,
    1, 1, 4, 4, 3, 4, 1, 1,
};

static bool checkValidityAndCommitChanges( GameWorld *gw, int r1, int c1, int r2, int c2 );
static void buildGrid( GameWorld *gw, int *pieces );

static void positionListAdd( int row, int col );
static void positionListClear( void );
static void positionListUncheckAndClear( GameWorld *gw );

static void animationListAdd( Piece *p, float targetY );
static void animationListClear( void );

static void resetGrid( GameWorld *gw ) {
    buildGrid( gw, NULL );
    //buildGrid( gw, linear );
    gw->state = GAME_STATE_PLAYING;
    positionListClear();
    animationListClear();
}

/**
 * @brief Creates a dinamically allocated GameWorld struct instance.
 */
GameWorld* createGameWorld( void ) {

    GameWorld *gw = (GameWorld*) calloc( 1, sizeof( GameWorld ) );
    gw->background = (Color){ 80, 49, 47, 255 };
    gw->detail = (Color){ 75, 45, 47, 255 };
    gw->pieceSize = GetScreenWidth() / GRID_WIDTH;
    gw->pieceMargin = 1;
    gw->state = GAME_STATE_PLAYING;
    
    resetGrid( gw );

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

    if ( IsKeyPressed( KEY_R ) ) {
        resetGrid( gw );
    }

    if ( gw->state == GAME_STATE_PLAYING ) {   

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

            if ( !checkValidityAndCommitChanges( gw, r2, c2, selectedRow, selectedCol ) ) {

                // rollback changes if not valid
                //TraceLog( LOG_INFO, "rolling back..." );

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

    if ( gw->state == GAME_STATE_DROPPING_NEW_PIECES ) {
        int ok = 0;
        for ( int i = 0; i < animationListSize; i++ ) {
            if ( animationList[i].piece->pos.y < animationList[i].targetY ) {
                animationList[i].piece->pos.y += fallSpeed * delta;
            } else {
                animationList[i].piece->pos.y = animationList[i].targetY;
                ok++;
            }
        }
        if ( ok == animationListSize ) {
            animationListClear();
            gw->state = GAME_STATE_PLAYING;
        }
        fallSpeed += GRAVITY * delta;
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

static bool checkLinear( GameWorld *gw, int row, int col ) {

    Piece (*grid)[GRID_HEIGHT] = gw->grid;

    // horizontal
    int count = 1;

    for ( int c = col + 1; c < GRID_WIDTH; c++ ) {
        if ( grid[row][c].type == grid[row][col].type ) {
            count++;
            grid[row][c].checked = true;
            positionListAdd( row, c );
        } else {
            break;
        }
    }

    for ( int c = col - 1; c >= 0; c-- ) {
        if ( grid[row][c].type == grid[row][col].type ) {
            count++;
            grid[row][c].checked = true;
            positionListAdd( row, c );
        } else {
            break;
        }
    }

    //TraceLog( LOG_INFO, "horizontal count: %d", count );

    if ( count >= 3 ) {
        grid[row][col].checked = true;
        positionListAdd( row, col );
        return true;
    }
    positionListUncheckAndClear( gw );

    // vertical
    count = 1;

    for ( int r = row + 1; r < GRID_HEIGHT; r++ ) {
        if ( grid[r][col].type == grid[row][col].type ) {
            count++;
            grid[r][col].checked = true;
            positionListAdd( r, col );
        } else {
            break;
        }
    }

    for ( int r = row - 1; r >= 0; r-- ) {
        if ( grid[r][col].type == grid[row][col].type ) {
            count++;
            grid[r][col].checked = true;
            positionListAdd( r, col );
        } else {
            break;
        }
    }

    //TraceLog( LOG_INFO, "vertical count: %d", count );

    if ( count >= 3 ) {
        grid[row][col].checked = true;
        positionListAdd( row, col );
        return true;
    }
    positionListUncheckAndClear( gw );

    return false;

}

static bool checkValidityAndCommitChanges( GameWorld *gw, int r1, int c1, int r2, int c2 ) {

    Piece (*grid)[GRID_HEIGHT] = gw->grid;

    // 1) check if the jewel type are different
    if ( grid[r1][c1].type == grid[r2][c2].type ) {
        return false;
    }

    /*
     * 2) disparar o algoritmo para tanto a peça selecionada quanto a movida,
     *    marcando as joias para remoção.
     * 3) verificar cada um dos casos:
     *    a) em linha, vertical e horizontal, 3 ou mais joias;
     *    b) em T, 4 ou mais joias;
     *    c) em L, 5 ou mais joias;
     *    d) em cruz, 5 joias;
     *    e) quadrado 2x2
     */
    bool linearFound1 = checkLinear( gw, r1, c1 );
    positionListClear();
    bool linearFound2 = checkLinear( gw, r2, c2 );
    positionListClear();

    bool matched = linearFound1 || linearFound2;

    // se houve match:
    if ( matched ) {

        // 1) remover as peças;
        for ( int i = 0; i < GRID_HEIGHT; i++ ) {
            for ( int j = 0; j < GRID_WIDTH; j++ ) {
                if ( gw->grid[i][j].checked ) {
                    gw->grid[i][j] = (Piece) {
                        .type = PIECE_NULL,
                        .pos = { j * gw->pieceSize, i * gw->pieceSize },
                        .dim = { gw->pieceSize, gw->pieceSize },
                        .selected = false,
                        .checked = false
                    };
                }
            }
        }

        // 2) fazer as peças caírem;

        // inserção na animação
        for ( int i = 0; i < GRID_HEIGHT; i++ ) {
            for ( int j = 0; j < GRID_WIDTH; j++ ) {
                int emptyCount = 0;
                if ( gw->grid[i][j].type != PIECE_NULL ) {
                    for ( int k = i+1; k < GRID_HEIGHT; k++ ) {
                        if ( gw->grid[k][j].type == PIECE_NULL ) {
                            emptyCount++;
                        }
                    }
                    if ( emptyCount > 0 ) {
                        animationListAdd( &gw->grid[i+emptyCount][j], gw->grid[i+emptyCount][j].pos.y );
                    }
                }
            }
        }

        // troca "física" (no grid)
        int newPieces[GRID_WIDTH] = {0};

        for ( int i = 0; i < GRID_HEIGHT; i++ ) {
            for ( int j = 0; j < GRID_WIDTH; j++ ) {
                if ( gw->grid[i][j].type == PIECE_NULL ) {
                    newPieces[j]++;
                    for ( int k = i; k > 0; k-- ) {
                        Piece p = gw->grid[k][j];
                        gw->grid[k][j] = gw->grid[k-1][j];
                        gw->grid[k-1][j] = p;
                    }
                }
            }
        }

        // 3) gerar novas peças;
        for ( int j = 0; j < GRID_WIDTH; j++ ) {
            //TraceLog( LOG_INFO, "%d", newPieces[j] );
            for ( int k = 0; k < newPieces[j]; k++ ) {
                gw->grid[k][j] = (Piece) {
                    .type = GetRandomValue( 1, 7 ),
                    .pos = { j * gw->pieceSize, ( k - newPieces[j] ) * gw->pieceSize },
                    .dim = { gw->pieceSize, gw->pieceSize },
                    .selected = false,
                    .checked = false
                };
                animationListAdd( &gw->grid[k][j], k * gw->pieceSize );
            }
        }

        gw->state = GAME_STATE_DROPPING_NEW_PIECES;
        fallSpeed = BASE_FALL_SPEED;

        // 4) realizar a verificação para cada peça, pois podem ser criados
        //    novos matches ao preencher o tabuleiro.

    }

    return matched;

}

static void buildGrid( GameWorld *gw, int *pieces ) {

    for ( int i = 0; i < GRID_HEIGHT; i++ ) {
        for ( int j = 0; j < GRID_WIDTH; j++ ) {
            gw->grid[i][j] = (Piece) {
                .type = pieces == NULL ? GetRandomValue( 1, 7 ) : pieces[i*GRID_WIDTH+j],
                .pos = { j * gw->pieceSize, i * gw->pieceSize },
                .dim = { gw->pieceSize, gw->pieceSize },
                .selected = false,
                .checked = false
            };
        }
    }

}

static void positionListAdd( int row, int col ) {
    if ( positionListSize < LIST_CAPACITY ) {
        positionlist[positionListSize++] = (Position) { row, col };
    }
}

static void positionListClear( void ) {
    positionListSize = 0;
}

static void positionListUncheckAndClear( GameWorld *gw ) {

    if ( positionListSize > 0 ) {
        for ( int i = 0; i < positionListSize; i++ ) {
            gw->grid[positionlist[i].row][positionlist[i].col].checked = false;
        }
    }

    positionListClear();

}

static void animationListAdd( Piece *p, float targetY ) {
    if ( animationListSize < LIST_CAPACITY ) {
        animationList[animationListSize++] = (FallingPiece) { p, targetY };
    }
}

static void animationListClear( void ) {
    animationListSize = 0;
}