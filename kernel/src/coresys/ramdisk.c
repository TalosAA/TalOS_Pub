#include <coresys/initrd.h>
#include <coresys/ramdisk.h>
#include <libk/kheap.h>
#include <libk/string.h>

typedef struct ramdisk_file_header {
  fs_node_t fs_node;
  void* filePtr;
  struct ramdisk_file_header* next;     /* same level nodes */
  struct ramdisk_file_header* children; /* children nodes */
} ramdisk_file_header_t;

static ramdisk_file_header_t root;

static void* initrd_base = NULL;

static ramdisk_file_header_t* LoadInitrdRegFile(initrd_file_header* initrdFile);
static ramdisk_file_header_t* LoadInitrdDirFile(initrd_file_header* initrdFile);
static int LoadInitrdFiles(ramdisk_file_header_t* parent,
                           ramdisk_file_header_t* currentDir,
                           initrd_file_header* initrdcurrentDir);

static inline ramdisk_file_header_t* get_last_child(
    ramdisk_file_header_t* parent) {
  ramdisk_file_header_t* currChild = (ramdisk_file_header_t*)parent->children;
  while (currChild->next != NULL) {
    currChild = currChild->next;
  }
  return currChild;
}

fs_node_t* getRoot(void) { return &root.fs_node; }

int LoadInitrd(const uint8_t* initrdPtr) {
  initrd_header* initrdHeader = (initrd_header*)initrdPtr;
  initrd_file_header* initrd_root =
      (initrd_file_header*)(((uintptr_t)initrdPtr) + sizeof(initrd_header));

  if (UINT32_INITRD_ORDER(initrdHeader->magic) != INITRD_MAGIC) {
    return RET_T_NOK;
  }

  /* Used to optimize stack */
  initrd_base = (void*)initrdPtr;

  /* Prepare root descriptor */
  root.fs_node.open = NULL;
  root.fs_node.close = NULL;
  root.fs_node.read = NULL;
  root.fs_node.write = NULL;
  root.fs_node.find = ramdisk_find;
  root.fs_node.readdir = ramdisk_readdir;
  root.fs_node.length = 0;
  root.fs_node.ino = fs_new_ino_id();
  root.fs_node.name[0] = '\0';
  root.fs_node.owner_group = root.fs_node.owner_id = 0;
  root.fs_node.perm = 0;
  root.fs_node.type = FTYPE_DIRECTORY;
  root.fs_node.node_ptr = NULL;
  root.fs_node.impl_def = 0;

  return LoadInitrdFiles(&root, &root, initrd_root);
}

ssize_t ramdisk_read(struct fs_node* node, void* buf, size_t nbyte,
                     off_t offset) {
  ramdisk_file_header_t* ramdNode = (ramdisk_file_header_t*)node;
  size_t toReadLen = nbyte;

  if (node == NULL || buf == NULL || node->read == NULL ||
      node->type != FTYPE_REGULAR || offset > node->length) {
    // TODO: errno
    return -1;
  }

  if ((nbyte + offset) > node->length) {
    toReadLen = node->length - offset;
  }

  memcpy(buf, ramdNode->filePtr, toReadLen);
  return toReadLen;
}

ssize_t ramdisk_write(struct fs_node* node, const void* buf, size_t nbyte,
                      off_t offset) {
  ramdisk_file_header_t* ramdNode = (ramdisk_file_header_t*)node;
  size_t toWriteLen = nbyte;

  if (node == NULL || buf == NULL || node->read == NULL ||
      node->type != FTYPE_REGULAR || offset > node->length) {
    // TODO: errno
    return -1;
  }

  if ((nbyte + offset) > node->length) {
    toWriteLen = node->length - offset;
  }

  memcpy(ramdNode->filePtr, buf, toWriteLen);
  return toWriteLen;
}

int ramdisk_open(struct fs_node* node, const char* restrict __is_unused mode) {
  // TODO
  return RET_T_OK;
}

