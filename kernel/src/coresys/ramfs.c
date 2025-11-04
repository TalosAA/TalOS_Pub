#include <coresys/initrd.h>
#include <coresys/ramfs.h>
#include <libk/kheap.h>
#include <libk/string.h>
#include <libk/kcleenup.h>

static ramfs_file_header_t root;

static void* initrd_base = NULL;

static ramfs_file_header_t* LoadInitrdRegFile(initrd_file_header* initrdFile);
static ramfs_file_header_t* LoadInitrdDirFile(initrd_file_header* initrdFile);
static int LoadInitrdFiles(ramfs_file_header_t* parent,
                           ramfs_file_header_t* currentDir,
                           initrd_file_header* initrdcurrentDir);

static inline ramfs_file_header_t* ramfs_allocNode(void) {
  ramfs_file_header_t* newNode;
  newNode = kmalloc(sizeof(ramfs_file_header_t));
  if(newNode == NULL){
    return NULL;
  }
  memset(newNode, 0, sizeof(ramfs_file_header_t));
  return newNode;
}

static inline void ramfs_deallocNode(ramfs_file_header_t* node) {
  kfree(node->filePtr);
  kfree(node);
}

struct fs_node* ramfs_getLastChild(struct fs_node* parent) {
  ramfs_file_header_t* currChild = ((ramfs_file_header_t*)parent)->children;
  while (currChild->next != NULL) {
    currChild = currChild->next;
  }
  return (struct fs_node*)currChild;
}

static inline ramfs_file_header_t* ramfs_getLastChild_inter(ramfs_file_header_t* parent) {
  return (ramfs_file_header_t*)ramfs_getLastChild((struct fs_node*)parent);
}

int ramfs_init(void) {
  CLEENUP_FUNCTION_SET(kfree, free_mem);
  ramfs_file_header_t* currentDirSimlink = NULL;
  ramfs_file_header_t* parentSimlink = NULL;

  /* Prepare root descriptor */
  root.fs_node.open = NULL;
  root.fs_node.close = NULL;
  root.fs_node.read = NULL;
  root.fs_node.write = NULL;
  root.fs_node.find = ramfs_find;
  root.fs_node.readdir = ramfs_readdir;
  root.fs_node.length = 0;
  root.fs_node.ino = fs_new_ino_id();
  root.fs_node.name[0] = '\0';
  root.fs_node.owner_group = root.fs_node.owner_id = 0;
  root.fs_node.perm = 0;
  root.fs_node.type = FTYPE_DIRECTORY;
  root.fs_node.node_ptr = NULL;
  root.fs_node.impl_def = 0;

  /* Prepare simlink to the current node */
  currentDirSimlink = ramfs_allocNode();
  if(currentDirSimlink == NULL) {
    CLEENUP(default);
  }
  fs_get_simlink(".", &root.fs_node, &currentDirSimlink->fs_node);
  /* Prepare simlink to the parent node */
  parentSimlink = ramfs_allocNode();
  if(parentSimlink == NULL) {
    CLEENUP(free_curr_dir_simlink);
  }
  fs_get_simlink("..", &root.fs_node, &parentSimlink->fs_node);
  if (root.children == NULL) {
    root.children = currentDirSimlink;
    currentDirSimlink->next = parentSimlink;
    parentSimlink->next = NULL;
  }

  fs_set_root(&root.fs_node);

  return VFS_T_OK;

  CLEENUP_LABEL(free_curr_dir_simlink, free_mem, currentDirSimlink);
  CLEENUP_DEFAULT_LABEL();

  return VFS_T_NOK;
}

static int ramfs_inter_deleteNode(ramfs_file_header_t* node) {

  if(node->children != NULL) {
    if(ramfs_inter_deleteNode(node->children) != VFS_T_OK) {
      return VFS_T_NOK;
    }
  }
  if(node->next != NULL) {
    if(ramfs_inter_deleteNode(node->next) != VFS_T_OK) {
      return VFS_T_NOK;
    }
  }

  ramfs_deallocNode(node);
  return VFS_T_OK;
}

