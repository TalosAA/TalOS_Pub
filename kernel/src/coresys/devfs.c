#include <coresys/devfs.h>
#include <coresys/ramfs.h>

/* Memorized for optimize access */
static fs_node_t* nullNodeFile;

/* /dev/null functions declarations */
static ssize_t null_read(struct fs_node* __is_unused node, void* buf, size_t __is_unused nbyte,
                     off_t __is_unused offset);
static ssize_t null_write(struct fs_node* __is_unused node, const void* __is_unused buf, size_t nbyte,
                      off_t __is_unused offset);

int devfs_init(void) {
  int ret = RET_T_NOK;
  fs_node_t* devDir = NULL;
  ramfs_file_header_t* nullNodeAddr = NULL;
  devDir = ramfs_mkDir("dev", "/", NULL);
  if(devDir != NULL) {
    /* Add /dev/null file */
    nullNodeAddr = kmalloc(sizeof(ramfs_file_header_t));
    if(nullNodeAddr != NULL){ 
      nullNodeAddr->children = NULL;
      nullNodeAddr->filePtr = NULL;
      nullNodeAddr->next = NULL;
      nullNodeFile = (fs_node_t*)nullNodeAddr;
      nullNodeFile->open = NULL;
      nullNodeFile->close = NULL;
      nullNodeFile->read = null_read;
      nullNodeFile->write = null_write;
      nullNodeFile->type = FTYPE_REGULAR;
      nullNodeFile->readdir = NULL;
      nullNodeFile->perm = 0; //TODO rw for everyone
      nullNodeFile->owner_group = nullNodeFile->owner_id = 0;
      nullNodeFile->find = NULL;
      nullNodeFile->node_ptr = NULL;
      nullNodeFile->length = 0;
      strcpy(nullNodeFile->name, "null", sizeof("null"));
      nullNodeFile->ino = fs_new_ino_id();
      nullNodeFile->impl_def = 0;
      ((ramfs_file_header_t*)ramfs_getLastChild(devDir))->next = nullNodeFile;
      ret = RET_T_OK;
    }
  }
  return ret;
}

/* /dev/null functions definitions */
static ssize_t null_read(struct fs_node* __is_unused node, void* buf, size_t __is_unused nbyte,
                     off_t __is_unused offset) {
  /* EOF reached simulation */
  return 0;
}

static ssize_t null_write(struct fs_node* __is_unused node, const void* __is_unused buf, size_t nbyte,
                      off_t __is_unused offset) {
  return nbyte;
}
