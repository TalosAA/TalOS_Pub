#ifndef _TREE_H_
#define _TREE_H_

#include <libk/kheap.h>

#define TREE_T_OK       (0)
#define TREE_T_NOK      (-1)

typedef struct tree_node {
  void* node_ptr;
  struct tree_node* next;     /* same level nodes */
  struct tree_node* children; /* children nodes */
} tree_node_t;

typedef int (*tree_compare_fun_t)(tree_node_t* node, void* param);

tree_node_t* tree_initialize(void);

tree_node_t* tree_allocNode(void);

tree_node_t* tree_getLastChild(tree_node_t* parent);

tree_node_t* tree_findChild(tree_node_t* parent,
                            tree_compare_fun_t compFun,
                            void* compParam);

tree_node_t* tree_findNodeRecursive(tree_node_t* parent,
                                    tree_compare_fun_t compFun,
                                    void* compParam);

int tree_addChild(tree_node_t* parent, void* node_ptr);

void tree_addSubTree(tree_node_t* parent, tree_node_t* tree);

int tree_pruneSubTree(tree_node_t* parent, tree_node_t* node);

#endif