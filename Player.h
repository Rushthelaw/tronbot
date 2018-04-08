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
 * tree contains the previous calculations made (will probably somewhat of a doublylinked list)
 * last_move is the current direction of player (you can't go back)
 * */
/* node_value can't return the value -100.0 as of now*/
typedef struct {
    int lost, gridx, gridy, id, last1, last2;
    Tree* tree;
    int** grid;
    int** pos;
    double (*node_value)(Node*);
} Player;

Player* init_player(int** grid, int gridx, int gridy, int** pos, int id,
        int last1, int last2, double (*node_value)(Node*));

void destroy_player(Player* player);

//int get_validmoves(int** grid, int gridx, int gridy, int** pos,
//        int id, int** valid_moves);

void expand_tree(Player* player);

void update_tree(Player* player, int move, int id);

void evaluate_node(Node* node, Player* player, int minmax);

int choose_next(Player* player);

int compute_next(Player* player, int last_self, int last_op);

double randommove(Node* node);

double voronoi_dist(Node* node);

double controller(int i, int j, int** grid, int gridx, int gridy, int** pos);

long elapsed_time(const struct timespec* start, const struct timespec* current);

#endif
