#include <libk/string.h>
#include <libk/stddef.h>

void* memset(void* bufptr, int value, size_t size) {
  unsigned char* buf = (unsigned char*) bufptr;
  /* TODO: optimize copy */
  for (size_t i = 0; i < size; i++)
    buf[i] = (unsigned char) value;
  return bufptr;
}

errno_t memset_s(void *dest, rsize_t destsz, int c, rsize_t len) {
  errno_t ret = -1;
	rsize_t i = 0;
	unsigned char* buf = (unsigned char*) dest;
  if(destsz > RSIZE_MAX || len > RSIZE_MAX || len > destsz){
    /* TODO: manage errno */
    return -1;
  }
  if(dest == NULL) {
    /* TODO: manage errno */
    return -1;
  }
  /* TODO: optimize copy */
  for (i = 0; (i < len) && (i < destsz); i++)
    buf[i] = (unsigned char) c;

  return ret;
}