#include <coresys/devfs.h>

static fs_node_t* devDir = NULL;
fs_node_t nullNode;

/* /dev/null functions declarations */
ssize_t null_read(struct fs_node* node, void* buf, size_t nbyte,
                     off_t offset);
ssize_t null_write(struct fs_node* node, const void* buf, size_t nbyte,
                      off_t offset);
int null_open(struct fs_node* node, const char* restrict __is_unused mode);
int null_close(struct fs_node* __is_unused node);

int devfs_init(void) {
  int ret = RET_T_NOK;
  if(devDir == NULL) {
    devDir = fs_get_node(NULL, "/dev");
    if(devDir != NULL) {
      /* Add /dev/null file */
      nullNode.open = null_open;
      nullNode.close = null_close;
      nullNode.read = null_read;
      nullNode.write = null_write;
      nullNode.type = FTYPE_REGULAR;
      nullNode.readdir = NULL;
      nullNode.perm = 0; //TODO rw for everyone
      nullNode.owner_group = nullNode.owner_id = 0;
      nullNode.find = NULL;
      nullNode.node_ptr = NULL;
      nullNode.length = 0;
      strcpy(nullNode.name, "null", sizeof("null"));
      nullNode.ino = fs_new_ino_id();
      nullNode.impl_def = 0;
      devfs_add_device_node(&nullNode);
    }
  }
  return ret;
}

/* /dev/null functions definitions */
ssize_t null_read(struct fs_node* __is_unused node, void* buf, size_t __is_unused nbyte,
                     off_t __is_unused offset) {
  /* EOF reached simulation */
  return 0;
}

ssize_t null_write(struct fs_node* __is_unused node, const void* __is_unused buf, size_t nbyte,
                      off_t __is_unused offset) {
  return nbyte;
}

int null_open(struct fs_node* node, const char* restrict __is_unused mode) {
  return RET_T_OK;
}

int null_close(struct fs_node* __is_unused node) {
  return RET_T_OK;
}