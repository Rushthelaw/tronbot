#include "Player.h"
//Declaration of the game structure and the game logic functions
#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

/* gridx is the x size of the grid
 * gridy is the y size of the grid
 * The grid is always a rectangle.
 * p1 and p2 are the players playing this game with id 0 and 1 respectively
 * grid is the game grid.
 * grid has to contain gridx pointers to pointers (arrays) of size gridy
 * pos contains the players positions
 * */
typedef struct {
    int gridx, gridy, is_over, p1lost, p2lost;
    Player* p1;
    Player* p2;
    int** grid;
    int** pos;
} Game;

Game* init_game(int gridx, int gridy, int** grid, int* p1, int* p2);

void destroy_game(Game* game);

void play_game(Game* game, int (*compute_p1)(Player*, int, int),
        int (*compute_p2)(Player*, int, int));

void print_grid(int** grid, int gridx, int gridy, int** pos);

void add_players(Game* game,int d1, int d2, double (*movep1)(), double (*movep2)());

void make_moves(Game* game, int m1, int m2);

int make_move(int** grid, int gridx, int gridy, int** pos, int id, int move);

int game_over(int** grid, int gridx, int gridy, int** pos);

int game_loser(int** grid, int gridx, int gridy, int** pos);

#endif
