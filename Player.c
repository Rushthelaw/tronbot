#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "Game.h"
#include "Player.h"
#include "GameTree.h"
#include "lfsr113.h"

// This function assumes grid and pos are taken from a player that's just been
// initialized. Therefore, we assume it will all work (no validity checks are made)
Player* init_player(int** grid, int gridx, int gridy, int** pos, int id,
        int last1, int last2, double (*node_value)(Node*)) {
    Player* player = malloc(sizeof(Player));
    if(!player) {
        printf("Out of memory or something like that.\n");
        return NULL;
    }
    // Creating game tree
    Node* treeroot = new_node(NULL, player, copy_grid(grid, gridx, gridy),
            gridx, gridy, copy_grid(pos, 2, 2), last1, last2);
    player->tree = new_tree(treeroot);

    player->pos = pos;
    player->id = id;
    player->grid = grid;
    player->gridx = gridx;
    player->gridy = gridy;
    player->last1 = last1;
    player->last2 = last2;
    player->node_value = node_value;
    player->lost = 0;

    return player;
}

// Frees allocated memory when creating a Player
void destroy_player(Player* player) {
    free_tree(player->tree);
    free(player);
}

// Develops the next level of the game tree of player.
int expand_tree(Player* player) {
    // We evaluate which player plays on next level of tree
    player->tree->depth++;
    int to_move = (player->tree->depth + player->id) % 2;
    int total_num = 0;
    int num_elem;

    Node*** last_level = malloc(player->tree->num_nodes * sizeof(Node**));
    if (!last_level) {
        printf("Out of memory!\n");
        player->tree->depth--;
        return 0;
    }

    for (int i = 0; i<player->tree->num_nodes; i++) {
        if (!player->tree->last_level[i]) {
            last_level[i] = NULL;
            continue;
        }
        num_elem = spawn_children(player->tree->last_level[i], to_move);
        if (!num_elem) {
            last_level[i] = NULL;
            continue;
        }

        last_level[i] = player->tree->last_level[i]->children;
        total_num += num_elem;
    }
    Node** last = malloc(total_num * sizeof(Node*));
    if (!last) {
        printf("Out of memory!\n");
        free(last_level);
        player->tree->depth--;
        return 0;
    }

    int total = 0;

    for (int i = 0; i<player->tree->num_nodes; i++) {
        if(!last_level[i]) continue;
        for (int j = 0; j<player->tree->last_level[i]->n_child; j++) {
            last[total] = last_level[i][j];
            total++;
        }
    }
    free(last_level);

    /* Les quatres prochaines lignes n'ont rien d'impressionnantes, mais il m'a
     * fallu 4 heures pour trouver qu'elles étaient nécessaire à ce que 
     * l'exécution se déroule comme voulue.
     * */
    for (int i = 0; i<player->tree->num_nodes; i++) {
        if (player->tree->last_level[i]) 
        player->tree->last_level[i]->pointer = NULL;
    }

    player->tree->num_nodes = total_num;
    free(player->tree->last_level);
    player->tree->last_level = last;
    for (int i = 0; i<total_num; i++) {
        player->tree->last_level[i]->pointer = player->tree->last_level+i;
    }
    return total_num;
}

