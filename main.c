#include <stdio.h>
#include <stdlib.h>
#include "Game.h"
#include "GameTree.h"
#include "Player.h"
#include "lfsr113.h"

// Generates a x times y symmetric grid
// I don't care that this does not generate every possible grid or if the grid
// is a forced tie
void generate_grid(int*** grid, int x, int y, int*** pos, int* direction) {
    if (*grid || *pos) {
        printf("Containers are not empty!\n");
        return;
    }
    *grid = malloc(x * sizeof(int*));
    *pos = malloc(2*sizeof(int*));
    if (!grid || !pos) {
        printf("Out of memory\n");
        return;
    }
    for (int i = 0; i<x; i++) {
        (*grid)[i] = malloc(y*sizeof(int));
        if (!(*grid)[i]) {
            printf("Out of memory\n");
            return;
        }
    }
    (*pos)[0] = malloc(2*sizeof(int));
    (*pos)[1] = malloc(2*sizeof(int));
    if (!(*pos)[0] || !(*pos)[1]) {
        printf("Out of memory\n");
        return;
    }
    for (int i = 0; i<x; i++) {
        for (int j = 0; j<y; j++) {
            (*grid)[i][j] = 0;
        }
    }
    
    double distance = -1.0;
    int halfx = x/2;
    int halfy = y/2;
    int reversey;
    int square;
    while (distance == -1.0) {
        reversey = (int)(lfsr113()*2);
        direction[0] = (int)(lfsr113()*4);
        if (direction[0]%2) {
            if (reversey) direction[1] = (direction[0]+2)%4;
            else direction[1] = direction[0];
        } else {
            direction[1] = (direction[0]+2)%4;
        }

        (*pos)[0][0] = (int)(lfsr113()*halfx);
        (*pos)[0][1] = (int)(lfsr113()*halfy);
        (*pos)[1][0] = x - 1 - (*pos)[0][0];
        if (reversey) (*pos)[1][1] = y - 1 - (*pos)[0][1];
        else (*pos)[1][1] = (*pos)[0][1];
        if (reversey) {
            for (int i=0; i<halfx; i++) {
                for (int j = 0; j<halfy; j++) {
                    if (i == (*pos)[0][0] && j == (*pos)[0][1]) {
                        (*grid)[i][j] = 0;
                        (*grid)[x-1-i][y-1-j] = 0;
                        continue;
                    }
                    square = (int)(lfsr113() * 1.2);
                    (*grid)[i][j] = square;
                    (*grid)[x-1-i][y-1-j] = square;
                    (*grid)[x-1-i][j] = square;
                    (*grid)[i][y-1-j] = square;
                }
            }
        } else {
            for (int i=0; i<halfx; i++) {
                for (int j = 0; j<y; j++) {
                    if (i == (*pos)[0][0] && j == (*pos)[0][1]) {
                        (*grid)[i][j] = 0;
                        (*grid)[x-1-i][j] = 0;
                        continue;
                    }
                    square = (int)(lfsr113() * 1.2);
                    (*grid)[i][j] = square;
                    (*grid)[x-1-i][j] = square;
                }
            }

        }
        distance = p_distance(*grid, x, y, *pos);
    }
}

