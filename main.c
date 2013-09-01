/*
 * Anvil block finder
 * By John Hodge (thePowersGang)
 */
#include <stdio.h>
#include <stdlib.h>
#include "anvil.h"
#include <dirent.h>
#include <string.h>
#include <stdbool.h>

#define ANVIL_EXT	".mca"
#define ANVIL_EXT_LEN	4

#define BOOKSELF_BLOCKID	47
#define _STR(x)	#x
#define STR(x)	_STR(x)

const char *gsLevelDotDat;	// path to level.dat
const char *gsRegionDir;	// directory containing region data

uint16_t	giDesiredBlockId = BOOKSELF_BLOCKID;
bool	gbHaveDesiredData;
uint16_t	giDesiredDataValue;

 int	ParseCommandline(int argc, char *argv[]);
void	SearchRegionFile(const char *dir, const char *file);
void	SearchChunk(tAnvilChunk *chunkdata);
void	CheckIfIsDesiredBlock(tAnvilChunk *chunk, tAnvilChunkSect *sectp, size_t ofs, int x, int y, int z);

int main(int argc, char *argv[])
{
	if( ParseCommandline(argc, argv) ) {
		return -1;
	}

//	tAnvilLevelInfo *lvlinfo = Anvil_LoadLevelInfo(gsLevelDotDat);
//	if( !lvlinfo ) {
//		fprintf(stderr, "Cannot open level file '%s'\n", gsLevelDotDat);
//		return 1;
//	}
	
	DIR	*dp = opendir(gsRegionDir);
	struct dirent *de;
	while( (de = readdir(dp)) )
	{
		size_t len = strlen(de->d_name);
		if( len < ANVIL_EXT_LEN )
			continue ;
		if( strcmp(de->d_name + len-ANVIL_EXT_LEN, ANVIL_EXT) != 0 )
			continue ;
		
		SearchRegionFile(gsRegionDir, de->d_name);
	}
	
	return 0;
}

void Usage(const char *progname)
{
	fprintf(stderr, "Usage: %s [opts] <regiondir>\n", progname);
	fprintf(stderr, "\n"
		"-b  : Block ID to search for (Defaults to "STR(BOOKSELF_BLOCKID)"/Bookshelf)\n"
		);
}

int ParseCommandline(int argc, char *argv[])
{
	bool	no_more_flags = false;
	for( int i = 1; i < argc; i ++ )
	{
		const char *arg = argv[i];
		if( no_more_flags || arg[0] != '-' )
		{
			if( !gsRegionDir ) {
				gsRegionDir = arg;
			}
			else {
				Usage(argv[0]);
				return 1;
			}
		}
		else if( arg[1] != '-' )
		{
			// - arguments
			while( *++arg )
			{
				switch(*arg)
				{
				case 'b':
					giDesiredBlockId = atoi(argv[++i]);
					break;
				default:
					Usage(argv[0]);
					return 1;
				}
			}
		}
		else
		{
			// -- arguments
			if( arg[2] == '\0' ) {
				no_more_flags = true;
			}
			else {
				Usage(argv[0]);
				return 1;
			}
		}
	}
	return 0;
}

void SearchRegionFile(const char *dir, const char *file)
{
	char path[ strlen(dir) + 1 + strlen(file) + 1 ];
	snprintf(path, sizeof(path), "%s/%s", dir, file);
	
	tAnvilRegion *rgn = Anvil_LoadRegion(path);
	
	for( int x = 0; x < CHUNK_X_PER_RGN; x ++ )
	{
		for( int z = 0; z < CHUNK_Z_PER_RGN; z ++ )
		{
			tAnvilChunk *chunkdata = Anvil_GetRegionChunk(rgn, x, z);
			if( chunkdata ) {
				SearchChunk(chunkdata);
				Anvil_FreeChunk(chunkdata);
			}
		}
	}
	
	Anvil_FreeRegion(rgn);
}

void SearchChunk(tAnvilChunk *chunkdata)
{
	for( int sect = 0; sect < chunkdata->nSections; sect ++ )
	{
		tAnvilChunkSect	*sectp = chunkdata->Sections[sect];
		
		size_t	ofs = 0;
		for( int y = 0; y < 16; y ++ )
		{
			for( int z = 0; z < CHUNK_W; z ++ )
			{
				for( int x = 0; x < CHUNK_W; x ++, ofs ++ )
				{
					CheckIfIsDesiredBlock( chunkdata, sectp, ofs, x,y,z );
				}
			}
		}
	}
}

void CheckIfIsDesiredBlock(tAnvilChunk *chunk, tAnvilChunkSect *sectp, size_t ofs, int x, int y, int z)
{
	// TODO: allow searching for multiple block IDs
	if( sectp->Blocks[ofs] == giDesiredBlockId )
	{
		if( gbHaveDesiredData && sectp->DataVals[ofs] == giDesiredDataValue )
		{
			printf("Found %x:%x at XZY %i,%i,%i\n",
				sectp->Blocks[ofs], sectp->DataVals[ofs],
				chunk->CX*CHUNK_W + x, chunk->CZ*CHUNK_W + z, sectp->Y*16 + y);
		}
	}
}