// Expands tree via Monte-Carlo. This will create children for num nodes
// This really is to have interresting results...
int expand_tree_MC(Player* player, int num) {
    if (player->tree->num_nodes == 0) return 0;
    int total_num = 0;
    int num_elem, next_node, to_move;

    Node*** last_level = malloc(num * sizeof(Node**));
    if (!last_level) {
        printf("Out of memory!\n");
        return -1;
    }

    // Choosing nodes and spawning children
    for (int i = 0; i<num; i++) {
        next_node = (int) lfsr113()*player->tree->num_nodes;
        if (next_node == player->tree->num_nodes) printf("TRUE\n");

        // We might not expand the right number of nodes. If we don't, we at
        // least update player->tree->last_level at the end
        if (!player->tree->last_level[next_node]) {
            last_level[i] = NULL;
            continue;
        }

        to_move = (player->tree->last_level[next_node]->depth + player->id+1)%2;
        num_elem = spawn_children(player->tree->last_level[next_node], to_move);
        
        player->tree->last_level[next_node]->pointer = NULL;
        if (num_elem <= 0) {
            last_level[i] = NULL;
            player->tree->last_level[next_node] = NULL;
            continue;
        }

        last_level[i] = player->tree->last_level[next_node]->children;
        total_num += num_elem;

        player->tree->last_level[next_node] = NULL;
    }
    num_elem = 0;
    for (int i = 0; i<player->tree->num_nodes; i++) {
        if (player->tree->last_level[i]) {
            player->tree->last_level[num_elem] = player->tree->last_level[i];
            num_elem++;
        }
    }
    Node** tmp = malloc((num_elem+total_num) * sizeof(Node*));
    if (!tmp) {
        printf("Out of memory!\n");
        free(last_level);
        return -1;
    }
    memcpy(tmp, player->tree->last_level, num_elem * sizeof(Node*));

    next_node = 0;
    for (int i = 0; i<num; i++) {
        if(!last_level[i]) continue;
        for (int j = 0; j<3; j++) {
            tmp[num_elem+next_node] = last_level[i][j];
            next_node++;
        }
    }
    free(last_level);
    free(player->tree->last_level);
    player->tree->last_level = tmp;

    player->tree->num_nodes = total_num+num_elem;
    for (int i = 0; i<player->tree->num_nodes; i++) {
        player->tree->last_level[i]->pointer = player->tree->last_level+i;
    }
    return 1;
}

// Cuts useless branches of tree and updates root.
// Updates the tree one level at a time (this has to be used after we select our
// move, and once more when we receive the move of opponent)
// Does not update last_level of tree, but expand tree takes care of the possible
// NULL pointers
void update_tree(Player* player, int move, int id) {
    Node* root = player->tree->root;
    if (!root->n_child) {
        printf("Tree root has no child!\n");
        return;
    }
    int pos[2];
    if (move==0) {
        pos[0] = root->pos[id][0] - 1;
        pos[1] = root->pos[id][1];
    } else if (move==1) {
        pos[0] = root->pos[id][0];
        pos[1] = root->pos[id][1] + 1;
    } else if (move==2) {
        pos[0] = root->pos[id][0] + 1;
        pos[1] = root->pos[id][1];
    } else if (move==3) {
        pos[0] = root->pos[id][0];
        pos[1] = root->pos[id][1] - 1;
    }
    for (int i = 0; i<3; i++) {
        if (!root->children[i]) continue;
        if (root->children[i]->pos[id][0] == pos[0] 
                && root->children[i]->pos[id][1] == pos[1]) {
            player->tree->root = root->children[i];
            root->children[i] = NULL;
            free_node(root);
            player->tree->depth--;
            return;
        }
    }
    printf("This move does not exists.\n");
}

// Explore tree and apply player->node_value on terminal nodes
// This is minimax minimizes if minmax=0, and maximizes otherwise
// This has no alpha-beta
void evaluate_node(Node* node, Player* player, int minmax) {
    if (node->n_child) {
        for (int i = 0; i< node->n_child; i++) {
            evaluate_node(node->children[i], player, (minmax+1)&1);
        }
        if (minmax) node->value = max(node->children, node->n_child);
        else node->value = min(node->children, node->n_child);
        return;
    }
    // Because node_value is a loss function for p1
    if(isinf(node->value)) {
        if (player->id) node->value = - player->node_value(node);
        else node->value = player->node_value(node);
    }
}

// Explore tree and apply player->node_value on terminal nodes
// This is minimax minimizes if minmax=0, and maximizes otherwise
// This has alpha-beta pruning. This will only cut nodes for moves made by player
// This is much much faster
/* Because this is a simultaneous game, and I want to keep the tree memorized at
 * each step (this is not really dynamic programming otherwise...), I can only
 * prune nodes that are the result of a move made by player. If we prune nodes
 * resulting of moves made by the opponent, the tree might not be updated
 * properly because pruning would assume that the opponent uses the same 
 * strategy as us.
 * */
