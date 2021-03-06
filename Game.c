#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Game.h"
#include "Player.h"
#include "GameTree.h"

// Initiales a Game and returns a pointer to it
// This returns NULL if it fails.
Game* init_game(int gridx, int gridy, int** grid, int p1[2], int p2[2]) {
    // This first section makes sure the arguments are legal
    if (p1[0] >= gridx || p1[0] < 0) {
        printf("p1[0] is %d and is not in grid!\n", p1[0]);
        return NULL;
    }
    if (p1[1] >= gridy || p1[1] < 0) {
        printf("p1[1] is %d and is not in grid!\n", p1[1]);
        return NULL;
    }
    if (p2[0] >= gridx || p2[0] < 0) {
        printf("p2[0] is %d and is not in grid!\n", p2[0]);
        return NULL;
    }
    if (p2[1] >= gridx || p2[1] < 0) {
        printf("p2[1] is %d and is not in grid!\n", p2[1]);
        return NULL;
    }

    Game* game = malloc(sizeof(Game));
    if(!game) {
        printf("Out of memory or something like that.\n");
        return NULL;
    }
    game->gridx = gridx;
    game->gridy = gridy;
    game->is_over = 0;
    game->p1lost = 0;
    game->p2lost = 0;

    game->grid = malloc(gridx * sizeof(int*));
    if(!(game->grid)) {
        free(game);
        printf("Out of memory.\n");
        return NULL;
    }
    for (int i = 0; i<gridx; i++) {
        game->grid[i] = malloc(gridy * sizeof(int));
        if(!(game->grid[i])) {
            for (int j = 0; j<i; j++) {
                free(game->grid[j]);
            }
            free(game->grid);
            free(game);
            printf("Out of memory.\n");
            return NULL;
        }
    }
    if(grid) {
        for (int i = 0; i<gridx; i++) {
            for (int j = 0; j<gridy; j++) {
                game->grid[i][j] = grid[i][j];
            }
        }
    } else {
        for (int i = 0; i<gridx; i++) {
            for (int j = 0; j<gridy; j++) {
                game->grid[i][j] = 0;
            }
        }
    }
    game->pos = malloc(2 * sizeof(int*));
    if(!game->pos) {
        free(game);
        printf("Out of memory.\n");
        return NULL;
    }
    for (int i = 0; i<2; i++) {
        game->pos[i] = malloc(2 * sizeof(int));
        if(!(game->pos[i])) {
            for (int j = 0; j<i; j++) {
                free(game->pos[j]);
            }
            for (int j = 0; j<gridx; j++) {
                free(game->grid[j]);
            }
            free(game->grid);
            free(game->pos);
            free(game);
            printf("Out of memory.\n");
            return NULL;
        }
    }
    for (int i = 0; i<2; i++) {
        game->pos[0][i] = p1[i];
        game->pos[1][i] = p2[i];
    }
    return game;
}

// Frees allocated memory when creating a Game
void destroy_game(Game* game) {
    if (!game) return;
    if (game->p1) destroy_player(game->p1);
    if (game->p2) destroy_player(game->p2);
    for (int i = 0; i<game->gridx; i++) {
        free(game->grid[i]);
    }
    free(game->grid);
    for (int i = 0; i<2; i++) {
        free(game->pos[i]);
    }
    free(game->pos);
    free(game);
}

// This plays game until the end
// This needs to be redone because i will change Player API to consider the time
// limit
int play_game(Game* game, int (*compute_p1)(Player*, int, int),
        int (*compute_p2)(Player*, int, int), double** times){
    Player* p1 = game->p1;
    Player* p2 = game->p2;
    int last1, last2, temp, num_moves = 0;
    struct timespec start, current;
    last1 = last2 = -1;

    Tree* tree = game->p1->tree;
    while (!(game->is_over)) {
        clock_gettime(CLOCK_REALTIME, &start);
        temp = compute_p1(p1, last1, last2);
        clock_gettime(CLOCK_REALTIME, &current);
        times[0][num_moves] = elapsed_time(&start, &current) /1000000000.0;
        clock_gettime(CLOCK_REALTIME, &start);
        last2 = compute_p2(p2, last2, last1);
        clock_gettime(CLOCK_REALTIME, &current);
        times[1][num_moves] = elapsed_time(&start, &current) /1000000000.0;
        last1 = temp;
        make_moves(game, last1, last2);
        //print_grid(game->grid, game->gridx, game->gridy, game->pos);
        num_moves++;
    }
    return num_moves;
}

