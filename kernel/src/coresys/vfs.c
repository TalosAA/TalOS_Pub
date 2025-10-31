#include <coresys/vfs.h>
#include <libk/string.h>

static ino_t last_ino = 0;
static fs_node_t* root = NULL;

void fs_set_root(struct fs_node* root_node) { root = root_node; }

struct fs_node* fs_get_root(void) { return root; }

struct fs_node* fs_get_node(struct fs_node* current, const char* path) {
  char intPath[MAX_PATH];
  fs_node_t* currNode = NULL;
  char* tokenContext = NULL;
  char* currTokenNode = NULL;

  strncpy(intPath, path, MAX_PATH);

  if (intPath[0] == '/') {
    /* relative to root */
    currNode = root;
  } else {
    /* relative to current node */
    currNode = current;
  }

  if(currNode != NULL) {
    currTokenNode = strtok_r(intPath, "/", &tokenContext);
    if (currTokenNode != NULL) {
      do {
        currNode = fs_find(currNode, currTokenNode);
        if (currNode == NULL) break;
      } while ((currTokenNode = strtok_r(NULL, "/", &tokenContext)) != NULL);
    }
  }

  return currNode;
}

void fs_get_simlink(const char* simlinkName, struct fs_node* inNode,
                    struct fs_node* simlink) {
  simlink->open = NULL;
  simlink->close = NULL;
  simlink->read = NULL;
  simlink->write = NULL;
  simlink->find = NULL;
  simlink->readdir = NULL;
  simlink->length = 0;
  simlink->ino = fs_new_ino_id();
  strncpy(simlink->name, simlinkName, MAX_FNAME);
  simlink->owner_group = simlink->owner_id = 0;
  simlink->perm = 0;
  simlink->type = FTYPE_SYMLINK;
  simlink->node_ptr = inNode;
  simlink->impl_def = 0;
}

static inline struct fs_node* get_node_from_simlink(struct fs_node* node) {
  while(node->type == FTYPE_SYMLINK) {
    node = node->node_ptr;
  }
  return node;
}


ssize_t fs_read(struct fs_node* node, void* buf, size_t nbyte, off_t offset) {
  node = get_node_from_simlink(node);
  if (node->read != NULL) {
    return node->read(node, buf, nbyte, offset);
  } else {
    return RET_T_NOK;
  }
}

ssize_t fs_write(struct fs_node* node, const void* buf, size_t nbyte,
                 off_t offset) {
  node = get_node_from_simlink(node);
  if (node->write != NULL) {
    return node->write(node, buf, nbyte, offset);
  }
  return RET_T_NOK;
}

int fs_open(struct fs_node* node, const char* restrict mode) {
  node = get_node_from_simlink(node);
  if (node->open != NULL) {
    return node->open(node, mode);
  }
  return RET_T_NOK;
}

int fs_close(struct fs_node* node) {
  node = get_node_from_simlink(node);
  if (node->close != NULL) {
    return node->close(node);
  }
  return RET_T_NOK;
}

int fs_readdir(struct fs_node* dir_node, uint32_t index, struct dirent* entry,
               struct dirent** result) {
  dir_node = get_node_from_simlink(dir_node);
  if (dir_node->readdir != NULL) {
    return dir_node->readdir(dir_node, index, entry, result);
  }
  return RET_T_NOK;
}

struct fs_node* fs_find(struct fs_node* dir_node, char* name) {
  dir_node = get_node_from_simlink(dir_node);
  if (dir_node->find != NULL) {
    return dir_node->find(dir_node, name);
  }
  return NULL;
}

ino_t fs_new_ino_id(void) {
  //TODO manage critical section
  last_ino++;
  return (last_ino - 1);
}