#ifndef _NBT_H_
#define _NBT_H_

#include <stddef.h>
#include <stdint.h>

enum eNBTTags
{
	TAG_End,
	TAG_Byte,
	TAG_Short,
	TAG_Int,
	TAG_Long,
	TAG_Float,
	TAG_Double,
	TAG_Byte_Array,
	TAG_String,
	TAG_List,
	TAG_Compound,
	TAG_Int_Array
};

typedef struct sNBT
{
	enum eNBTTags	Tag;
	const char	*Name;
	struct sNBT	*Next;
} tNBT;

typedef struct sNBT_List
{
	tNBT	Header;
	enum eNBTTags	Payload;
	size_t	Count;
	tNBT	*Children[];
} tNBT_List;

typedef struct sNBT_ByteArray
{
	tNBT	Header;
	size_t	Count;
	uint8_t	Data[];
} tNBT_ByteArray;

typedef struct sNBT_IntArray
{
	tNBT	Header;
	size_t	Count;
	uint32_t	Data[];
} tNBT_IntArray;

extern tNBT	*NBT_Load(const void *Buffer, size_t Len);
extern void	NBT_Release(tNBT *NBT);

extern tNBT	*NBT_GetTag(tNBT *Parent, const char *Name);
extern int32_t	NBT_GetInt(tNBT *Parent, const char *Name);
extern tNBT_ByteArray	*NBT_GetByteArray(tNBT *Parent, const char *Name);
extern tNBT_List	*NBT_GetList(tNBT *Parent, const char *Name);

extern tNBT	*NBT_GetListItem(tNBT_List *List, int Index);

#endif