int ramfs_deleteNode(const char* nodeAbsPath) {
  ramfs_file_header_t* node = NULL;
  ramfs_file_header_t* parentDir = NULL;
  char parentPath[MAX_FNAME];

  if(fs_getParentPath(nodeAbsPath, parentPath) != RET_T_OK){
    return VFS_T_NOK;
  }

  parentDir = (ramfs_file_header_t*)fs_get_node(NULL, parentPath);
  if(parentDir == NULL) {
    return VFS_T_NOK;
  }

  node = (ramfs_file_header_t*)fs_get_node(NULL, nodeAbsPath);
  if(node == NULL) {
    return VFS_T_NOK;
  }

  /* find node within the children of parent and remove it */
  if(parentDir->children == node) {
    parentDir->children = node->next;
  } else {
    ramfs_file_header_t* prev = parentDir->children;
    ramfs_file_header_t* curr = prev->next;
    while(curr != NULL) {
      if(curr == node) {
        prev->next = node->next;
        break;
      }
      prev = curr;
      curr = curr->next;
    }
    if(curr == NULL) {
      return VFS_T_NOK;
    }
  }

  return ramfs_inter_deleteNode(node);
}

fs_node_t* ramfs_mkDir(const char* dirName, const char* parentDirAbsPath, int *err) {
  CLEENUP_FUNCTION_SET(kfree, free_mem);
  ramfs_file_header_t* newDir;
  ramfs_file_header_t* currentDirSimlink;
  ramfs_file_header_t* parentDirSimlink;
  ramfs_file_header_t* parentDir;

  if(err != NULL) {
    *err = VFS_T_NOK;
  }

  if(fs_checkFileName(dirName, NULL) != RET_T_OK) {
    CLEENUP(default);
  }

  parentDir = (ramfs_file_header_t*)fs_get_node(NULL, parentDirAbsPath);
  if(parentDir == NULL){
    CLEENUP(default);
  }

  newDir = (ramfs_file_header_t*)fs_find((fs_node_t*)parentDir, dirName);
  if(newDir != NULL){
    if(err != NULL) {
      *err = RFS_FILE_ALREADY_EXISTS;
    }
    CLEENUP(default);
  }

  newDir = ramfs_allocNode();
  if(newDir == NULL) {
    CLEENUP(default);
  }

  memset(newDir, 0, sizeof(ramfs_file_header_t));

  /* Prepare root descriptor */
  newDir->fs_node.open = NULL;
  newDir->fs_node.close = NULL;
  newDir->fs_node.read = NULL;
  newDir->fs_node.write = NULL;
  newDir->fs_node.find = ramfs_find;
  newDir->fs_node.readdir = ramfs_readdir;
  newDir->fs_node.length = 0;
  newDir->fs_node.ino = fs_new_ino_id();
  strncpy(newDir->fs_node.name, dirName, MAX_FNAME);
  newDir->fs_node.owner_group = newDir->fs_node.owner_id = 0;
  newDir->fs_node.perm = 0;
  newDir->fs_node.type = FTYPE_DIRECTORY;
  newDir->fs_node.node_ptr = NULL;
  newDir->fs_node.impl_def = 0;

  /* Prepare simlink to the current node */
  currentDirSimlink = ramfs_allocNode();
  if(currentDirSimlink == NULL) {
    CLEENUP(cleen_newDirSimlink);
  }

  fs_get_simlink(".", &newDir->fs_node, &currentDirSimlink->fs_node);

  /* Prepare simlink to the parent node */
  parentDirSimlink = ramfs_allocNode();
  if(parentDirSimlink == NULL) {
    CLEENUP(cleen_currentDirSimlink);
  }
  fs_get_simlink("..", (fs_node_t*)parentDir, &parentDirSimlink->fs_node);

  if (newDir->children == NULL) {
    newDir->children = currentDirSimlink;
    currentDirSimlink->next = parentDirSimlink;
    parentDirSimlink->next = NULL;
  }

  (ramfs_getLastChild_inter(parentDir))->next = newDir;

  if(err != NULL) {
    *err = VFS_T_OK;
  }

  return (struct fs_node*)newDir;

  CLEENUP_LABEL(cleen_newDirSimlink, free_mem, newDir);
  CLEENUP_LABEL(cleen_currentDirSimlink, free_mem, currentDirSimlink);
  CLEENUP_DEFAULT_LABEL();

  return NULL;
}

