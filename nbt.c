/*
 * Anvil block finder
 * - By John Hodge (thePowersGang)
 * 
 * nbt.c
 * - Named Binary Tag format handling
 */
#include "nbt.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct sNBT_Int
{
	tNBT	Header;
	long long int	Value;
} tNBT_Int;

typedef struct sNBT_Float
{
	tNBT	Header;
	double	Value;
} tNBT_Float;

typedef struct sNBT_Compound
{
	tNBT	Header;
	tNBT	*FirstChild;
} tNBT_Compound;

typedef struct sNBT_String
{
	tNBT	Header;
	char	Data[];
} tNBT_String;

typedef struct
{
	const uint8_t *Data;
	size_t	Ofs;
	size_t	Len;
} tBuf;

// === PROTOTYPES ===
tNBT	*NBT_int_LoadNamed(tBuf *Buffer);
tNBT	*NBT_int_LoadTag(tBuf *Buffer, uint8_t Tag, const char *Name);

// === CODE ===
static void _readBytes(tBuf *Buffer, size_t Len, void *Dest)
{
	size_t	avail = Buffer->Len - Buffer->Ofs;
	if( avail < Len ) {
		memcpy(Dest, Buffer->Data + Buffer->Ofs, avail);
		memset((char*)Dest + avail, 0, Len - avail);
		Buffer->Ofs += avail;
	}
	else {
		memcpy(Dest, Buffer->Data + Buffer->Ofs, Len);
		Buffer->Ofs += Len;
	}
}
static uint8_t _readInt8(tBuf *Buffer)
{
	if( Buffer->Ofs == Buffer->Len )
		return 0;
	return Buffer->Data[Buffer->Ofs ++];
}
static uint16_t _readInt16(tBuf *Buffer)
{
	uint16_t rv = 0;
	rv |= ((uint16_t)_readInt8(Buffer)) <<  8;
	rv |= ((uint16_t)_readInt8(Buffer)) <<  0;
	return rv;
}
static uint32_t _readInt32(tBuf *Buffer)
{
	uint32_t rv = 0;
	// Java is big-endian
	rv |= ((uint32_t)_readInt8(Buffer)) << 24;
	rv |= ((uint32_t)_readInt8(Buffer)) << 16;
	rv |= ((uint32_t)_readInt8(Buffer)) <<  8;
	rv |= ((uint32_t)_readInt8(Buffer)) <<  0;
	return rv;
}
static uint64_t _readInt64(tBuf *Buffer)
{
	uint64_t rv = 0;
	// Java is big-endian
	rv |= ((uint64_t)_readInt32(Buffer)) << 32;
	rv |= ((uint64_t)_readInt32(Buffer)) <<  0;
	return rv;
}
// TODO: Is this a good assumption?
static float _readFloat32(tBuf *Buffer)
{
	float	rv = 0;
	_readBytes(Buffer, sizeof(rv), &rv);
	return rv;
}
static double _readFloat64(tBuf *Buffer)
{
	double	rv = 0;
	_readBytes(Buffer, sizeof(rv), &rv);
	return rv;
}

tNBT *NBT_Load(const void *Buffer, size_t Len)
{
	tBuf buffer = {Buffer, 0, Len};
	
	return NBT_int_LoadNamed(&buffer);
}

tNBT *NBT_int_LoadNamed(tBuf *Buffer)
{
	uint8_t type = _readInt8(Buffer);
	if( type == 0 ) {
		return NULL;
	}
	size_t	namelen = _readInt16(Buffer);
	char	name[namelen+1];
	_readBytes(Buffer, namelen, name);
	name[namelen] = 0;
	
	return NBT_int_LoadTag(Buffer, type, name);
}