void evaluate_ab(Node* node, Player* player, int minmax, int depth) {
    if (node->n_child) {
        for (int i = 0; i<node->n_child; i++) {
            if (node->children[i])
                evaluate_ab(node->children[i], player, (minmax+1)&1, depth+1);
        }
        if (minmax) node->value = max(node->children, node->n_child);
        else node->value = min(node->children, node->n_child);
        // We prune after evaluation, this way we don't have to keep a beta
        // (best possible score in current position for player) because it is
        // node->value.
        if (depth%6 == 1) {
            for (int i = 0; i<node->n_child; i++) {
                if (!node->children[i]) continue;
                if (node->children[i]->value > node->value) {
                    free_node(node->children[i]);
                    node->children[i] = NULL;
                }
            }
        }
        return;
    }
    // Because node_value is a loss function for p1
    if(isinf(node->value)) {
        if (player->id) node->value = - player->node_value(node);
        else node->value = player->node_value(node);
    }
}

// This is not super efficient, but i don't feel this needs to be
// Selects a move in the children of tree root that have an optimal value
// If many moves are equal, this will be random between them.
int choose_next(Player* player) {
    int next_move[3];
    Node* root = player->tree->root;
    int num_moves = 0;
    for (int i = 0; i<root->n_child; i++) {
        if (!root->children[i]) continue;
        if (root->children[i]->value <= root->value) {
            if (player->id) {
                next_move[num_moves] = root->children[i]->last2;
                num_moves++;
            } else {
                 next_move[num_moves] = root->children[i]->last1;
                 num_moves++;
            }
        }
    }
    if (num_moves) return next_move[rand() % num_moves];
    else {
        printf("No child, could not choose next.\n");
        return -1;
    }
}

// This is the time control for the execution of the calculations
int compute_next(Player* player, int last_self, int last_op) {
    struct timespec start, current;
    clock_gettime(CLOCK_REALTIME, &start);
    if(player->tree->depth>2) {
        update_tree(player, last_self, player->id);
        update_tree(player, last_op, (player->id+1)&1);
        if(player->id) {
            player->last1 = last_op;
            player->last2 = last_self;
        } else {
            player->last1 = last_self;
            player->last2 = last_op;
        }
    }
    while(player->tree->depth<3) {
        expand_tree(player);
    }
    evaluate_node(player->tree->root, player, 0);
    clock_gettime(CLOCK_REALTIME, &current);
    // The second condition is because of memory limitations on my PC
    while(elapsed_time(&start, &current)<500000000 && player->tree->num_nodes<3000000) {
        expand_tree(player);
        evaluate_node(player->tree->root, player, 0);
        clock_gettime(CLOCK_REALTIME, &current);
    }
    //printf("Time taken : %f\n", elapsed_time(&start, &current)/1000000000.0);
    return choose_next(player);
}

// This is the time control for the execution of the calculations
int compute_ab(Player* player, int last_self, int last_op) {
    struct timespec start, current;
    clock_gettime(CLOCK_REALTIME, &start);
    if(player->tree->depth>2) {
        update_tree(player, last_self, player->id);
        update_tree(player, last_op, (player->id+1)&1);
        if(player->id) {
            player->last1 = last_op;
            player->last2 = last_self;
        } else {
            player->last1 = last_self;
            player->last2 = last_op;
        }
    }
    while(player->tree->depth<3) {
        expand_tree(player);
    }
    evaluate_ab(player->tree->root, player, 0, 1);
    clock_gettime(CLOCK_REALTIME, &current);
    // The second condition is because of memory limitations on my PC
    while(elapsed_time(&start, &current)<500000000 && player->tree->num_nodes<3000000) {
        expand_tree(player);
        evaluate_ab(player->tree->root, player, 0, 1);
        clock_gettime(CLOCK_REALTIME, &current);
    }
    //printf("Time taken : %f\n", elapsed_time(&start, &current)/1000000000.0);
    return choose_next(player);
}

