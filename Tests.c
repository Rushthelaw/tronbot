#include <stdio.h>
#include <stdlib.h>
#include "Game.h"
#include "GameTree.h"
#include "Player.h"

int main(void) {
    int p1[2] = {5,5};
    int p2[2] = {1,1};

    /*
    int num;
    int* moves;

    Game* game = init_game(7, 7, NULL, p1, p2);
    add_players(game, 1, 1, randommove, randommove);

    print_grid(game->grid, game->gridx, game->gridy, game->pos);
    print_grid(game->p1->grid, game->p1->gridx, game->p1->gridy, game->p1->pos);
    // Testing getvalidmoves
    //num = get_validmoves(game->grid, game->gridx, game->gridy, game->pos, 0, &moves);
    //for (int i = 0; i<num; i++) {
    //    printf("M%d : %d\n", i, moves[i]);
    //}
    //free(moves);
    //num = get_validmoves(game->grid, game->gridx, game->gridy, game->pos, 1, &moves);
    //for (int i = 0; i<num; i++) {
    //    printf("M%d : %d\n", i, moves[i]);
    //}
    //free(moves);

    // Testing expand_tree
    expand_tree(game->p1);
    expand_tree(game->p2);
    expand_tree(game->p1);
    expand_tree(game->p2);
    //spawn_children(game->p1->tree->root, 0);
    //spawn_children(game->p2->tree->root, 1);

    // printing trees
    printf("after 2 expand\n");
    printf("P1 tree\n");
    Tree* tree = game->p1->tree;
    for (int i = 0; i<tree->num_nodes; i++) {
        if (!tree->last_level[i]) continue;
        printf("i: %d\n", i);
        print_grid(tree->last_level[i]->grid, game->gridx, game->gridy, tree->last_level[i]->pos);
    }
    printf("P2 tree\n");
    tree = game->p2->tree;
    for (int i = 0; i<tree->num_nodes; i++) {
        if (!tree->last_level[i]) continue;
        printf("i: %d\n", i);
        print_grid(tree->last_level[i]->grid, game->gridx, game->gridy, tree->last_level[i]->pos);
    }

    //testing make_moves
    make_moves(game, 1, 1);
    print_grid(game->grid, game->gridx, game->gridy, game->pos);
    print_grid(game->p1->grid, game->p1->gridx, game->p1->gridy, game->p1->pos);

    //testing update tree
    update_tree(game->p1,1,0);
    printf("after 1 update\n");
    printf("P1 tree\n");
    tree = game->p1->tree;
    for (int i = 0; i<tree->num_nodes; i++) {
        if (!tree->last_level[i]) continue;
        printf("i: %d\n", i);
        print_grid(tree->last_level[i]->grid, game->gridx, game->gridy, tree->last_level[i]->pos);
    }
    update_tree(game->p1,1,1);
    update_tree(game->p2,1,1);
    printf("P2 tree\n");
    tree = game->p2->tree;
    for (int i = 0; i<tree->num_nodes; i++) {
        if (!tree->last_level[i]) continue;
        printf("i: %d\n", i);
        print_grid(tree->last_level[i]->grid, game->gridx, game->gridy, tree->last_level[i]->pos);
    }
    update_tree(game->p2,1,0);
    printf("after 2 updates\n");
    printf("P1 tree\n");
    tree = game->p1->tree;
    for (int i = 0; i<tree->num_nodes; i++) {
        if (!tree->last_level[i]) continue;
        printf("i: %d\n", i);
        print_grid(tree->last_level[i]->grid, game->gridx, game->gridy, tree->last_level[i]->pos);
    }
    printf("P2 tree\n");
    tree = game->p2->tree;
    for (int i = 0; i<tree->num_nodes; i++) {
        if (!tree->last_level[i]) continue;
        printf("i: %d\n", i);
        print_grid(tree->last_level[i]->grid, game->gridx, game->gridy, tree->last_level[i]->pos);
    }
    // Testing expand_tree after update_tree
    expand_tree(game->p1);
    expand_tree(game->p2);
    expand_tree(game->p1);
    expand_tree(game->p2);
    printf("after 2 more expand\n");
    printf("P1 tree\n");
    tree = game->p1->tree;
    for (int i = 0; i<tree->num_nodes; i++) {
        if (!tree->last_level[i]) continue;
        printf("i: %d\n", i);
        print_grid(tree->last_level[i]->grid, game->gridx, game->gridy, tree->last_level[i]->pos);
    }
    printf("P2 tree\n");
    tree = game->p2->tree;
    for (int i = 0; i<tree->num_nodes; i++) {
        if (!tree->last_level[i]) continue;
        printf("i: %d\n", i);
        print_grid(tree->last_level[i]->grid, game->gridx, game->gridy, tree->last_level[i]->pos);
    }
    make_moves(game, 0, 1);
    print_grid(game->grid, game->gridx, game->gridy, game->pos);
    //testing update tree
    update_tree(game->p1,0,0);
    printf("after 1 update\n");
    printf("P1 tree\n");
    tree = game->p1->tree;
    for (int i = 0; i<tree->num_nodes; i++) {
        if (!tree->last_level[i]) continue;
        printf("i: %d\n", i);
        print_grid(tree->last_level[i]->grid, game->gridx, game->gridy, tree->last_level[i]->pos);
    }
    update_tree(game->p1,1,1);
    update_tree(game->p2,1,1);
    printf("P2 tree\n");
    tree = game->p2->tree;
    for (int i = 0; i<tree->num_nodes; i++) {
        if (!tree->last_level[i]) continue;
        printf("i: %d\n", i);
        print_grid(tree->last_level[i]->grid, game->gridx, game->gridy, tree->last_level[i]->pos);
    }
    update_tree(game->p2,0,0);
    printf("after 2 updates\n");
    printf("P1 tree\n");
    tree = game->p1->tree;
    for (int i = 0; i<tree->num_nodes; i++) {
        if (!tree->last_level[i]) continue;
        printf("i: %d\n", i);
        print_grid(tree->last_level[i]->grid, game->gridx, game->gridy, tree->last_level[i]->pos);
    }
    printf("P2 tree\n");
    tree = game->p2->tree;
    for (int i = 0; i<tree->num_nodes; i++) {
        if (!tree->last_level[i]) continue;
        printf("i: %d\n", i);
        print_grid(tree->last_level[i]->grid, game->gridx, game->gridy, tree->last_level[i]->pos);
    }
    // Testing expand_tree after update_tree
    expand_tree(game->p1);
    expand_tree(game->p2);
    expand_tree(game->p1);
    expand_tree(game->p2);
    printf("after 2 more expand\n");
    printf("P1 tree\n");
    tree = game->p1->tree;
    for (int i = 0; i<tree->num_nodes; i++) {
        if (!tree->last_level[i]) continue;
        printf("i: %d\n", i);
        print_grid(tree->last_level[i]->grid, game->gridx, game->gridy, tree->last_level[i]->pos);
    }
    printf("P2 tree\n");
    tree = game->p2->tree;
    for (int i = 0; i<tree->num_nodes; i++) {
        if (!tree->last_level[i]) continue;
        printf("i: %d\n", i);
        print_grid(tree->last_level[i]->grid, game->gridx, game->gridy, tree->last_level[i]->pos);
    }
    make_moves(game, 0, 1);
    print_grid(game->grid, game->gridx, game->gridy, game->pos);
    //testing update tree
    update_tree(game->p1,0,0);
    printf("after 1 update\n");
    printf("P1 tree\n");
    tree = game->p1->tree;
    for (int i = 0; i<tree->num_nodes; i++) {
        if (!tree->last_level[i]) continue;
        printf("i: %d\n", i);
        print_grid(tree->last_level[i]->grid, game->gridx, game->gridy, tree->last_level[i]->pos);
    }
    update_tree(game->p1,1,1);
    update_tree(game->p2,1,1);
    printf("P2 tree\n");
    tree = game->p2->tree;
    for (int i = 0; i<tree->num_nodes; i++) {
        if (!tree->last_level[i]) continue;
        printf("i: %d\n", i);
        print_grid(tree->last_level[i]->grid, game->gridx, game->gridy, tree->last_level[i]->pos);
    }
    update_tree(game->p2,0,0);
    printf("after 2 updates\n");
    printf("P1 tree\n");
    tree = game->p1->tree;
    for (int i = 0; i<tree->num_nodes; i++) {
        if (!tree->last_level[i]) continue;
        printf("i: %d\n", i);
        print_grid(tree->last_level[i]->grid, game->gridx, game->gridy, tree->last_level[i]->pos);
    }
    printf("P2 tree\n");
    tree = game->p2->tree;
    for (int i = 0; i<tree->num_nodes; i++) {
        if (!tree->last_level[i]) continue;
        printf("i: %d\n", i);
        print_grid(tree->last_level[i]->grid, game->gridx, game->gridy, tree->last_level[i]->pos);
    }

    destroy_game(game);
    for (int i = 0; i<100; i++) {
        p1[0] = p1[1] = 7;
        p2[0] = p2[1] = 2;
        game = init_game(10, 10, NULL, p1, p2);
        add_players(game, 3, 1, randommove, randommove);
        //print_grid(game->grid, game->gridx, game->gridy, game->pos);
        play_game_minmax(game);
        destroy_game(game);
    }
    */
        Game* game;
    int num_games = 1;
    int random1, random2;
    int voronoi1, voronoi2;
    long timeminmax, timeab;
    struct timespec start, current;
    voronoi1 = voronoi2 = 0;
    /*
    clock_gettime(CLOCK_REALTIME, &start);
    for (int i = 0; i<num_games; i++) {
        p1[0] = p1[1] = 7;
        p2[0] = p2[1] = 2;
        game = init_game(10, 10, NULL, p1, p2);
        add_players(game, 3, 1, voronoi_dist, voronoi_dist);
        //print_grid(game->grid, game->gridx, game->gridy, game->pos);
        play_game_minmax(game);
        if (game->p1lost && !game->p2lost) voronoi1++;
        if (game->p2lost && !game->p1lost) voronoi2++;
        destroy_game(game);
    }
    clock_gettime(CLOCK_REALTIME, &current);
    timeminmax = elapsed_time(&start, &current);
    clock_gettime(CLOCK_REALTIME, &start);
    for (int i = 0; i<num_games; i++) {
        p1[0] = p1[1] = 7;
        p2[0] = p2[1] = 2;
        game = init_game(10, 10, NULL, p1, p2);
        add_players(game, 3, 1, voronoi_dist, voronoi_dist);
        //print_grid(game->grid, game->gridx, game->gridy, game->pos);
        play_game_ab(game);
        if (game->p1lost && !game->p2lost) voronoi1++;
        if (game->p2lost && !game->p1lost) voronoi2++;
        destroy_game(game);
    }
    clock_gettime(CLOCK_REALTIME, &current);
    timeab = elapsed_time(&start, &current);
    printf("Time minmax : %f\n", (float)timeminmax/1E9);
    printf("Time ab :     %f\n", (float)timeab/1E9);
    printf("Difference : %f\n", (float)(timeminmax - timeab)/1E9);
    */
    /*
    for (int i = 0; i<num_games; i++) {
        p1[0] = p1[1] = 7;
        p2[0] = p2[1] = 2;
        game = init_game(10, 10, NULL, p1, p2);
        add_players(game, 3, 1, voronoi_dist, voronoi_dist);
        //print_grid(game->grid, game->gridx, game->gridy, game->pos);
        play_game(game, compute_next, compute_next);
        if (game->p1lost && !game->p2lost) voronoi2++;
        if (game->p2lost && !game->p1lost) voronoi1++;
        destroy_game(game);
    }
    printf("Wins p1 : %d\n", voronoi1);
    printf("Wins p2 : %d\n", voronoi2);
    voronoi1 = voronoi2 = 0;
    for (int i = 0; i<num_games; i++) {
        p1[0] = 4;
        p1[1] = 4;
        p2[0] = p2[1] = 1;
        game = init_game(6, 6, NULL, p1, p2);
        add_players(game, 3, 1, voronoi_dist, combo_score);
        //print_grid(game->grid, game->gridx, game->gridy, game->pos);
        play_game(game, compute_next, compute_ab);
        if (game->p1lost && !game->p2lost) voronoi2++;
        if (game->p2lost && !game->p1lost) voronoi1++;
        destroy_game(game);
    }
    printf("Wins p1 : %d\n", voronoi1);
    printf("Wins p2 : %d\n", voronoi2);
    */
    voronoi1 = voronoi2 = 0;
    for (int i = 0; i<num_games; i++) {
        p1[0] = 7;
        p1[1] = 7;
        p2[0] = p2[1] = 2;
        game = init_game(10, 10, NULL, p1, p2);
        add_players(game, 3, 1, combo_score, combo_score);
        //print_grid(game->grid, game->gridx, game->gridy, game->pos);
        play_game(game, compute_MC, compute_ab);
        if (game->p1lost && !game->p2lost) voronoi2++;
        if (game->p2lost && !game->p1lost) voronoi1++;
        destroy_game(game);
    }
    printf("Wins p1 : %d\n", voronoi1);
    printf("Wins p2 : %d\n", voronoi2);

    printf("Ended correctly\n");
}

/* Methods to test

void update_tree(Player* player, int move, int id);

int randommove(Player* player);

long elapsed_time(const struct timespec* start, const struct timespec* current);

*/
