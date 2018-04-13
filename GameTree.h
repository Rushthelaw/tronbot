//Declaration of the game tree and related (nodes) structure
//Will enventually also contain traversal function declaration
#ifndef TREE_H_INCLUDED
#define TREE_H_INCLUDED
#include "Player.h"

/* Node structure
 * We want node to contain a game configuration
 * pos[0] is the position of p1 and pos[1] is the position of p2
 * We also want to reference children and parent
 * last1 is the last move of p1, last2 is the last move of p2
 * */
// pointer is a workaround. It points to the pointer referencing this in last_level
struct Node {
    struct Node* parent;
    Player* player;
    int** grid;
    int** pos;
    int gridx, gridy, terminal, n_child, last1, last2, depth;
    double value;
    struct Node** pointer;
    struct Node** children;
};

Node* new_node(Node* parent, Player* player, int** grid, int gridx, int gridy,
        int** pos, int last1, int last2);

int** copy_grid(int** grid, int gridx, int gridy);

double min(Node** nodes, int n_nodes);

double max(Node** nodes, int n_nodes);

int spawn_children(Node* parent, int id);

void free_node(Node* node);

/* root is the root node (duh)
 * last_level is the last level of the tree (we expand with breadth first)
 * num_nodes is the number of nodes in the last level
 * depth is the number of levels in the tree (note that it can be used to know
 * which player turn it is on the next level)
 * */
struct Tree {
    Node* root;
    Node** last_level;
    int num_nodes;
    int depth;
};

Tree* new_tree(Node* root);

void free_tree(Tree* tree);

#endif