// This is the time control for MCT
int compute_MC(Player* player, int last_self, int last_op) {
    struct timespec start, current;
    clock_gettime(CLOCK_REALTIME, &start);
    if (last_self >= 0) {
        update_tree(player, last_self, player->id);
        update_tree(player, last_op, (player->id+1)&1);
        if(player->id) {
            player->last1 = last_op;
            player->last2 = last_self;
        } else {
            player->last1 = last_self;
            player->last2 = last_op;
        }
    }
    
    Node** nodes = malloc(3*sizeof(Node*));
    nodes[0] = player->tree->root;
    nodes[1] = NULL;
    nodes[2] = NULL;
    int num[3] = {0};
    // Expanding tree to the proper depth
    int total = 0;
    for (int i = 0; i<2; i++) {
        for (int j = 0; j<3; j++) {
            if (nodes[j]) num[j] = spawn_children(nodes[j], (i+player->id)%2);
            if (num[j] > 0 && i > 0) {
                total += num[j];
            }
        }
        if (i==0 && nodes[0]->children) {
            for (int j = 0; j<3; j++) {
                nodes[2-j] = nodes[0]->children[j];
            }
        }
    }
    Node** tmp = malloc((player->tree->num_nodes + total) * sizeof(Node*));
    if (!tmp) {
        free(nodes);
        printf("Out of memory\n");
        return -1;
    }
    total = 0;
    memcpy(tmp, player->tree->last_level, player->tree->num_nodes*sizeof(Node*));
    for (int i = 0; i<3; i++) {
        if (num[i] > 0) {
            for (int j = 0; j<num[i]; j++) {
                tmp[player->tree->num_nodes + total] = nodes[i]->children[j];
                total++;
            }
        }
    }
    player->tree->num_nodes += total;

    free(nodes);
    free(player->tree->last_level);
    player->tree->last_level = tmp;
    for (int k = 0; k<player->tree->num_nodes; k++) {
        if(player->tree->last_level[k])
        player->tree->last_level[k]->pointer = player->tree->last_level + k;
    }

    evaluate_ab(player->tree->root, player, 0, 1);
    clock_gettime(CLOCK_REALTIME, &current);
    int has_nodes = 1;
    while(elapsed_time(&start, &current)<TTPLAY && has_nodes) {
        has_nodes = expand_tree_MC(player, 5);
        evaluate_ab(player->tree->root, player, 0, 1);
        clock_gettime(CLOCK_REALTIME, &current);
    }
    //printf("Time taken : %f\n", elapsed_time(&start, &current)/1000000000.0);
    return choose_next(player);
}
// This is an exact solver of the game
// This will timeout if it takes more than 30 minutes to expand the tree
int compute_exact(Player* player, int last_self, int last_op) {
    struct timespec start, current;
    clock_gettime(CLOCK_REALTIME, &start);
    clock_gettime(CLOCK_REALTIME, &current);
    int next = 1;
    while ((elapsed_time(&start, &current)/1E9 < 1800) && next) {
        clock_gettime(CLOCK_REALTIME, &start);
        next = expand_tree(player);
        clock_gettime(CLOCK_REALTIME, &current);
    }
    if (elapsed_time(&start, &current)/1E9 >= 1800) {
        printf("Tree is way big, algorithm has timed out\n");
        return -1;
    }
    if (last_op >= 0) update_tree(player, last_op, (player->id+1)&1);
    if (isinf(player->tree->root->value)) 
        evaluate_ab(player->tree->root, player, 0, 1);
    next = choose_next(player);
    update_tree(player, next, player->id);
    return next;
}

// Always return 0. Every move that is not endgame is equal.
double randommove(Node* node) {
    if (node->terminal) {
        int loser = game_loser(node->grid, node->gridx, node->gridy, node->pos);
        if (loser == 2) return 0.0;
        if (loser == 1) return -1.0;
        if (loser == 0) return 1.0;
    }
    return 0.0;
}

// Returns difference between number of "controlled squares for p1 and p2
double voronoi_dist(Node* node) {
    if (node->terminal) {
        int loser = game_loser(node->grid, node->gridx, node->gridy, node->pos);
        if (loser == 2) return 0.0;
        if (loser == 1) return -(double)(node->gridx*node->gridy);
        if (loser == 0) return (double)(node->gridx*node->gridy);
    }
    double score = 0.0;
    for (int i = 0; i<node->gridx; i++) {
        for (int j = 0; j<node->gridy; j++) {
            score -= controller(i, j, node->grid, node->gridx, node->gridy, node->pos);
        }
   }
    return score;
}