tNBT *NBT_int_LoadTag(tBuf *Buffer, uint8_t Tag, const char *Name)
{
	tNBT	*ret;
	switch(Tag)
	{
	case TAG_End:
		return NULL;
	case TAG_Byte:
		ret = malloc(sizeof(tNBT_Int));
		((tNBT_Int*)ret)->Value = _readInt8(Buffer);
		break;
	case TAG_Short:
		ret = malloc(sizeof(tNBT_Int));
		((tNBT_Int*)ret)->Value = _readInt16(Buffer);
		break;
	case TAG_Int:
		ret = malloc(sizeof(tNBT_Int));
		((tNBT_Int*)ret)->Value = _readInt32(Buffer);
		break;
	case TAG_Long:
		ret = malloc(sizeof(tNBT_Int));
		((tNBT_Int*)ret)->Value = _readInt64(Buffer);
		break;
	case TAG_Float:
		ret = malloc(sizeof(tNBT_Float));
		((tNBT_Float*)ret)->Value = _readFloat32(Buffer);
		break;
	case TAG_Double:
		ret = malloc(sizeof(tNBT_Float));
		((tNBT_Float*)ret)->Value = _readFloat64(Buffer);
		break;
	case TAG_Byte_Array: {
		size_t len = _readInt32(Buffer);
		tNBT_ByteArray *array = malloc(sizeof(tNBT_ByteArray) + len);
		ret = &array->Header;
		array->Count = len;
		_readBytes(Buffer, len, array->Data);
		break; }
	case TAG_String: {
		size_t	len = _readInt16(Buffer);
		tNBT_String *str = malloc( sizeof(tNBT_String) + len + 1 );
		ret = &str->Header;
		_readBytes(Buffer, len, str->Data);
		str->Data[len] = 0;
		break; }
	case TAG_List: {
		uint8_t	list_tag = _readInt8(Buffer);
		size_t len = _readInt32(Buffer);
		tNBT_List *list = malloc(sizeof(tNBT_List) + len*sizeof(tNBT*));
		ret = &list->Header;
		list->Count = len;
		for( int i = 0; i < len; i ++ ) {
			list->Children[i] = NBT_int_LoadTag(Buffer, list_tag, "");
		}
		break; }
	case TAG_Compound: {
		tNBT_Compound *comp = malloc(sizeof(tNBT_Compound));
		ret = &comp->Header;
		tNBT *child = NBT_int_LoadNamed(Buffer);
		comp->FirstChild = child;
		while( child )
		{
			child->Next = NBT_int_LoadNamed(Buffer);
			child = child->Next;
		}
		break; }
	case TAG_Int_Array: {
		size_t	len = _readInt32(Buffer);
		tNBT_IntArray	*array = malloc( sizeof(tNBT_IntArray) + len*sizeof(uint32_t));
		ret = &array->Header;
		for( int i = 0; i < len; i ++ )
			array->Data[i] = _readInt32(Buffer);
		break; }
	default:
		fprintf(stderr, "NBT_int_LoadBuf: Unknown NBT tag %i\n", Tag);
		exit(1);
	}
	ret->Tag = Tag;
	ret->Name = strdup(Name);
	ret->Next = NULL;
	return ret;
}

void NBT_Release(tNBT *NBT)
{
	switch(NBT->Tag)
	{
	case TAG_Compound: {
		tNBT_Compound *comp = (void*)NBT;
		while( comp->FirstChild ) {
			tNBT *n = comp->FirstChild->Next;
			NBT_Release(comp->FirstChild);
			comp->FirstChild = n;
		}
		break; }
	case TAG_List: {
		tNBT_List *list = (void*)NBT;
		for( int i = 0; i < list->Count; i ++ )
			NBT_Release(list->Children[i]);
		break; }
	default:
		break;
	}
	free((void*)NBT->Name);
	free(NBT);
}

tNBT *NBT_GetTag(tNBT *Parent, const char *Name)
{
	if( Parent->Tag != TAG_Compound )
		return NULL;
	tNBT_Compound *comp = (void*)Parent;
	for(tNBT *tag = comp->FirstChild; tag; tag = tag->Next )
	{
		if(strcmp(tag->Name, Name) == 0)
			return tag;
	}
	return NULL;
}

tNBT *NBT_int_GetTag_Type(tNBT *Parent, const char *Name, enum eNBTTags Tag)
{
	tNBT	*tag = NBT_GetTag(Parent, Name);
	if( tag && tag->Tag != Tag )
	{
		fprintf(stderr, "Type mismatch getting tag '%s', exp %i got %i\n",
			Name, Tag, tag->Tag);
		return NULL;
	}
	return tag;
}

int32_t	NBT_GetInt(tNBT *Parent, const char *Name)
{
	tNBT	*tag = NBT_int_GetTag_Type(Parent, Name, TAG_Int);
	return tag ? ((tNBT_Int*)tag)->Value : 0;
}
int8_t NBT_GetByte(tNBT *Parent, const char *Name)
{
	tNBT	*tag = NBT_int_GetTag_Type(Parent, Name, TAG_Byte);
	return tag ? ((tNBT_Int*)tag)->Value : 0;
}

tNBT_ByteArray *NBT_GetByteArray(tNBT *Parent, const char *Name)
{
	return (void*)NBT_int_GetTag_Type(Parent, Name, TAG_Byte_Array);
}

tNBT_List *NBT_GetList(tNBT *Parent, const char *Name)
{
	return (void*)NBT_int_GetTag_Type(Parent, Name, TAG_List);
}

tNBT *NBT_GetListItem(tNBT_List *List, int Index)
{
	if(Index >= List->Count)
		return NULL;
	return List->Children[Index];
}

