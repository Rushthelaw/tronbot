#include <stdio.h>
#include <stdlib.h>
#include "Game.h"
#include "Player.h"
#include "GameTree.h"

// This function assumes grid and pos are taken from a player that's just been
// initialized. Therefore, we assume it will all work.
Player* init_player(int** grid, int gridx, int gridy, int** pos, int id,
        int (*nextMove)()) {
    Player* player = malloc(sizeof(Player));
    if(!player) {
        printf("Out of memory or something like that.\n");
        return NULL;
    }
    // Creating game tree
    Node* treeroot = new_node(NULL, copy_grid(grid, gridx, gridy),
            gridx, gridy, copy_grid(pos, 2, 2));
    player->tree = new_tree(treeroot);

    player->pos = pos;
    player->id = id;
    player->grid = grid;
    player->gridx = gridx;
    player->gridy = gridy;
    player->nextMove = nextMove;
    player->lost = 0;

    return player;
}

// Frees allocated memory when creating a Player
void destroy_player(Player* player) {
    free_tree(player->tree);
    free(player);
}

// Sets valid_moves to an array containing valid moves for player id
// Does not directly use player as argument because it will be needed elsewhere
// Have to check valid_moves after using this, it might be set to NULL
// C'est vraiment cancérigène ma façon de faire, mais je ne sais pas si je peux faire mieux
int get_validmoves(int** grid, int gridx, int gridy, int** pos,
        int id, int** valid_moves) {
    int moves[4] = {0};
    int num = 0;
    if (pos[id][0]-1 >= 0 && !grid[pos[id][0]-1][pos[id][1]]) {
        num++;
        moves[0] = 1;
    }
    if (pos[id][1]+1 < gridy && !grid[pos[id][0]][pos[id][1]+1]) {
        num++;
        moves[1]=1;
    }
    if (pos[id][0]+1 < gridx && !grid[pos[id][0]+1][pos[id][1]]) {
        num++;
        moves[2]=1;
    }
    if (pos[id][1]-1 >= 0 && !grid[pos[id][0]][pos[id][1]-1]) {
        num++;
        moves[3]=1;
    }
    if(!num) {
        valid_moves = NULL;
        return 0;
    }
    *valid_moves = malloc(num*sizeof(int));
    if(!*valid_moves) {
        printf("Out of memory.\n");
        return 0;
    }
    int count = 0;
    for (int i = 0; i<4; i++) {
        if (moves[i]) {
            (*valid_moves)[count] = i;
            count++;
        }
    }
    return num;
}

// Develops the next level of the game tree of player.
// C'est affreux
void expand_tree(Player* player) {
    // We evaluate which player plays on next level of tree
    player->tree->depth++;
    int to_move = (player->tree->depth + player->id) % 2;

    int total_num = 0;
    int num_elem;
    Node*** last_level = malloc(player->tree->num_nodes * sizeof(Node**));
    if (!last_level) {
        printf("Out of memory!\n");
        player->tree->depth--;
        return;
    }
    for (int i = 0; i<player->tree->num_nodes; i++) {
        num_elem = spawn_children(player->tree->last_level[i], to_move);
        printf("num_elem : %d\n", num_elem);
        last_level[i] = malloc(num_elem * sizeof(Node*));
        if (!last_level) {
            printf("Out of memory!\n");
            player->tree->depth--;
            return;
        }
        for (int j = 0; j<num_elem; j++) {
            last_level[i][j] = player->tree->last_level[i]->children[j];
        }
        total_num += num_elem;
    }
    Node** last = malloc(total_num * sizeof(Node*));
    int total = 0;
    printf("num_nodes : %d, n_child : \n", player->tree->num_nodes);
    for (int i = 0; i<player->tree->num_nodes; i++) {
        for (int j = 0; j<(player->tree->last_level[i])->n_child; j++) {
            //printf("%d, %d\n",i,j);
            last[total] = last_level[i][j];
            total++;
        }
    }

    player->tree->num_nodes = total_num;
    free(player->tree->last_level);
    player->tree->last_level = last;
}

// Cuts useless branches of tree and updates root.
// Updates the tree on level at a time (this has to be used after we select our
// move, and once more when we receive the move of opponent)
// Has to update last_level and i'm bored
void update_tree(Player* player, int move, int id) {
    Node* root = player->tree->root;
    int pos[2];
    if (move=0) {
        pos[0] = root->pos[id][0]--;
        pos[1] = root->pos[id][1];
    } else if (move=1) {
        pos[0] = root->pos[id][0];
        pos[1] = root->pos[id][1]++;
    } else if (move=2) {
        pos[0] = root->pos[id][0]++;
        pos[1] = root->pos[id][1];
    } else if (move=3) {
        pos[0] = root->pos[id][0];
        pos[1] = root->pos[id][1]--;
    }
    for (int i = 0; i<root->n_child; i++) {
        if ((root->children)[i]->pos[id][0] == pos[0] 
                && (root->children)[i]->pos[id][1] == pos[1]) {
            player->tree->root = root->children[i];
            root->children[i] = NULL;
            free_node(root);
            break;
        }
    }
    player->tree->depth--;
}

int next_randommove(Player* player) {
    struct timespec start, current;
    clock_gettime(CLOCK_REALTIME, &start);
}

// Returns the time elapsed since start at time current in nanoseconds.
// This function is only loosely precise.
// I think this implementation is not platform specific (not sure)
long elapsed_time(const struct timespec* start, const struct timespec* current) {
    return (current->tv_sec-start->tv_sec) *1E9 + (current->tv_nsec-start->tv_nsec);
}

// Keep in mind that time in nsec is cyclic
int get_time(void) {
    long a =0;
    struct timespec time1, time2;
    clockid_t clock = CLOCK_REALTIME;
    clock_gettime(clock, &time1);
    printf("time1 : %d\n", time1.tv_nsec);
    for (long i =0; i<10000000000; i++) {
        a++;
    }
    clock_gettime(clock, &time2);
    printf("time2 : %d\n", time2.tv_nsec);
    printf("t2-t1 = %d\n", time2.tv_nsec-time1.tv_nsec); 
    printf("t2-t1 = %d\n", time2.tv_sec-time1.tv_sec); 
    return 0;
}