// This is a heuristic function that evaluates voronoi partitions as well as
// giving a bonus to the score if the player chooses a move next to a wall, and
// if the opponent is close
/* The reason why we don't give a bonus if the distance is less than 2 is that
 * it is possible that a crash occurs between the players in this situation,
 * resulting in a tie, but ONLY VICTORY COUNTS, FOR GLORY!
 * */
double combo_score(Node* node) {
    double max_score = (double)(node->gridx*node->gridy);
    if (node->terminal) {
        int loser = game_loser(node->grid, node->gridx, node->gridy, node->pos);
        if (loser == 2) return 0.0;
        if (loser == 1) return -max_score;
        if (loser == 0) return max_score;
    }
    double score = 0.0;
    for (int i = 0; i<node->gridx; i++) {
        for (int j = 0; j<node->gridy; j++) {
            score -= controller(i, j, node->grid, node->gridx, node->gridy, node->pos);
        }
    }
    double to_get = max_score+score;
    double distance = p_distance(node->grid, node->gridx, node->gridy, node->pos);
    if (distance > 2.0) score -= 0.5 * to_get / distance ;
    score -= 0.5 * to_get * w_distance(node->grid, node->gridx, node->gridy, node->player);
    return score;
}

/* The next 2 functions basically do the same thing with different objectives
 * in mind. They all use Dijkstra (sort of) to compute the distance between some
 * stuff. This is super redundant so feel free to only read the description.
 * */
// Returns the controller of the space (i,j) in grid (closest player)
// 1.0 is p1, -1.0 is p2, 0.0 means both are or the space is inacessible.
double controller(int i, int j, int** grid, int gridx, int gridy, int** pos) {
    if (grid[i][j]) return 0.0;
    if ((pos[0][0] == i && pos[0][1] == j)
            || (pos[1][0] == i && pos[1][1] == j)) return 0.0;
    int distance[gridx][gridy];
    for (int k = 0; k<gridx; k++) {
        for (int l = 0; l<gridy; l++) {
            distance[k][l] = gridx + gridy + 10;
        }
    }
    int dist, p1, p2, list_length, last_length;
    dist = p1 = p2 = list_length = 0;
    // These will act as open.
    int** list1 = malloc(gridx * gridy * sizeof(int*));
    if (!list1) {
        printf("Out of memory.\n");
        return 0.0;
    }
    int** list2;
    int* position = malloc(2 * sizeof(int));
    if (!position) {
        printf("Out of memory.\n");
        free(list1);
        free(list2);
        return 0.0;
    }
    position[0] = i;
    position[1] = j;
    list1[0] = position;
    last_length = 1;
    while (!p1 && !p2) {
        list2 = malloc(4 * last_length * sizeof(int*));
        if (!list2) {
            for (int i = 0; i<last_length; i++) free(list1[i]);
            free(list1);
            printf("Out of memory\n");
            return 0.0;
        }
        for (int k = 0; k<last_length; k++) {
            if (grid[list1[k][0]][list1[k][1]]) {
                free(list1[k]);
                continue;
            }
            if (list1[k][0] == pos[0][0] && list1[k][1] == pos[0][1])
                p1 += 1;
            else if (list1[k][0] == pos[1][0] && list1[k][1] == pos[1][1])
                p2 += 1;
            if (p1 || p2) {
                free(list1[k]);
                continue;
            }

            if (distance[list1[k][0]][list1[k][1]] > dist) {
                distance[list1[k][0]][list1[k][1]] = dist;
                for (int l = 0; l<4; l++) {
                    if (l == 0) {
                        if (list1[k][0] - 1 >= 0) {
                            position = malloc(2*sizeof(int));
                            if (!position) {
                                printf("Out of memory.\n");
                                for (int m = k; m<last_length; m++)
                                    free(list1[m]);
                                for (int m = 0; m<list_length; m++)
                                    free(list2[m]);
                                free(list1);
                                free(list2);
                                return 0.0;
                            }
                            position[0] = list1[k][0]-1;
                            position[1] = list1[k][1];
                            list2[list_length] = position;
                            list_length++;
                        }
                    } else if (l == 1) {
                        if (list1[k][1] + 1 < gridy) {
                            position = malloc(2*sizeof(int));
                            if (!position) {
                                printf("Out of memory.\n");
                                for (int m = k; m<last_length; m++)
                                    free(list1[m]);
                                for (int m = 0; m<list_length; m++)
                                    free(list2[m]);
                                free(list1);
                                free(list2);
                                return 0.0;
                            }
                            position[0] = list1[k][0];
                            position[1] = list1[k][1]+1;
                            list2[list_length] = position;
                            list_length++;
                        }
                    } else if (l == 2) {
                        if (list1[k][0] + 1 < gridx) {
                            position = malloc(2*sizeof(int));
                            if (!position) {
                                printf("Out of memory.\n");
                                for (int m = k; m<last_length; m++)
                                    free(list1[m]);
                                for (int m = 0; m<list_length; m++)
                                    free(list2[m]);
                                free(list1);
                                free(list2);
                                return 0.0;
                            }
                            position[0] = list1[k][0]+1;
                            position[1] = list1[k][1];
                            list2[list_length] = position;
                            list_length++;
                        }
                    } else if (l == 3) {
                        if (list1[k][1] - 1 >= 0) {
                            position = malloc(2*sizeof(int));
                            if (!position) {
                                printf("Out of memory.\n");
                                for (int m = k; m<last_length; m++)
                                    free(list1[m]);
                                for (int m = 0; m<list_length; m++)
                                    free(list2[m]);
                                free(list1);
                                free(list2);
                                return 0.0;
                            }
                            position[0] = list1[k][0];
                            position[1] = list1[k][1]-1;
                            list2[list_length] = position;
                            list_length++;
                        }
                    }
                }
            }
            free(list1[k]);
        }
        free(list1);
        last_length = list_length;
        list_length = 0;
        list1 = list2;
        dist++;
        if (last_length == list_length) {
            free(list1);
            return 0.0;
        }
    }
    if (last_length) {
        for (int k = 0; k<last_length; k++) {
            free(list1[k]);
        }
    }
    free(list1);
    if (p1 && p2) return 0.0;
    else if (p1) return 1.0;
    else if (p2) return -1.0;
}