fs_node_t* ramfs_newRegFile(const char* fileName, const char* parentDirAbsPath, int *err) {
  CLEENUP_FUNCTION_SET(kfree, free_mem);
  ramfs_file_header_t* newRegFile;
  ramfs_file_header_t* parentDir;

  if(err != NULL) {
    *err = VFS_T_NOK;
  }

  if(fs_checkFileName(fileName, NULL) != RET_T_OK) {
    CLEENUP(default);
  }

  parentDir = (ramfs_file_header_t*)fs_get_node(NULL, parentDirAbsPath);
  if(parentDir == NULL){
    CLEENUP(default);
  }

  if(fs_find((fs_node_t*)parentDir, fileName) != NULL){
    if(err != NULL) {
      *err = RFS_FILE_ALREADY_EXISTS;
    }
    CLEENUP(default);
  }

  newRegFile = ramfs_allocNode();
  if(newRegFile == NULL) {
    CLEENUP(default);
  }

  memset(newRegFile, 0, sizeof(ramfs_file_header_t));

  /* Prepare root descriptor */
  newRegFile->fs_node.open = ramfs_open;
  newRegFile->fs_node.close = ramfs_close;
  newRegFile->fs_node.read = ramfs_read;
  newRegFile->fs_node.write = ramfs_write;
  newRegFile->fs_node.find = NULL;
  newRegFile->fs_node.readdir = NULL;
  newRegFile->fs_node.length = 0;
  newRegFile->fs_node.ino = fs_new_ino_id();
  strncpy(newRegFile->fs_node.name, fileName, MAX_FNAME);
  newRegFile->fs_node.owner_group = newRegFile->fs_node.owner_id = 0;
  newRegFile->fs_node.perm = 0;
  newRegFile->fs_node.type = FTYPE_REGULAR;
  newRegFile->fs_node.node_ptr = NULL;
  newRegFile->fs_node.impl_def = 0;

  (ramfs_getLastChild_inter(parentDir))->next = newRegFile;

  if(err != NULL) {
    *err = VFS_T_OK;
  }

  return (struct fs_node*)newRegFile;

  CLEENUP_DEFAULT_LABEL();

  return NULL;
}

int ramfs_loadInitrd(const uint8_t* initrdPtr, char* path) {
  initrd_header* initrdHeader = (initrd_header*)initrdPtr;
  initrd_file_header* initrd_root =
      (initrd_file_header*)(((uintptr_t)initrdPtr) + sizeof(initrd_header));
  ramfs_file_header_t* curr_dir;
  ramfs_file_header_t* parent_dir;

  if (UINT32_INITRD_ORDER(initrdHeader->magic) != INITRD_MAGIC) {
    return VFS_T_NOK;
  }

  /* Used to optimize stack */
  initrd_base = (void*)initrdPtr;

  curr_dir = (ramfs_file_header_t*) fs_get_node(NULL, path);
  if(curr_dir == NULL || curr_dir->fs_node.type != FTYPE_DIRECTORY) {
    return VFS_T_NOK;
  }

  /* The second children is the simlink to the parent dir */
  parent_dir = ((ramfs_file_header_t*) &curr_dir->fs_node)->children;
  parent_dir = parent_dir->next;

  return LoadInitrdFiles((ramfs_file_header_t*)parent_dir->fs_node.node_ptr, curr_dir, initrd_root);
}

