/**
 * Filesystem
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "drivers/bfs.h"

/**
 * __new_node
*/

INODE *__new_node(dword minimumDegree, bool isLeaf) {
    return NULL;
}

/**
 * __new_btree
*/

BTREE *__new_btree(dword minimumDegree) {
    BTREE *btree = (BTREE *)malloc(sizeof(BTREE));
    btree->minimumDegree = minimumDegree;

    INODE *root = __new_node(minimumDegree, TRUE);

    if (!root) TODO;

    btree->root = root;

    return btree;
}

/**
 * Insert Node
*/

void __insert_node(void) {

}

/**
 * Search Node
*/

void __search_node(void) {

}