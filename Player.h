//Declaration of the player structure and all of the related functions
//Also contains the declaration of decision functions
#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED
#include <time.h>
#include "GameTree.h"

// Total time allowed for a move in nanoseconds
// To be thinkered with
#define TTPLAY 900000000

/* Probably need to add a few things like the player id
 * Have to fix get_validmoves to check if the other player is in the next space
 *
 * to implement : -expand tree (develop the next level of tree)
 *                -update tree (cut parts of tree after a move is done)
 * */

/* lost indicates that the player has lost
 * grid is a 2d array representing the game grid
 * gridx and gridy are the grid dimensions. Needed to compute possible moves
 * id is the player id (0 for p1 and 1 for p2), can be used to indice pos
 * pos are the x and y coordinates of the players on grid
 * gametree contains the previous calculations made (will probably somewhat of a doublylinked list)
 * */
typedef struct {
    int lost, gridx, gridy, id;
    Tree* tree;
    int** grid;
    int** pos;
    int (*nextMove)();
} Player;

Player* init_player(int** grid, int gridx, int gridy, int** pos, int id,
        int (*nextMove)());

void destroy_player(Player* player);

int get_validmoves(int** grid, int gridx, int gridy, int** pos,
        int id, int** valid_moves);

void expand_tree(Player* player);

void update_tree(Player* player, int move, int id);

int next_randommove(Player* player);

long elapsed_time(const struct timespec* start, const struct timespec* current);

#endif
