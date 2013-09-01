/*
 * Anvil block finder
 * By John Hodge (thePowersGang)
 */
#include <stdio.h>
#include "anvil.h"
#include <stdlib.h>
#include <assert.h>
#include "compression.h"
#include "nbt.h"

#define SECTOR_BYTES	0x1000
#define SECTOR_INTS	(SECTOR_BYTES/4)

#define VERSION_GZIP	1
#define VERSION_DEFLATE	2

struct sAnvilRegion
{
	FILE	*FP;
	size_t	nSectors;
	uint32_t	Offsets[SECTOR_INTS];
	uint32_t	Timestamps[SECTOR_INTS];
};

// === CODE ===
static uint8_t _readInt8(FILE *fp)
{
	uint8_t rv = 0;
	fread(&rv, 1, 1, fp);	// if error, just returns 0
	return rv;
}
static uint32_t _readInt32(FILE *fp)
{
	uint32_t rv = 0;
	// Java is big-endian
	rv |= ((uint32_t)_readInt8(fp)) << 24;
	rv |= ((uint32_t)_readInt8(fp)) << 16;
	rv |= ((uint32_t)_readInt8(fp)) <<  8;
	rv |= ((uint32_t)_readInt8(fp)) <<  0;
	return rv;
}

tAnvilRegion *Anvil_LoadRegion(const char *Path)
{
	FILE	*fp = fopen(Path, "rb");

	printf("Path = %s\n", Path);	

	fseek(fp, 0, SEEK_END);
	int nSectors = (ftell(fp) + SECTOR_BYTES-1) / SECTOR_BYTES;
	fseek(fp, 0, SEEK_SET);
	
	tAnvilRegion *ret = malloc( sizeof(tAnvilRegion) );
	assert(ret);
	ret->FP = fp;
	ret->nSectors = nSectors;
	for( int i = 0; i < SECTOR_INTS; i ++ )
		ret->Offsets[i] = _readInt32(fp);
	for( int i = 0; i < SECTOR_INTS; i ++ )
		ret->Timestamps[i] = _readInt32(fp);
	
	return ret;
}

void Anvil_FreeRegion(tAnvilRegion *Rgn)
{
	fclose(Rgn->FP);
	free(Rgn);
}

uint8_t _getDataVal(tNBT_ByteArray *nbt_bytes, int offset)
{
	 int	byte = offset >> 1;
	 int	nibble = offset & 1;
	return (nbt_bytes->Data[byte] >> (nibble * 4)) & 0xF;
}

tAnvilChunk *Anvil_GetRegionChunk(tAnvilRegion *Region, int X, int Z)
{
	uint32_t offset = Region->Offsets[X + Z*32];
	if( offset == 0 ) {
		// Non-present chunk
		return NULL;
	}

	int sector_num = offset >> 8;
	int numSectors = offset & 0xFF;
	
	if( sector_num + numSectors >= Region->nSectors ) {
		// oops, crap data
		return NULL;
	}
	
	fseek(Region->FP, sector_num * SECTOR_BYTES, SEEK_SET);
	
	uint32_t stored_len = _readInt32(Region->FP);
	uint8_t	ver = _readInt8(Region->FP);

	if( stored_len > numSectors * SECTOR_BYTES ) {
		// More crap data
		return NULL;
	}

	void *compressed_data = calloc(1, stored_len-1);
	assert(compressed_data);
	fread(compressed_data, 1, stored_len-1, Region->FP);

	size_t	uncompressed_len;
	void	*uncompressed_data;
	switch(ver)
	{
	case VERSION_GZIP:
		assert(ver != VERSION_GZIP);
		break;
	case VERSION_DEFLATE:
		uncompressed_data = inflate_buffer(compressed_data, stored_len-1, &uncompressed_len);
		assert(uncompressed_data);
		break;
	default:
		// Unknown compression
		return NULL;
	}
	free(compressed_data);

	// Parse NBT
	tNBT	*nbt = NBT_Load(uncompressed_data, uncompressed_len);
	tNBT	*level = NBT_GetTag(nbt, "Level");
	tNBT_List *nbt_sects = NBT_GetList(level, "Sections");
	assert(nbt_sects);
	tAnvilChunk	*ret = malloc( sizeof(tAnvilRegion) + nbt_sects->Count * sizeof(tAnvilChunkSect*));
	assert(ret);
	
	ret->CX = NBT_GetInt(level, "xPos");
	ret->CZ = NBT_GetInt(level, "zPos");
	ret->nSections = nbt_sects->Count;

	for( int sidx = 0; sidx < nbt_sects->Count; sidx ++ )
	{
		tNBT	*nbt_sect = NBT_GetListItem(nbt_sects, sidx);
		assert(nbt_sect);
		tAnvilChunkSect	*sect = malloc( sizeof(tAnvilChunkSect) );
		ret->Sections[sidx] = sect;
		assert(sect);
		sect->Y = NBT_GetInt(nbt_sect, "Y");
		
		// Block IDs
		tNBT_ByteArray *nbt_blocks = NBT_GetByteArray(nbt_sect, "Blocks");
		assert(nbt_blocks);
		assert(nbt_blocks->Count == BLOCKS_PER_SECT);
		for( int i = 0; i < BLOCKS_PER_SECT; i ++ )
			sect->Blocks[i] = nbt_blocks->Data[i];
		// (optional) Extra bits for block IDs (4 per block)
		tNBT_ByteArray	*nbt_add = NBT_GetByteArray(nbt_sect, "Add");
		if( nbt_add )
		{
			assert(nbt_add->Count == BLOCKS_PER_SECT/2);
			for( int i = 0; i < BLOCKS_PER_SECT; i ++ )
				sect->Blocks[i] |= _getDataVal(nbt_add, i) << 8;
		}
		// Data values
		tNBT_ByteArray	*nbt_datavals = NBT_GetByteArray(nbt_sect, "Data");
		assert(nbt_datavals);
		assert(nbt_datavals->Count == BLOCKS_PER_SECT/2);
		for( int i = 0; i < BLOCKS_PER_SECT; i ++ )
			sect->DataVals[i] = _getDataVal(nbt_datavals, i);
	}
	NBT_Release(nbt);
	free(uncompressed_data);

	return ret;
}

void Anvil_FreeChunk(tAnvilChunk *Chunk)
{
	
}
