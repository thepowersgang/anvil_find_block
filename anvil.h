#ifndef _ANVIL_H_
#define _ANVIL_H_

#include <stdint.h>

#define CHUNK_X_PER_RGN	32
#define CHUNK_Z_PER_RGN	32
#define CHUNK_W	16
#define SECT_H	16
#define BLOCKS_PER_SECT	(CHUNK_W*CHUNK_W*SECT_H)

typedef struct sAnvilRegion tAnvilRegion;
typedef struct sAnvilLevelInfo	tAnvilLevelInfo;

typedef struct sAnvilChunk	tAnvilChunk;
typedef struct sAnvilChunkSect	tAnvilChunkSect;

struct sAnvilChunkSect
{
	 int	Y;
	uint16_t	Blocks[BLOCKS_PER_SECT];
	uint16_t	DataVals[BLOCKS_PER_SECT];
};

struct sAnvilChunk
{
	 int	CX, CZ;
	 int	nSections;
	tAnvilChunkSect	*Sections[];
};

extern tAnvilLevelInfo	*Anvil_LoadLevelInfo(const char *Path);

extern tAnvilRegion *Anvil_LoadRegion(const char *Path);
extern void	Anvil_FreeRegion(tAnvilRegion *Rgn);

extern tAnvilChunk	*Anvil_GetRegionChunk(tAnvilRegion *Region, int X, int Y);
extern void	Anvil_FreeChunk(tAnvilChunk *Chunk);

#endif
