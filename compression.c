/*
 * Anvil block finder
 * - By John Hodge (thePowersGang)
 *
 * compression.c
 * - Decompression shims 
 */
#include "compression.h"
#include <zlib.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// === CODE ===
const char *zlib_errstr(int ret)
{
	switch (ret)
	{
	case Z_ERRNO:
		return "Z_ERRNO";
	case Z_STREAM_ERROR:
		return "invalid compression level";
	case Z_DATA_ERROR:
		return "invalid or incomplete deflate data";
	case Z_MEM_ERROR:
		return "out of memory";
	case Z_VERSION_ERROR:
		return "zlib version mismatch!";
	default: {
		size_t	len = snprintf(NULL, 0, "unk zlib error %i", ret);
		char *buf = malloc(len+1);
		snprintf(buf, len+1, "unk zlib error %i", ret);
		return buf;
		}
	}
}


void *inflate_buffer(const void *Buffer, size_t Len, size_t *OutLenP)
{
	char	*ret = NULL;
	size_t	retLen = 0;
	z_stream	strm = {};
	 int	rv;
	
	// allocate inflate state
	strm.avail_in = Len;
	strm.next_in = (void*)Buffer;
	rv = inflateInit(&strm);
	if( rv != Z_OK ) {
		fprintf(stderr, "zlib error: %s\n", zlib_errstr(rv));
		return NULL;
	}

	do {
		char	outbuf[128*1024];
		strm.avail_out = sizeof(outbuf);
		strm.next_out = (void*)outbuf;
		
		rv = inflate(&strm, Z_NO_FLUSH);
		assert(rv != Z_STREAM_ERROR);
		switch(rv) {
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
		case Z_OK:
		case Z_STREAM_END:
			break;
		default:
			fprintf(stderr, "zlib error: %s\n", zlib_errstr(rv));
			free(ret);
			inflateEnd(&strm);
			return NULL;
		}
		
		size_t	have = sizeof(outbuf) - strm.avail_out;
		ret = realloc(ret, retLen + have);
		assert(ret);
		memcpy(ret+retLen, outbuf, have);
		retLen += have;
	} while( strm.avail_out == 0 );

	inflateEnd(&strm);

	*OutLenP = retLen;
	return ret;
}