ssize_t ramfs_read(struct fs_node* node, void* buf, size_t nbyte,
                     off_t offset) {
  ramfs_file_header_t* ramfsNode = (ramfs_file_header_t*)node;
  size_t toReadLen = nbyte;

  if (node == NULL || buf == NULL || node->read == NULL ||
      node->type != FTYPE_REGULAR || offset > node->length) {
    // TODO: errno
    return -1;
  }

  if ((nbyte + offset) > node->length) {
    toReadLen = node->length - offset;
  }

  memcpy(buf, ramfsNode->filePtr  + offset, toReadLen);
  return toReadLen;
}

ssize_t ramfs_write(struct fs_node* node, const void* buf, size_t nbyte,
                      off_t offset) {
  CLEENUP_FUNCTION_SET(kfree, free_mem);
  ramfs_file_header_t* ramfsNode = (ramfs_file_header_t*)node;
  char* newFilePtr = NULL;

  if (node == NULL || buf == NULL || node->read == NULL ||
      node->type != FTYPE_REGULAR || offset > node->length) {
    CLEENUP(default);
  }

  if ((nbyte + offset) > node->length) {
    size_t newLen = nbyte + offset;
    newFilePtr = krealloc(ramfsNode->filePtr, newLen);
    if(newFilePtr == NULL) {
      CLEENUP(newFilePtr);
    }
    ramfsNode->filePtr = newFilePtr;
    node->length = newLen;
  }

  memcpy(ramfsNode->filePtr + offset, buf, nbyte);

  return nbyte;

  CLEENUP_LABEL(newFilePtr, free_mem, newFilePtr);
  CLEENUP_DEFAULT_LABEL();

  return VFS_T_NOK;
}

int ramfs_open(struct fs_node* node, const char* restrict __is_unused mode) {
  // TODO
  return VFS_T_OK;
}

int ramfs_close(struct fs_node* __is_unused node) {
  // TODO
  return VFS_T_OK;
}

int ramfs_readdir(struct fs_node* dir_node, uint32_t index,
                    struct dirent* entry, struct dirent** result) {
  ramfs_file_header_t* ramfsNode = (ramfs_file_header_t*)dir_node;
  int ret = VFS_T_NOK;
  uint32_t i = 0;

  *result = NULL;

  if (dir_node == NULL || dir_node->readdir == NULL ||
      dir_node->type != FTYPE_DIRECTORY) {
    // TODO errno
    return VFS_T_NOK;
  }
  if (ramfsNode->children == NULL) {
    *result = NULL;
    ret = VFS_T_OK;
  } else {
    ramfsNode = ramfsNode->children;
    i = 0;
    while (i != index && ramfsNode != NULL) {
      i++;
      ramfsNode = ramfsNode->next;
    }
    if (i != index || ramfsNode == NULL) {
      *result = NULL;
    } else {
      entry->d_ino = ramfsNode->fs_node.ino;
      entry->d_type = ramfsNode->fs_node.type;
      strncpy(entry->d_name, ramfsNode->fs_node.name, MAX_FNAME);
      entry->d_reclen = sizeof(dirent_t) - MAX_FNAME + strlen(entry->d_name);
      *result = entry;
    }
    ret = VFS_T_OK;
  }
  return ret;
}

struct fs_node* ramfs_find(struct fs_node* dir_node, const char* name) {
  ramfs_file_header_t* ramfsNode = (ramfs_file_header_t*)dir_node;
  fs_node_t* retNode = NULL;

  if (dir_node == NULL || dir_node->readdir == NULL ||
      dir_node->type != FTYPE_DIRECTORY || name == NULL) {
    // TODO errno
    return NULL;
  }

  if (ramfsNode->children != NULL) {
    ramfsNode = ramfsNode->children;
    do {
      if (strncmp(ramfsNode->fs_node.name, name, MAX_FNAME) == 0) {
        retNode = &ramfsNode->fs_node;
        break;
      }
      ramfsNode = ramfsNode->next;
    } while (ramfsNode != NULL);
  }

  return retNode;
}