// Enemy distance
// This evaluates the distance between players via shortest path (norm 1 might
// not be accurate because it is not obvious there is a path between p1 and p2)
// Returns -1.0 if grid is not connected between both players, so this function
// may be used to test connectivity
double p_distance(int** grid, int gridx, int gridy, int** pos) {
    int distance[gridx][gridy];
    for (int k = 0; k<gridx; k++) {
        for (int l = 0; l<gridy; l++) {
            distance[k][l] = gridx + gridy + 10;
        }
    }
    int dist, list_length, last_length, connect;
    dist = list_length = connect = 0;
    // These will act as open.
    int** list1 = malloc(gridx * gridy * sizeof(int*));
    if (!list1) {
        printf("Out of memory.\n");
        return 0.0;
    }
    int** list2;
    int* position = malloc(2 * sizeof(int));
    if (!position) {
        printf("Out of memory.\n");
        free(list1);
        free(list2);
        return 0.0;
    }
    position[0] = pos[0][0];
    position[1] = pos[0][1];
    list1[0] = position;
    last_length = 1;
    while (!connect) {
        list2 = malloc(gridx * gridy *sizeof(int*));
        for (int k = 0; k<last_length; k++) {
            if (grid[list1[k][0]][list1[k][1]]) {
                free(list1[k]);
                continue;
            }
            if (list1[k][0] == pos[1][0] && list1[k][1] == pos[1][1])
                connect = 1;
            if (connect) {
                free(list1[k]);
                continue;
            }

            if (distance[list1[k][0]][list1[k][1]] > dist) {
                distance[list1[k][0]][list1[k][1]] = dist;
                for (int l = 0; l<4; l++) {
                    if (l == 0) {
                        if (list1[k][0] - 1 >= 0) {
                            position = malloc(2*sizeof(int));
                            if (!position) {
                                printf("Out of memory.\n");
                                for (int m = k; m<last_length; m++)
                                    free(list1[m]);
                                for (int m = 0; m<list_length; m++)
                                    free(list2[m]);
                                free(list1);
                                free(list2);
                                return 0.0;
                            }
                            position[0] = list1[k][0]-1;
                            position[1] = list1[k][1];
                            list2[list_length] = position;
                            list_length++;
                        }
                    }
                    else if (l == 1) {
                        if (list1[k][1] + 1 < gridy) {
                            position = malloc(2*sizeof(int));
                            if (!position) {
                                printf("Out of memory.\n");
                                for (int m = k; m<last_length; m++)
                                    free(list1[m]);
                                for (int m = 0; m<list_length; m++)
                                    free(list2[m]);
                                free(list1);
                                free(list2);
                                return 0.0;
                            }
                            position[0] = list1[k][0];
                            position[1] = list1[k][1]+1;
                            list2[list_length] = position;
                            list_length++;
                        }
                    }
                    else if (l == 2) {
                        if (list1[k][0] + 1 < gridx) {
                            position = malloc(2*sizeof(int));
                            if (!position) {
                                printf("Out of memory.\n");
                                for (int m = k; m<last_length; m++)
                                    free(list1[m]);
                                for (int m = 0; m<list_length; m++)
                                    free(list2[m]);
                                free(list1);
                                free(list2);
                                return 0.0;
                            }
                            position[0] = list1[k][0]+1;
                            position[1] = list1[k][1];
                            list2[list_length] = position;
                            list_length++;
                        }
                    }
                    else if (l == 3) {
                        if (list1[k][1] - 1 >= 0) {
                            position = malloc(2*sizeof(int));
                            if (!position) {
                                printf("Out of memory.\n");
                                for (int m = k; m<last_length; m++)
                                    free(list1[m]);
                                for (int m = 0; m<list_length; m++)
                                    free(list2[m]);
                                free(list1);
                                free(list2);
                                return 0.0;
                            }
                            position[0] = list1[k][0];
                            position[1] = list1[k][1]-1;
                            list2[list_length] = position;
                            list_length++;
                        }
                    }
                }
            }
            free(list1[k]);
        }
        free(list1);
        last_length = list_length;
        list_length = 0;
        list1 = list2;
        if (last_length == list_length) break;
        dist++;
    }
    if (last_length) {
        for (int k = 0; k<last_length; k++) {
            free(list1[k]);
        }
    }
    free(list1);
    if (connect) return (double)dist;
    return -1.0;
}

