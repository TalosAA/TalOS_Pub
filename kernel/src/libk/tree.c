#include <libk/tree.h>

static int tree_pruneSubTree_inter(tree_node_t* node);

tree_node_t* tree_initialize(void) {
  return tree_allocNode();
}

tree_node_t* tree_allocNode(void) {
  tree_node_t *newNode;
  newNode = kmalloc(sizeof(tree_node_t));
  if(newNode == NULL) {
    return NULL;
  }
  newNode->node_ptr = NULL;
  newNode->next = NULL;
  newNode->children = NULL;
  return newNode;
}

tree_node_t* tree_getLastChild(tree_node_t* parent) {
  tree_node_t* currChild = (tree_node_t*)parent->children;
  while (currChild->next != NULL) {
    currChild = currChild->next;
  }
  return currChild;
}

static int tree_pruneSubTree_inter(tree_node_t* node) {
  int ret = TREE_T_NOK;
  if(node->children != NULL) {
    if(tree_pruneSubTree_inter(node->children) != TREE_T_OK) {
      return TREE_T_NOK;
    }
  }
  if(node->next != NULL) {
    if(tree_pruneSubTree_inter(node->next) != TREE_T_OK) {
      return TREE_T_NOK;
    }
  }

  kfree(node);
  return ret;
}

int tree_pruneSubTree(tree_node_t* parent, tree_node_t* node) {

  /* find node within the children of parent and remove it */
  if(parent->children == node) {
    parent->children = node->next;
  } else {
    tree_node_t* prev = parent->children;
    tree_node_t* curr = prev->next;
    while(curr != NULL) {
      if(curr == node) {
        prev->next = node->next;
        break;
      }
      prev = curr;
      curr = curr->next;
    }
    if(curr == NULL) {
      return TREE_T_NOK;
    }
  }
  
  return tree_pruneSubTree_inter(node);
}

tree_node_t* tree_findChild(tree_node_t* parent,
                            tree_compare_fun_t compFun,
                            void* compParam) {
  tree_node_t* currNode = (tree_node_t*)parent;
  tree_node_t* retNode = NULL;

  if (currNode->children != NULL) {
    currNode = currNode->children;
    do {
      if (compFun(currNode, compParam) == TREE_T_OK) {
        retNode = currNode;
        break;
      }
      currNode = currNode->next;
    } while (currNode != NULL);
  }

  return retNode;
}

tree_node_t* tree_findNodeRecursive(tree_node_t* node,
                                    tree_compare_fun_t compFun,
                                    void* compParam){                                
  tree_node_t* foundNode = NULL;
  if (compFun(node, compParam) == TREE_T_OK) {
    foundNode = node;
  } else {
    if(node->children != NULL) {
      foundNode = tree_findNodeRecursive(node->children, compFun, compParam);
    }
    if(node->next != NULL && foundNode == NULL) {
      foundNode = tree_findNodeRecursive(node->children, compFun, compParam);
    }
  }
  return foundNode;
}

int tree_addChild(tree_node_t* parent, void* node_ptr){
  tree_node_t* lastChild = tree_getLastChild(parent);
  lastChild->next = tree_allocNode();
  if(lastChild->next == NULL) {
    return TREE_T_NOK;
  }
  lastChild->next->node_ptr = node_ptr;
  return TREE_T_OK;
}

void tree_addSubTree(tree_node_t* parent, tree_node_t* tree){
  tree_node_t* lastChild = tree_getLastChild(parent);
  lastChild->next = tree;
}