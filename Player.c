#include <stdio.h>
#include <stdlib.h>
#include "Game.h"
#include "Player.h"
#include "GameTree.h"

// This function assumes grid and pos are taken from a player that's just been
// initialized. Therefore, we assume it will all work.
Player* init_player(int** grid, int gridx, int gridy, int** pos, int id,
        int last1, int last2, double (*node_value)(Node*)) {
    Player* player = malloc(sizeof(Player));
    if(!player) {
        printf("Out of memory or something like that.\n");
        return NULL;
    }
    // Creating game tree
    Node* treeroot = new_node(NULL, copy_grid(grid, gridx, gridy),
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

/* This is useless
// Sets valid_moves to an array containing valid moves for player id
// Does not directly use player as argument because it will be needed elsewhere
// Have to check valid_moves after using this, it might be set to NULL
int get_validmoves(int** grid, int gridx, int gridy, int** pos,
        int id, int** valid_moves) {
    int moves[4] = {0};
    int num = 0;
    if (pos[id][0]-1 >= 0 && !grid[pos[id][0]-1][pos[id][1]]
            && !(pos[id][0]-1 == pos[(id+1)&1][0] && pos[id][1] == pos[(id+1)&1][1])) {
        num++;
        moves[0] = 1;
    }
    if (pos[id][1]+1 < gridy && !grid[pos[id][0]][pos[id][1]+1]
            && !(pos[id][0] == pos[(id+1)&1][0] && pos[id][1]+1 == pos[(id+1)&1][1])) {
        num++;
        moves[1]=1;
    }
    if (pos[id][0]+1 < gridx && !grid[pos[id][0]+1][pos[id][1]]
            && !(pos[id][0]+1 == pos[(id+1)&1][0] && pos[id][1] == pos[(id+1)&1][1])) {
        num++;
        moves[2]=1;
    }
    if (pos[id][1]-1 >= 0 && !grid[pos[id][0]][pos[id][1]-1]
            && !(pos[id][0] == pos[(id+1)&1][0] && pos[id][1]-1 == pos[(id+1)&1][1])) {
        num++;
        moves[3]=1;
    }
    if(!num) {
        *valid_moves = NULL;
        printf("There are no valid moves\n");
        return 0;
    }
    //int availmoves[num];
    *valid_moves = malloc(num*sizeof(int));
    if(!*valid_moves) {
        printf("Out of memory.\n");
        return 0;
    }
    int count = 0;
    for (int i = 0; i<4; i++) {
        if (moves[i]) {
            //availmoves[count] = i;
            (*valid_moves)[count] = i;
            count++;
        }
    }
    /valid_moves = availmoves;
    return num;
}
*/

// Develops the next level of the game tree of player.
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
        return;
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
    for (int i = 0; i< total_num; i++) {
        player->tree->last_level[i]->pointer = player->tree->last_level+i;
    }
}

// Cuts useless branches of tree and updates root.
// Updates the tree one level at a time (this has to be used after we select our
// move, and once more when we receive the move of opponent)
// Does not update last_level of tree, but expand tree takes care of the possible
// NULL pointers
void update_tree(Player* player, int move, int id) {
    Node* root = player->tree->root;
    if (!root->n_child) {
        //printf("Tree root has no child!\n");
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
        if (root->children[i]->pos[id][0] == pos[0] 
                && root->children[i]->pos[id][1] == pos[1]) {
            player->tree->root = root->children[i];
            root->children[i] = NULL;
            free_node(root);
            player->tree->depth--;
            return;
        }
    }
    //printf("This move does not exists.\n");
}

// Explore tree and apply player->node_value on terminal nodes
// This is minimax minimizes if minmax=0, and maximizes otherwise
void evaluate_node(Node* node, Player* player, int minmax) {
    if (node->n_child) {
        for (int i = 0; i< node->n_child; i++) {
            if (node->children[i]->value == -100.0)
                evaluate_node(node->children[i], player, (minmax+1)&1);
        }
        if (minmax) node->value = max(node->children, node->n_child);
        else node->value = min(node->children, node->n_child);
        return;
    }
    // Because node_value is a loss function for p1
    if(node->value == -100.0) {
        if (player->id) node->value = - player->node_value(node);
        else node->value = player->node_value(node);
    }
}

// This is not super efficient, but i don't feel this needs to be
// Selects a move in the children of tree root that have an optimal value
// If many moves are equal, this will be random between them.
// This has no alpha-beta
int choose_next(Player* player) {
    int next_move[3];
    Node* root = player->tree->root;
    int num_moves = 0;
    for (int i = 0; i<root->n_child; i++) {
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
        //printf("No child, could not choose next.\n");
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
    while(player->tree->depth<7) {
        expand_tree(player);
    }
    evaluate_node(player->tree->root, player, 0);
    clock_gettime(CLOCK_REALTIME, &current);
    while(elapsed_time(&start, &current)<TTPLAY && player->tree->depth<3) {
        expand_tree(player);
        evaluate_node(player->tree->root, player, 0);
        clock_gettime(CLOCK_REALTIME, &current);
    }
    //printf("Time taken : %f\n", elapsed_time(&start, &current)/1000000000.0);
    return choose_next(player);
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

// Returns the controller of the space (i,j) in grid (closest player)
// 1.0 is p1, -1.0 is p2, 0.0 means both are or the space is inacessible.
// Uses dijkstra.
// This would probably be easier with a dynamic queue.
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
    int dist, list, p1, p2, l1l, l2l;
    dist = p1 = p2 = l2l = 0;
    list = 1;
    // These will act as open.
    int** list1 = malloc(gridx * gridy * sizeof(int*));
    if (!list1) {
        printf("Out of memory.\n");
        return 0.0;
    }
    int** list2 = malloc(gridx * gridy * sizeof(int*));
    if (!list2) {
        free(list1);
        printf("Out of memory.\n");
        return 0.0;
    }

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
    l1l = 1;
    while (!p1 && !p2) {
        //printf("hello\n");
        if (list) {
            //printf("1 %d %d\n", l1l, l2l);
            for (int k = 0; k<l1l; k++) {
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
                                    for (int m = k; m<l1l; m++) free(list1[m]);
                                    free(list1);
                                    free(list2);
                                    return 0.0;
                                }
                                position[0] = list1[k][0]-1;
                                position[1] = list1[k][1];
                                list2[l2l] = position;
                                l2l++;
                            }
                        }
                        else if (l == 1) {
                            if (list1[k][1] + 1 < gridy) {
                                position = malloc(2*sizeof(int));
                                if (!position) {
                                    printf("Out of memory.\n");
                                    for (int m = k; m<l1l; m++) free(list1[m]);
                                    free(list1);
                                    free(list2);
                                    return 0.0;
                                }
                                position[0] = list1[k][0];
                                position[1] = list1[k][1]+1;
                                list2[l2l] = position;
                                l2l++;
                            }
                        }
                        else if (l == 2) {
                            if (list1[k][0] + 1 < gridx) {
                                position = malloc(2*sizeof(int));
                                if (!position) {
                                    printf("Out of memory.\n");
                                    for (int m = k; m<l1l; m++) free(list1[m]);
                                    free(list1);
                                    free(list2);
                                    return 0.0;
                                }
                                position[0] = list1[k][0]+1;
                                position[1] = list1[k][1];
                                list2[l2l] = position;
                                l2l++;
                            }
                        }
                        else if (l == 3) {
                            if (list1[k][1] - 1 >= 0) {
                                position = malloc(2*sizeof(int));
                                if (!position) {
                                    printf("Out of memory.\n");
                                    for (int m = k; m<l1l; m++) free(list1[m]);
                                    free(list1);
                                    free(list2);
                                    return 0.0;
                                }
                                position[0] = list1[k][0];
                                position[1] = list1[k][1]-1;
                                list2[l2l] = position;
                                l2l++;
                            }
                        }
                    }
                }
                free(list1[k]);
            }
            //printf("concluded\n");
            l1l = 0;
            dist++;
            list = (list+1)&1;
        } else {
            //printf("2 %d %d\n", l1l, l2l);
            for (int k = 0; k<l2l; k++) {
                if (grid[list2[k][0]][list2[k][1]]) {
                    free(list2[k]);
                    continue;
                }

                if (list2[k][0] == pos[0][0] && list2[k][1] == pos[0][1])
                    p1 += 1;
                if (list2[k][0] == pos[1][0] && list2[k][1] == pos[1][1])
                    p2 += 1;
                if (p1 || p2) {
                    free(list2[k]);
                    continue;
                }

                if (distance[list2[k][0]][list2[k][1]] > dist) {
                    distance[list2[k][0]][list2[k][1]] = dist;
                    for (int l = 0; l<4; l++) {
                        if (l == 0) {
                            if (list2[k][0] - 1 >= 0) {
                                position = malloc(2*sizeof(int));
                                if (!position) {
                                    printf("Out of memory.\n");
                                    for (int m = k; m<l2l; m++) free(list2[m]);
                                    free(list1);
                                    free(list2);
                                    return 0.0;
                                }
                                position[0] = list2[k][0]-1;
                                position[1] = list2[k][1];
                                list1[l1l] = position;
                                l1l++;
                            }
                        }
                        else if (l == 1) {
                            if (list2[k][1] + 1 < gridy) {
                                position = malloc(2*sizeof(int));
                                if (!position) {
                                    printf("Out of memory.\n");
                                    for (int m = k; m<l2l; m++) free(list2[m]);
                                    free(list1);
                                    free(list2);
                                    return 0.0;
                                }
                                position[0] = list2[k][0];
                                position[1] = list2[k][1]+1;
                                list1[l1l] = position;
                                l1l++;
                            }
                        }
                        else if (l == 2) {
                            if (list2[k][0] + 1 < gridx) {
                                position = malloc(2*sizeof(int));
                                if (!position) {
                                    printf("Out of memory.\n");
                                    for (int m = k; m<l2l; m++) free(list2[m]);
                                    free(list1);
                                    free(list2);
                                    return 0.0;
                                }
                                position[0] = list2[k][0]+1;
                                position[1] = list2[k][1];
                                list1[l1l] = position;
                                l1l++;
                            }
                        }
                        else if (l == 3) {
                            if (list2[k][1] - 1 >= 0) {
                                position = malloc(2*sizeof(int));
                                if (!position) {
                                    printf("Out of memory.\n");
                                    for (int m = k; m<l2l; m++) free(list2[m]);
                                    free(list1);
                                    free(list2);
                                    return 0.0;
                                }
                                position[0] = list2[k][0];
                                position[1] = list2[k][1]-1;
                                list1[l1l] = position;
                                l1l++;
                            }
                        }
                    }
                }
                free(list2[k]);
            }
            l2l = 0;
            dist++;
            list = (list+1)&1;
            //printf("concluded\n");
        }
        if (l1l == l2l) {
            //printf("ended\n");
            free(list1);
            free(list2);
            return 0.0;
        }
    }
    if (l1l) {
        for (int k = 0; k<l1l; k++) {
            free(list1[k]);
        }
    }
    if (l2l) {
        for (int k = 0; k<l2l; k++) {
            free(list2[k]);
        }
    }
    free(list1);
    free(list2);
    //printf("ended\n");
    if (p1 && p2) return 0.0;
    else if (p1) return 1.0;
    else if (p2) return -1.0;
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