// Wall proximity reward
// This has a little twist to not look directly behind player
// Returns 1.0 if player has a wall or the grid limits at distance <= range
double w_distance(int** grid, int gridx, int gridy, Player* player) {
    int pos[2];
    pos[0] = player->pos[player->id][0];
    pos[1] = player->pos[player->id][1];
    double has_wall = 0.0;
    // These will act as open.

    for (int l = 0; l<4; l++) {
        if (player->id) {
            if (player->last2 == (l+2)%4) continue;
        } else {
            if (player->last1 == (l+2)%4) continue;
        }
        if (l == 0) {
            if (pos[0] - 1 >= 0) {
                if (grid[pos[0]-1][pos[1]]) {
                    has_wall = 1.0;
                }
            } else has_wall = 1.0;
        }
        else if (l == 1) {
            if (pos[1] + 1 < gridy) {
                if (grid[pos[0]][pos[1]+1]) {
                    has_wall = 1.0;
                }
            } else has_wall = 1.0;
        }
        else if (l == 2) {
            if (pos[0] + 1 < gridx) {
                if (grid[pos[0]+1][pos[1]]) {
                    has_wall = 1.0;
                }
            } else has_wall = 1.0;
        }
        else if (l == 3) {
            if (pos[1] - 1 >= 0) {
                if (grid[pos[0]][pos[1]-1]) {
                    has_wall = 1.0;
                }
            } else has_wall = 1.0;
        }
    }
    return has_wall;
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