void play_games(int num_games, int x, int y, double (*eval_p1)(),
        double (*eval_p2)(), int (*compute_p1)(), int (*compute_p2)()) {
    int** grid = NULL;
    int** pos = NULL;
    int win1, win2, draw, num_moves, total_moves = 0;
    double** times;
    double t1 = 0.0, t2 = 0.0;
    int direction[2];
    Game* game;
    times = malloc(2*sizeof(double*));
    if (!times) {
        printf("Out of memory\n");
        return;
    }
    times[0] = malloc((x*y/2+1)*sizeof(double));
    times[1] = malloc((x*y/2+1)*sizeof(double));
    if (!times[0] || !times[1]) {
        printf("Out of memory\n");
        free(times);
        return;
    }
    
    win1 = win2 = draw = 0;
    for (int i = 0; i<num_games; i++) {
        generate_grid(&grid, x, y, &pos, direction);
        game = init_game(x, y, grid, pos[0], pos[1]);
        add_players(game, direction[0], direction[1], eval_p1, eval_p2);
        num_moves = play_game(game, compute_p1, compute_p2, times);
        if (game->p1lost && !game->p2lost) win2++;
        else if (game->p2lost && !game->p1lost) win1++;
        else draw++;
        destroy_game(game);
        for (int i = 0; i<x; i++) {
            free(grid[i]);
        }
        free(grid);
        grid = NULL;
        free(pos[0]); free(pos[1]); free(pos); pos = NULL;

        for (int j = 0; j<num_moves; j++) {
            t1 += times[0][j];
            t2 += times[1][j];
            total_moves++;
        }
    }
    printf("Average time p1 : %f\n", t1/total_moves);
    printf("Average time p2 : %f\n", t2/total_moves);
    printf("Wins p1 : %d\n", win1);
    printf("Wins p2 : %d\n", win2);
    printf("p1 - p2 : %d\n", win1 - win2);
    printf("Draw    : %d\n", draw);
    free(times[0]); free(times[1]); free(times);
}


