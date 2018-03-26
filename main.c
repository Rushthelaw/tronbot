#include <stdio.h>
#include "Game.h"
#include "GameTree.h"
#include "Player.h"

int main(void) {
    int p1[2] = {1,1};
    int p2[2] = {0,0};
    Game* game = init_game(3, 4, NULL, p1, p2);
    Player* player1 = init_player(game->grid, game->gridx, game->gridy, game->pos, 0,
           next_randommove);
    print_grid(game->grid, game->gridx, game->gridy, game->pos);
    print_grid(player1->grid, player1->gridx, player1->gridy, player1->pos);
    make_moves(game, 1, 1);
    print_grid(game->grid, game->gridx, game->gridy, game->pos);
    print_grid(player1->grid, player1->gridx, player1->gridy, player1->pos);

    destroy_player(player1); 
    destroy_game(game);
    printf("Ended correctly\n");
}
