#ifndef _ZLIB_SHIM_H_
#define _ZLIB_SHIM_H_

#include <stddef.h>

extern void *inflate_buffer(const void *srcbuffer, size_t srclen, size_t *uc_len);

#endif