int main(void) {
    int num_games = 40, x = 5, y = 5;

    printf("Resolutions exactes\n");
    printf("Random next vs Exact\n");
    play_games(num_games, x, y, randommove, randommove, compute_exact, compute_next);
    printf("\nExact vs Random ab\n");
    play_games(num_games, x, y, randommove, randommove, compute_exact, compute_ab);
    printf("\nExact vs Random MC\n");
    play_games(num_games, x, y, randommove, randommove, compute_exact, compute_MC);
    printf("\nExact vs Voronoi next\n");
    play_games(num_games, x, y, voronoi_dist, voronoi_dist, compute_exact, compute_next);
    printf("\nExact vs Voronoi ab\n");
    play_games(num_games, x, y, voronoi_dist, voronoi_dist, compute_exact, compute_ab);
    printf("\nExact vs Voronoi MC\n");
    play_games(num_games, x, y, voronoi_dist, voronoi_dist, compute_exact, compute_MC);
    printf("\nExact vs Combo next\n");
    play_games(num_games, x, y, combo_score, combo_score, compute_exact, compute_next);
    printf("\nExact vs Combo ab\n");
    play_games(num_games, x, y, combo_score, combo_score, compute_exact, compute_ab);
    printf("\nExact vs Combo MC\n");
    play_games(num_games, x, y, combo_score, combo_score, compute_exact, compute_MC);

    num_games = 25;
    x = y = 10; 
    printf("\nResolutions non exactes\n");
    printf("\nContre soi-meme\n");
    printf("\nRandom next vs Random ab\n");
    play_games(num_games, x, y, randommove, randommove, compute_next, compute_ab);
    printf("\nRandom next vs Random MC\n");
    play_games(num_games, x, y, randommove, randommove, compute_next, compute_MC);
    printf("\nRandom ab vs Random MC\n");
    play_games(num_games, x, y, randommove, randommove, compute_ab, compute_MC);
    printf("\nVoronoi next vs Voronoi ab\n");
    play_games(num_games, x, y, voronoi_dist, voronoi_dist, compute_next, compute_ab);
    printf("\nVoronoi next vs Voronoi MC\n");
    play_games(num_games, x, y, voronoi_dist, voronoi_dist, compute_next, compute_MC);
    printf("\nVoronoi ab vs Voronoi MC\n");
    play_games(num_games, x, y, voronoi_dist, voronoi_dist, compute_ab, compute_MC);
    printf("\nCombo next vs Combo ab\n");
    play_games(num_games, x, y, combo_score, combo_score, compute_next, compute_ab);
    printf("\nCombo next vs Combo MC\n");
    play_games(num_games, x, y, combo_score, combo_score, compute_next, compute_MC);
    printf("\nCombo ab vs Combo MC\n");
    play_games(num_games, x, y, combo_score, combo_score, compute_ab, compute_MC);

    printf("\nContre aleatoire\n");
    printf("\nRandom next vs Voronoi next\n");
    play_games(num_games, x, y, randommove, voronoi_dist, compute_next, compute_next);
    printf("\nRandom next vs Voronoi ab\n");
    play_games(num_games, x, y, randommove, voronoi_dist, compute_next, compute_ab);
    printf("\nRandom next vs Voronoi MC\n");
    play_games(num_games, x, y, randommove, voronoi_dist, compute_next, compute_MC);
    printf("\nRandom next vs combo next\n");
    play_games(num_games, x, y, randommove, combo_score, compute_next, compute_next);
    printf("\nRandom next vs combo ab\n");
    play_games(num_games, x, y, randommove, combo_score, compute_next, compute_ab);
    printf("\nRandom next vs Combo MC\n");
    play_games(num_games, x, y, randommove, combo_score, compute_next, compute_MC);

    printf("\nRandom ab vs Voronoi next\n");
    play_games(num_games, x, y, randommove, voronoi_dist, compute_ab, compute_next);
    printf("\nRandom ab vs Voronoi ab\n");
    play_games(num_games, x, y, randommove, voronoi_dist, compute_ab, compute_ab);
    printf("\nRandom ab vs Voronoi MC\n");
    play_games(num_games, x, y, randommove, voronoi_dist, compute_ab, compute_MC);
    printf("\nRandom ab vs combo next\n");
    play_games(num_games, x, y, randommove, combo_score, compute_ab, compute_next);
    printf("\nRandom ab vs combo ab\n");
    play_games(num_games, x, y, randommove, combo_score, compute_ab, compute_ab);
    printf("\nRandom ab vs Combo MC\n");
    play_games(num_games, x, y, randommove, combo_score, compute_ab, compute_MC);

    printf("\nRandom MC vs Voronoi next\n");
    play_games(num_games, x, y, randommove, voronoi_dist, compute_MC, compute_next);
    printf("\nRandom MC vs Voronoi ab\n");
    play_games(num_games, x, y, randommove, voronoi_dist, compute_MC, compute_ab);
    printf("\nRandom MC vs Voronoi MC\n");
    play_games(num_games, x, y, randommove, voronoi_dist, compute_MC, compute_MC);
    printf("\nRandom MC vs combo next\n");
    play_games(num_games, x, y, randommove, combo_score, compute_MC, compute_next);
    printf("\nRandom MC vs combo ab\n");
    play_games(num_games, x, y, randommove, combo_score, compute_MC, compute_ab);
    printf("\nRandom MC vs Combo MC\n");
    play_games(num_games, x, y, randommove, combo_score, compute_MC, compute_MC);

    printf("\nContre Voronoi\n");
    printf("\nVoronoi next vs combo next\n");
    play_games(num_games, x, y, voronoi_dist, combo_score, compute_next, compute_next);
    printf("\nVoronoi next vs combo ab\n");
    play_games(num_games, x, y, voronoi_dist, combo_score, compute_next, compute_ab);
    printf("\nVoronoi next vs Combo MC\n");
    play_games(num_games, x, y, voronoi_dist, combo_score, compute_next, compute_MC);
    printf("\nVoronoi ab vs combo next\n");
    play_games(num_games, x, y, voronoi_dist, combo_score, compute_ab, compute_next);
    printf("\nVoronoi ab vs combo ab\n");
    play_games(num_games, x, y, voronoi_dist, combo_score, compute_ab, compute_ab);
    printf("\nVoronoi ab vs Combo MC\n");
    play_games(num_games, x, y, voronoi_dist, combo_score, compute_ab, compute_MC);
    printf("\nVoronoi MC vs combo next\n");
    play_games(num_games, x, y, voronoi_dist, combo_score, compute_MC, compute_next);
    printf("\nVoronoi MC vs combo ab\n");
    play_games(num_games, x, y, voronoi_dist, combo_score, compute_MC, compute_ab);
    printf("\nVoronoi MC vs Combo MC\n");
    play_games(num_games, x, y, voronoi_dist, combo_score, compute_MC, compute_MC);

    return 0;
}
