#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Player.h"
#include "Game.h"
#include "GameTree.h"

// I won't perform non-critical tests here because they will use precious time
Node* new_node(Node* parent, Player* player, int** grid, int gridx, int gridy,
        int** pos, int last1, int last2) {
    Node* node = malloc(sizeof(Node));
    if(!node) {
        printf("Out of memory or something like that.\n");
        return NULL;
    }

    node->parent = parent;
    node->player = player;
    node->grid = grid;
    node->pos = pos;
    node->gridx = gridx;
    node->gridy = gridy;
    node->terminal = game_over(grid, gridx, gridy, pos);
    node->n_child = 0;
    node->pointer = NULL;
    node->children = NULL;
    node->last1 = last1;
    node->last2 = last2;
    node->value = -INFINITY;

    return node;
}

// Copy a pointer of pointer of int
int** copy_grid(int** grid, int gridx, int gridy) {
    int** copy = malloc(gridx*sizeof(int*));
    if(!copy) {
        printf("Out of memory or something like that.\n");
        return NULL;
    }
    for (int i = 0; i<gridx; i++) {
        copy[i] = malloc(gridy*sizeof(int));
        if(!copy[i]) {
            for (int j = 0; j<i; j++) {
                free(copy[j]);
            }
            free(copy);
            printf("Out of memory.\n");
            return NULL;
        }
        for (int j = 0; j<gridy; j++) {
            copy[i][j] = grid[i][j];
        }
    }
    return copy;
}

// This assumes nodes have been evaluated
// Right now, those just return min and max value, but i might need to add some
// stuff to allow me to find the next node without having to look for it
double min(Node** nodes, int n_nodes) {
    double min = INFINITY;
    for (int i = 0; i<n_nodes; i++) {
        if (!nodes[i]) continue;
        if (nodes[i]->value < min) min = nodes[i]->value;
    }
    return min;
}

// This assumes nodes have been evaluated
double max(Node** nodes, int n_nodes) {
    double max = -INFINITY;
    for (int i = 0; i<n_nodes; i++) {
        if (!nodes[i]) continue;
        if (nodes[i]->value > max) max = nodes[i]->value;
    }
    return max;
}

// This creates all the subsequent game positions for moves made by player id
// Note that this does generate impossible positions
int spawn_children(Node* parent, int id) {
    if (!parent) return 0;
    // we won't spawn children of a game that is over
    if (parent->terminal) return 0;

    parent->n_child = 3;
    int** new_grid;
    int** new_pos;
    parent->children = malloc(3 * sizeof(Node*));
    if(!parent->children) {
        printf("Out of memory!\n");
        return -1;
    }
    int count = 0;
    if(id) {
        for (int i = 0; i<4; i++) {
            //This is to avoid generating a backwards move. This is especialy usefull
            //to avoid making an illegal first move.
            if ((parent->last2+2)%4 == i) continue;
            new_grid = copy_grid(parent->grid, parent->gridx, parent->gridy);
            new_pos = copy_grid(parent->pos, 2, 2);
            make_move(new_grid, parent->gridx, parent->gridy, new_pos, id, i);
            parent->children[count] = new_node(parent, parent->player, new_grid,
                    parent->gridx, parent->gridy, new_pos, parent->last1, i);
            count++;
        }
    } else {
        for (int i = 0; i<4; i++) {
            if ((parent->last1+2)%4 == i) continue;
            new_grid = copy_grid(parent->grid, parent->gridx, parent->gridy);
            new_pos = copy_grid(parent->pos, 2, 2);
            make_move(new_grid, parent->gridx, parent->gridy, new_pos, id, i);
            parent->children[count] = new_node(parent, parent->player, new_grid,
                    parent->gridx, parent->gridy, new_pos, i, parent->last2);
            count++;
        }
    }
    return 3;
}

//Frees memory allocated to a node. Will recursively free memory allocated to
// childs too, beware.
// This function returns null because we will somethimes need to set a pointer
// to NULL after freeing it.
void free_node(Node* node) {
    if (!node) return;

    if (node->n_child) {
        for (int i = 0; i<node->n_child; i++) {
            free_node(node->children[i]);
        }
        free(node->children);
    }

    if (node->grid) {
        for (int i = 0; i<node->gridx; i++) {
            free(node->grid[i]);
        }
        free(node->grid);
    }

    if (node->pos) {
        for (int i = 0; i<2; i++) {
            free(node->pos[i]);
        }
        free(node->pos);
    }

    if (node->pointer) *node->pointer = NULL;
    free(node);
}

Tree* new_tree(Node* root) {
    Tree* tree = malloc(sizeof(Tree));
    if(!tree) {
        printf("Out of memory or something like that.\n");
        return NULL;
    }
    tree->depth = 1;
    tree->root = root;
    tree->last_level = malloc(sizeof(Node*));
    tree->last_level[0] = root;
    tree->num_nodes = 1;

    return tree;
}

void free_tree(Tree* tree) {
    if (!tree) return;
    free_node(tree->root);
    free(tree->last_level);
    free(tree);
}
