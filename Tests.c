#include <stdio.h>
#include <stdlib.h>
#include "Game.h"
#include "GameTree.h"
#include "Player.h"

int main(void) {
    int p1[2] = {1,1};
    int p2[2] = {0,0};

    int num;
    int* moves;

    Game* game = init_game(3, 4, NULL, p1, p2);
    add_players(game, next_randommove, next_randommove);

    print_grid(game->grid, game->gridx, game->gridy, game->pos);
    print_grid(game->p1->grid, game->p1->gridx, game->p1->gridy, game->p1->pos);
    num = get_validmoves(game->grid, game->gridx, game->gridy, game->pos, 0, &moves);
    for (int i = 0; i<num; i++) {
        printf("M%d : %d\n", i, moves[i]);
    }
    free(moves);
    num = get_validmoves(game->grid, game->gridx, game->gridy, game->pos, 1, &moves);
    for (int i = 0; i<num; i++) {
        printf("M%d : %d\n", i, moves[i]);
    }
    free(moves);

    expand_tree(game->p1);
    expand_tree(game->p2);
    //spawn_children(game->p1->tree->root, 0);
    //spawn_children(game->p2->tree->root, 1);
    printf("P1 tree\n");
    Tree* tree = game->p1->tree;
    for (int i = 0; i<tree->root->n_child; i++) {
        printf("i: %d\n");
        print_grid((tree->root->children)[i]->grid, game->gridx, game->gridy, (tree->root->children)[i]->pos);
    }
    printf("P2 tree\n");
    tree = game->p2->tree;
    for (int i = 0; i<tree->root->n_child; i++) {
        printf("i: %d\n");
        print_grid((tree->root->children)[i]->grid, game->gridx, game->gridy, (tree->root->children)[i]->pos);
    }

    make_moves(game, 1, 1);
    print_grid(game->grid, game->gridx, game->gridy, game->pos);
    print_grid(game->p1->grid, game->p1->gridx, game->p1->gridy, game->p1->pos);
    num = get_validmoves(game->grid, game->gridx, game->gridy, game->pos, 0, &moves);
    for (int i = 0; i<num; i++) {
        printf("M%d : %d\n", i, moves[i]);
    }
    free(moves);
    num = get_validmoves(game->grid, game->gridx, game->gridy, game->pos, 1, &moves);
    for (int i = 0; i<num; i++) {
        printf("M%d : %d\n", i, moves[i]);
    }
    free(moves);

    destroy_game(game);
    printf("Ended correctly\n");
}

/* Methods to test

void expand_tree(Player* player);

void update_tree(Player* player, int move, int id);

int next_randommove(Player* player);

long elapsed_time(const struct timespec* start, const struct timespec* current);

*/
