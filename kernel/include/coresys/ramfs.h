
#ifndef _RAMFS_H_
#define _RAMFS_H_

#include <coresys/vfs.h>
#include <libk/stdint.h>

/**
 * RAM File System errors codes
 */
#define RFS_OK                      (0)
#define RFS_NOK                     (-1)
#define RFS_FILE_ALREADY_EXISTS     (1)

int ramfs_init(void);

int ramfs_loadInitrd(const uint8_t* initrdPtr, char* path);

int ramfs_add_child(struct fs_node* dirNode, struct fs_node* copyChild);

ssize_t ramfs_read(struct fs_node* node, void* buf, size_t nbyte,
                    off_t offset);

ssize_t ramfs_write(struct fs_node* node, const void* buf, size_t nbyte,
                     off_t offset);

int ramfs_open(struct fs_node* node, const char* mode);

int ramfs_close(struct fs_node* __is_unused node);

int ramfs_readdir(struct fs_node* dir_node,
                    uint32_t index,
                    struct dirent *entry,
                    struct dirent **result);

struct fs_node* ramfs_find(struct fs_node* dir_node, char* name);

fs_node_t* ramfs_mkDir(const char* dirName, const char* parentDirAbsPath, int *err);

fs_node_t* ramfs_newRegFile(const char* fileName, const char* parentDirAbsPath, int *err);

int ramfs_deleteNode(const char* nodeAbsPath);

#endif