// Prints the grid.
// This does not check if the game is over, it might not work as intended if it is
// Bottom left corner is x = y = 0, x is horizontal, y is vertical
// This is solely for debugging, I have no intention for the game to be playable
void print_grid(int** grid, int gridx, int gridy, int** pos) {
    for (int j = gridy-1; j>=0; j--) {
        for (int i = 0; i<gridx; i++) {
            if (pos[0][0]==i && pos[0][1]==j) {
                printf("p1 ");
                if (pos[1][0]==i && pos[1][1]==j) {
                    printf("p2 ");
                } else {
                    printf("   ");
                }
                continue;
            }
            if (pos[1][0]==i && pos[1][1]==j) {
                printf("p2    ");
                continue;
            }
            printf("%d     ", grid[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Creates players for game.
// d1 and d2 are the initial directions
void add_players(Game* game, int d1, int d2, double (*movep1)(), double (*movep2)()) {
    game->p1 = init_player(game->grid, game->gridx, game->gridy, game->pos, 0, 
            d1, d2, movep1);
    if(!game->p1) {
        printf("Could not create p1.\n");
        return;
    }
    game->p2 = init_player(game->grid, game->gridx, game->gridy, game->pos, 1,
            d1, d2, movep2);
    if(!game->p2) {
        printf("Could not create p2.\n");
        return;
    }
}

// Makes move m1 for player 1 and move m2 for player 2
// Possible moves are 0 (go left), 1 (go up), 2 (go right) and 3 (go down)
// Checks if game is over
void make_moves(Game* game, int m1, int m2) {
    if (game->is_over) {
        printf("Game is over, no need to make moves!\n");
    }
    
    if(m1>3 || m1<0 || make_move(game->grid,
                game->gridx, game->gridy, game->pos, 0, m1)) {
        game->is_over = 1;
        game->p1lost = 1;
    }
    if(m2>3 || m2<0 || make_move(game->grid,
                game->gridx, game->gridy, game->pos, 1, m2)) {
        game->is_over = 1;
        game->p2lost = 1;
    } 
    if (!game->p1lost && game->grid[game->pos[0][0]][game->pos[0][1]] == 1) {
        game->is_over = 1;
        game->p1lost = 1;
    }
    if (!game->p2lost && game->grid[game->pos[1][0]][game->pos[1][1]] == 1) {
        game->is_over = 1;
        game->p2lost = 1;
    }
    if ((game->pos[0][0] == game->pos[1][0]) 
            && (game->pos[0][1] == game->pos[1][1])) {
        game->is_over = 1;
        game->p1lost = 1;
        game->p2lost = 1;
    }
}

// Makes move for player on grid
// Change the value of grid, then we move the player.
// Checks for out of bounds moves
int make_move(int** grid, int gridx, int gridy, int** pos, int id, int move) {
    grid[pos[id][0]][pos[id][1]] = 1;
    if (move == 0) {
        pos[id][0]--;
        if (pos[id][0]<0) return 1;
    } else if (move == 1) {
        pos[id][1]++;
        if (pos[id][1]>=gridy) return 1;
    }
    else if (move == 2) {
        pos[id][0]++;
        if (pos[id][0]>=gridx) return 1;
    }
    else if (move == 3) {
        pos[id][1]--;
        if (pos[id][1]<0) return 1;
    }
    return 0;
}

// Checks if game specified by the parameters is over
int game_over(int** grid, int gridx, int gridy, int** pos) {
    if(pos[0][0] < 0
            || pos[0][1] < 0
            || pos[0][0] >= gridx
            || pos[0][1] >= gridy
            || pos[1][0] < 0
            || pos[1][1] < 0
            || pos[1][0] >= gridx
            || pos[1][1] >= gridy
            || (pos[0][0] == pos[1][0] && pos[0][1] == pos[1][1])
            || grid[pos[0][0]][pos[0][1]]
            || grid[pos[1][0]][pos[1][1]]) {
        return 1;
    }
    return 0;
}

// Returns 0 if p1 lost, 1 if p2 lost and 2 of both lost
// This assumes the game is over. This will return a valid value even if it is
// not.
int game_loser(int** grid, int gridx, int gridy, int** pos) {
    if (pos[0][0] < 0
            || pos[0][1] < 0
            || pos[0][0] >= gridx
            || pos[0][1] >= gridy
            || grid[pos[0][0]][pos[0][1]]) {
        if (pos[1][0] < 0
                || pos[1][1] < 0
                || pos[1][0] >= gridx
                || pos[1][1] >= gridy
                || grid[pos[1][0]][pos[1][1]]) return 2;
        return 0;
    }
    if (pos[1][0] < 0
            || pos[1][1] < 0
            || pos[1][0] >= gridx
            || pos[1][1] >= gridy
            || grid[pos[1][0]][pos[1][1]]) return 1;
    return 2;
}
