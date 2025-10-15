#ifndef _INITRD_H_
#define _INITRD_H_

#include <stdint.h>

#define MAX_FNAME_INITRD (128)

#define INITRD_FTYPE_REGULAR  (1)
#define INITRD_FTYPE_DIR      (2)
#define INITRD_MAGIC          (0x696E7264)

#ifdef __ORDER_LITTLE_ENDIAN__
  #define UINT32_INITRD_ORDER(input)   (input)
#else
  #define UINT32_INITRD_ORDER(input)   (((input >> 24)&0xff) | \
                                       ((input << 8) & 0xff0000) | \
                                       ((input >> 8) & 0xff00) | \
                                       ((inpu << 24) & 0xff000000)
#endif

typedef struct {
  uint32_t magic;
  uint32_t initrd_len;
  uint32_t nfiles;
  /* followed by root dir header */
} initrd_header;

typedef struct {
  char name[MAX_FNAME_INITRD];
  uint32_t id;
  uint32_t type;
  uint32_t dir_nfiles; /* 0 for regular files */
  uint32_t start_file_off; /* 0 for directories */
  uint32_t length;     /* regular file length or dir length */
  uint32_t checksum;  // TODO
  /* the file starts after the header */
} initrd_file_header;

#endif