int ramfs_addChild(struct fs_node* dirNode, struct fs_node* copyChild) {
  int ret = VFS_T_NOK;
  ramfs_file_header_t* parent = (ramfs_file_header_t*)dirNode;

  if (dirNode != NULL && dirNode->type == FTYPE_DIRECTORY) {
    ramfs_file_header_t* ramfs_child =
        (ramfs_file_header_t*)ramfs_allocNode();
    if(ramfs_child != NULL) {
        ramfs_child->next = NULL;
        ramfs_child->children = NULL;
        memcpy(&ramfs_child->fs_node, copyChild, sizeof(fs_node_t));
      if (parent->children == NULL) {
        parent->children = ramfs_child;
      } else {
        (ramfs_getLastChild_inter(parent))->next = ramfs_child;
      }
      ret = VFS_T_OK;
    }
  }
  return ret;
}

static ramfs_file_header_t* LoadInitrdRegFile(
    initrd_file_header* initrdFile) {
  ramfs_file_header_t* node = ramfs_allocNode();

  if(node != NULL) {
    node->fs_node.open = ramfs_open;
    node->fs_node.close = ramfs_close;
    node->fs_node.read = ramfs_read;
    node->fs_node.write = ramfs_write;
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

static ramfs_file_header_t* LoadInitrdDirFile(
    initrd_file_header* initrdFile) {
  ramfs_file_header_t* node = ramfs_allocNode();

  if(node != NULL) {
    node->fs_node.open = NULL;
    node->fs_node.close = NULL;
    node->fs_node.read = NULL;
    node->fs_node.write = NULL;
    node->fs_node.find = ramfs_find;
    node->fs_node.readdir = ramfs_readdir;
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

static int LoadInitrdFiles(ramfs_file_header_t* parent,
                           ramfs_file_header_t* currentDir,
                           initrd_file_header* initrdcurrentDir) {
  CLEENUP_FUNCTION_SET(kfree, free_mem);
  initrd_file_header* initrdchildren =
      (initrd_file_header*)(((uintptr_t)initrdcurrentDir) +
                            sizeof(initrd_file_header));
  ramfs_file_header_t* prev = NULL;
  ramfs_file_header_t* curr = NULL;
  ramfs_file_header_t* currentDirSimlink;
  ramfs_file_header_t* parentDirSimlink;
  uint32_t i = 0;

  if (currentDir->children == NULL) {
    /* Prepare simlink to the current node */
    currentDirSimlink = ramfs_allocNode();
    if(currentDirSimlink == NULL) {
      CLEENUP(default);
    }
    fs_get_simlink(".", &currentDir->fs_node, &currentDirSimlink->fs_node);

    /* Prepare simlink to the parent node */
    parentDirSimlink = ramfs_allocNode();
    if(parentDirSimlink == NULL) {
      CLEENUP(free_cur_dir_simlink);
    }
    fs_get_simlink("..", &parent->fs_node, &parentDirSimlink->fs_node);

    currentDir->children = currentDirSimlink;
    currentDirSimlink->next = parentDirSimlink;
    parentDirSimlink->next = NULL;
  } else {
    currentDirSimlink = currentDir->children;
    parentDirSimlink = currentDir->children->next;
  }

  for (i = 0; i < UINT32_INITRD_ORDER(initrdcurrentDir->dir_nfiles); i++) {
    if (initrdchildren[i].type == UINT32_INITRD_ORDER(INITRD_FTYPE_REGULAR)) {
      curr = LoadInitrdRegFile(&initrdchildren[i]);
    } else if (initrdchildren[i].type ==
               UINT32_INITRD_ORDER(INITRD_FTYPE_DIR)) {
      curr = LoadInitrdDirFile(&initrdchildren[i]);
      if(LoadInitrdFiles(currentDir, curr, &initrdchildren[i]) != VFS_T_OK) {
        return VFS_T_NOK;
      }
    } else {
      /* jump to the next cycle */
      continue;
    }
    if (parentDirSimlink->next == NULL) {
      parentDirSimlink->next = curr;
    } else {
      prev->next = curr;
    }
    prev = curr;
  }

  return VFS_T_OK;

  CLEENUP_LABEL(free_cur_dir_simlink, free_mem, currentDir);
  CLEENUP_DEFAULT_LABEL();

  return VFS_T_NOK;
}