int ramdisk_close(struct fs_node* __is_unused node) {
  // TODO
  return RET_T_OK;
}

int ramdisk_readdir(struct fs_node* dir_node, uint32_t index,
                    struct dirent* entry, struct dirent** result) {
  ramdisk_file_header_t* ramdNode = (ramdisk_file_header_t*)dir_node;
  int ret = RET_T_NOK;
  uint32_t i = 0;

  *result = NULL;

  if (dir_node == NULL || dir_node->readdir == NULL ||
      dir_node->type != FTYPE_DIRECTORY) {
    // TODO errno
    return RET_T_NOK;
  }
  if (ramdNode->children == NULL) {
    *result = NULL;
    ret = RET_T_OK;
  } else {
    ramdNode = ramdNode->children;
    i = 0;
    while (i != index && ramdNode != NULL) {
      i++;
      ramdNode = ramdNode->next;
    }
    if (i != index || ramdNode == NULL) {
      *result = NULL;
    } else {
      entry->d_ino = ramdNode->fs_node.ino;
      entry->d_type = ramdNode->fs_node.type;
      strncpy(entry->d_name, ramdNode->fs_node.name, MAX_FNAME);
      entry->d_reclen = sizeof(dirent_t) - MAX_FNAME + strlen(entry->d_name);
      *result = entry;
    }
    ret = RET_T_OK;
  }
  return ret;
}

struct fs_node* ramdisk_find(struct fs_node* dir_node, char* name) {
  ramdisk_file_header_t* ramdNode = (ramdisk_file_header_t*)dir_node;
  fs_node_t* retNode = NULL;

  if (dir_node == NULL || dir_node->readdir == NULL ||
      dir_node->type != FTYPE_DIRECTORY || name == NULL) {
    // TODO errno
    return NULL;
  }

  if (ramdNode->children != NULL) {
    ramdNode = ramdNode->children;
    do {
      if (strncmp(ramdNode->fs_node.name, name, MAX_FNAME) == 0) {
        retNode = &ramdNode->fs_node;
        break;
      }
      ramdNode = ramdNode->next;
    } while (ramdNode != NULL);
  }

  return retNode;
}

int ramdisk_add_child(struct fs_node* dirNode, struct fs_node* copyChild) {
  int ret = RET_T_NOK;
  ramdisk_file_header_t* parent = (ramdisk_file_header_t*)dirNode;

  if (dirNode != NULL && dirNode->type == FTYPE_DIRECTORY) {
    ramdisk_file_header_t* ramdisk_child =
        (ramdisk_file_header_t*)kmalloc(sizeof(ramdisk_file_header_t));
    if(ramdisk_child != NULL) {
        ramdisk_child->next = NULL;
        ramdisk_child->children = NULL;
        memcpy(&ramdisk_child->fs_node, copyChild, sizeof(fs_node_t));
      if (parent->children == NULL) {
        parent->children = ramdisk_child;
      } else {
        (get_last_child(parent))->next = ramdisk_child;
      }
      ret = RET_T_OK;
    }
  }
  return ret;
}

static ramdisk_file_header_t* LoadInitrdRegFile(
    initrd_file_header* initrdFile) {
  ramdisk_file_header_t* node = kmalloc(sizeof(ramdisk_file_header_t));

  if(node != NULL) {
    node->fs_node.open = ramdisk_open;
    node->fs_node.close = ramdisk_close;
    node->fs_node.read = ramdisk_read;
    node->fs_node.write = ramdisk_write;
    node->fs_node.find = NULL;
    node->fs_node.readdir = NULL;
    node->fs_node.length = UINT32_INITRD_ORDER(initrdFile->length);
    node->fs_node.ino = fs_new_ino_id();
    strncpy(node->fs_node.name, initrdFile->name, MAX_FNAME);
    node->fs_node.owner_group = node->fs_node.owner_id = 0;
    node->fs_node.perm = 0;
    node->fs_node.type = FTYPE_REGULAR;
    node->fs_node.node_ptr = NULL;
    node->fs_node.impl_def = 0;
    node->children = NULL;
    node->next = NULL;

    /* Alloc space for the file and copy it */
    node->filePtr = kmalloc(node->fs_node.length);
    if(node->filePtr != NULL) {
      memcpy(node->filePtr,
            (void*)((uintptr_t)initrd_base +
                    UINT32_INITRD_ORDER(initrdFile->start_file_off)),
            node->fs_node.length);
    } else {
      kfree(node);
      node = NULL;
    }
  }

  return node;
}

