
#ifndef _RAMDISK_H_
#define _RAMDISK_H_
#include <coresys/vfs.h>
#include <libk/stdint.h>

int LoadInitrd(const uint8_t* initrdPtr);

fs_node_t* getRoot(void);

int ramdisk_add_child(struct fs_node* dirNode, struct fs_node* copyChild);

ssize_t ramdisk_read(struct fs_node* node, void* buf, size_t nbyte,
                    off_t offset);

ssize_t ramdisk_write(struct fs_node* node, const void* buf, size_t nbyte,
                     off_t offset);

int ramdisk_open(struct fs_node* node, const char* mode);

int ramdisk_close(struct fs_node* __is_unused node);

int ramdisk_readdir(struct fs_node* dir_node,
                    uint32_t index,
                    struct dirent *entry,
                    struct dirent **result);

struct fs_node* ramdisk_find(struct fs_node* dir_node, char* name);

#endif