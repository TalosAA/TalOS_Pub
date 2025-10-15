#ifndef _DEV_FILESYSTEM_H_
#define _DEV_FILESYSTEM_H_

#include <libk/stdint.h>
#include <coresys/vfs.h>

int devfs_init(void);
int devfs_add_device_node(struct fs_node* devNode);
void devfs_del_device_node(const char* devName);

#endif