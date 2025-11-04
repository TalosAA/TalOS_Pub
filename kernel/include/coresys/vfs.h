#ifndef _VIRTUAL_FILE_SYSTEM_H_
#define _VIRTUAL_FILE_SYSTEM_H_

#include <coresys/types.h>
#include <libk/stddef.h>
#include <libk/stdint.h>

#define VFS_T_NOK (-1)
#define VFS_T_OK  (0)

#define MAX_FNAME (128)
#define MAX_PATH  (255)

#define FTYPE_REGULAR     (0x00000001)
#define FTYPE_DIRECTORY   (0x00000002)
#define FTYPE_BLOCKDEV    (0x00000004)
#define FTYPE_CHARDEV     (0x00000008)
#define FTYPE_FIFO        (0x00000010)
#define FTYPE_SYMLINK     (0x00000020)
#define FTYPE_SOCK        (0x00000040)

typedef uint32_t ino_t;
typedef uint32_t fs_type_t;
typedef size_t off_t;
struct fs_node;
struct dirent;

typedef ssize_t (*fs_read_fun_t)(struct fs_node* node, void* buf, size_t nbyte,
                                 off_t offset);
typedef ssize_t (*fs_write_fun_t)(struct fs_node* node, const void* buf,
                                  size_t nbyte, off_t offset);
typedef int (*fs_open_fun_t)(struct fs_node* node, const char* restrict mode);
typedef int (*fs_close_fun_t)(struct fs_node* node);
typedef int (*fs_readdir_fun_t)(struct fs_node* dir_node,
                                uint32_t index,
                                struct dirent *entry,
                                struct dirent **result);
typedef struct fs_node* (*fs_find_fun_t)(struct fs_node* dir_node,
                                         const char* name);

typedef struct fs_node {
  char name[MAX_FNAME];     /* Filename */
  ino_t ino;                /* Unique id of the file */
  uint32_t owner_id;        /* Id of the file owner */
  uint32_t owner_group;     /* Id of the file group owner */
  uint32_t perm;            /* Permissions */
  fs_type_t type;           /* File Type */
  size_t length;            /* File Length */
  uint64_t impl_def;        /* Implementation defined parameter */
  fs_open_fun_t open;       /* Open Callback */
  fs_close_fun_t close;     /* Close Callback */
  fs_read_fun_t read;       /* Read Callback */
  fs_write_fun_t write;     /* Write Callback */
  fs_readdir_fun_t readdir; /* Read directory Callback */
  fs_find_fun_t find;       /* Find file in directory Callback */
  struct fs_node* node_ptr;
} fs_node_t;

typedef struct dirent {
  ino_t d_ino;             /* Unique id of the file */
  unsigned short d_reclen; /* Length of this record */
  unsigned char d_type;    /* Type of file */
  char d_name[MAX_FNAME];  /* Null-terminated filename */
} dirent_t;

void fs_set_root(struct fs_node* root_node);
struct fs_node* fs_get_root(void);
struct fs_node* fs_get_node(struct fs_node* current, const char * path);
void fs_get_simlink(const char* simlinkName, struct fs_node* inNode,
                    struct fs_node* simlink);
ssize_t fs_read(struct fs_node* node, void* buf, size_t nbyte, off_t offset);
ssize_t fs_write(struct fs_node* node, const void* buf, size_t nbyte,
                 off_t offset);
int fs_open(struct fs_node* node, const char* restrict mode);
int fs_close(struct fs_node* node);
int fs_readdir(struct fs_node* dir_node,
               uint32_t index,
               struct dirent *entry,
               struct dirent **result);
struct fs_node* fs_find(struct fs_node* dir_node, const char* name);

ino_t fs_new_ino_id(void);

char * fs_getFileName(const char* filePath);

int fs_getParentPath(const char* filePath, char* parentPath);

int fs_checkFileName(const char* fileName, char** errChar);

#endif