static ramdisk_file_header_t* LoadInitrdDirFile(
    initrd_file_header* initrdFile) {
  ramdisk_file_header_t* node = kmalloc(sizeof(ramdisk_file_header_t));

  if(node != NULL) {
    node->fs_node.open = NULL;
    node->fs_node.close = NULL;
    node->fs_node.read = NULL;
    node->fs_node.write = NULL;
    node->fs_node.find = ramdisk_find;
    node->fs_node.readdir = ramdisk_readdir;
    node->fs_node.length = 0;
    node->fs_node.ino = fs_new_ino_id();
    strncpy(node->fs_node.name, initrdFile->name, MAX_FNAME);
    node->fs_node.owner_group = node->fs_node.owner_id = 0;
    node->fs_node.perm = 0;
    node->fs_node.type = FTYPE_DIRECTORY;
    node->fs_node.node_ptr = NULL;
    node->fs_node.impl_def = 0;
    node->children = NULL;
    node->next = NULL;
    node->filePtr = NULL;
  }

  return node;
}

static int LoadInitrdFiles(ramdisk_file_header_t* parent,
                            ramdisk_file_header_t* currentDir,
                            initrd_file_header* initrdcurrentDir) {
  initrd_file_header* initrdchildren =
      (initrd_file_header*)(((uintptr_t)initrdcurrentDir) +
                            sizeof(initrd_file_header));
  ramdisk_file_header_t* prev = NULL;
  ramdisk_file_header_t* curr = NULL;
  uint32_t i = 0;

  /* Prepare simlink to the parent node */
  ramdisk_file_header_t* currentDirSimlink =
      kmalloc(sizeof(ramdisk_file_header_t));
  if(currentDirSimlink == NULL) {
    return RET_T_NOK;
  }
  ramdisk_file_header_t* parentSimlink = kmalloc(sizeof(ramdisk_file_header_t));
  if(currentDirSimlink == NULL) {
    return RET_T_NOK;
  }
  fs_get_simlink(".", &currentDir->fs_node, &currentDirSimlink->fs_node);
  fs_get_simlink("..", &parent->fs_node, &parentSimlink->fs_node);
  if (currentDir->children == NULL) {
    currentDir->children = currentDirSimlink;
    currentDirSimlink->next = parentSimlink;
    parentSimlink->next = NULL;
  }

  for (i = 0; i < UINT32_INITRD_ORDER(initrdcurrentDir->dir_nfiles); i++) {
    if (initrdchildren[i].type == UINT32_INITRD_ORDER(INITRD_FTYPE_REGULAR)) {
      curr = LoadInitrdRegFile(&initrdchildren[i]);
    } else if (initrdchildren[i].type ==
               UINT32_INITRD_ORDER(INITRD_FTYPE_DIR)) {
      curr = LoadInitrdDirFile(&initrdchildren[i]);
      if(LoadInitrdFiles(currentDir, curr, &initrdchildren[i]) != RET_T_OK) {
        return RET_T_NOK;
      }
    } else {
      /* jump to the next cycle */
      continue;
    }
    if (parentSimlink->next == NULL) {
      parentSimlink->next = curr;
    } else {
      prev->next = curr;
    }
    prev = curr;
  }
  return RET_T_OK;
}