#include <stdio.h>
#include <stdlib.h>
#include "Player.h"
#include "Game.h"
#include "GameTree.h"

// I won't perform non-critical tests here because they will use precious time
Node* new_node(Node* parent, int** grid, int gridx, int gridy, int** pos) {
    Node* node = malloc(sizeof(Node));
    if(!node) {
        printf("Out of memory or something like that.\n");
        return NULL;
    }

    node->parent = parent;
    node->grid = grid;
    node->pos = pos;
    node->gridx = gridx;
    node->gridy = gridy;
    node->terminal = 0;
    node->n_child = 0;
    node->pointer = NULL;
    node->children = NULL;

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

// This creates all the subsequent game positions for move made by player id
int spawn_children(Node* parent, int id) {
    if (parent->terminal) return 0;
    printf("helloqsadfa\n");
    if (!parent) return 0;

    int* valid_moves;
    int n_validmoves;
    n_validmoves = get_validmoves(parent->grid, parent->gridx, parent->gridy,
            parent->pos, id, &valid_moves);
    if(!valid_moves) {
        parent->terminal = 1;
        parent->children = NULL;
        return 0;
    }
    parent->n_child = n_validmoves;

    int** new_grid;
    int** new_pos;
    parent->children = malloc(n_validmoves*sizeof(Node*));
    if(!parent->children) {
        printf("Out of memory!\n");
        free(valid_moves);
        return -1;
    }
    for (int i = 0; i<n_validmoves; i++) {
        new_grid = copy_grid(parent->grid, parent->gridx, parent->gridy);
        new_pos = copy_grid(parent->pos, 2, 2);
        make_move(new_grid, parent->gridx, parent->gridy, new_pos, id, valid_moves[i]);
        parent->children[i] = new_node(parent, new_grid, parent->gridx, parent->gridy, new_pos);
    }
    free(valid_moves);
    return n_validmoves;
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
    for (int i = 0; i<node->gridx; i++) {
        free(node->grid[i]);
    }
    free(node->grid);
    for (int i = 0; i<2; i++) {
        free(node->pos[i]);
    }
    free(node->pos);
    if (node->pointer) *node->pointer = NULL;
    free(node);
}

Tree* new_tree(Node* root) {
    Tree* tree = malloc(sizeof(tree));
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
    free_node(tree->root);
    free(tree->last_level);
    free(tree);
}
