/****************************************************************/
/* Buffer unit                                                 */
/* (c) Christophe CALMEJANE (Ze KiLleR) - 1999-2014             */
/****************************************************************/

/*
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <skyutils/skyutils.h>

/* Internal functions */

static void SU_BF_Realloc(SU_PBuffer buffer)
{
	switch(buffer->policy)
	{
		case kSU_Buffer_GrowPolicy_Double:
			buffer->size *= 2;
			buffer->buffer = realloc(buffer->buffer,buffer->size);
			break;
	}
}


/* Exported API */

SU_PBuffer SU_BF_Alloc(void)
{
	SU_PBuffer buffer = (SU_PBuffer) malloc(sizeof(SU_TBuffer));
	memset(buffer,0,sizeof(SU_TBuffer));
	return buffer;
}

SU_PBuffer SU_BF_Create(size_t defaultSize,SU_Buffer_GrowPolicy policy)
{
	SU_PBuffer buffer = SU_BF_Alloc();
	SU_BF_Init(buffer,defaultSize,policy);
	return buffer;
}

void SU_BF_Init(SU_PBuffer buffer,size_t defaultSize,SU_Buffer_GrowPolicy policy)
{
	buffer->size = defaultSize;
	if(buffer->buffer != NULL)
		free(buffer->buffer);
	buffer->buffer = malloc(buffer->size);
	buffer->pos = 0;
	buffer->policy = policy;
}

void SU_BF_Free(SU_PBuffer buffer)
{
	if(buffer->buffer != NULL)
		free(buffer->buffer);
	free(buffer);
}

void SU_BF_Empty(SU_PBuffer buffer)
{
	buffer->pos = 0;
}

size_t SU_BF_ReserveBytes(SU_PBuffer buffer,size_t len)
{
	size_t pos = buffer->pos;

	// Check for enough room
	while((buffer->pos+len) > buffer->size)
	{
		SU_BF_Realloc(buffer);
	}

	// Copy data
	buffer->pos += len;

	// Return reserved position
	return pos;
}

size_t SU_BF_ConsumeBufferLength(SU_PBuffer buffer,size_t len)
{
	// Sanity check
	if(len > buffer->pos)
		len = buffer->pos;

	// Set buffer position to the correct place
	buffer->pos -= len;

	// Move remaining data
	if(buffer->pos > 0)
		memmove(buffer->buffer,(char*)(buffer->buffer)+len,buffer->pos);

	// Return remaining bytes in buffer
	return buffer->pos;
}

void SU_BF_AddToBuffer(SU_PBuffer buffer,void* data,size_t len)
{
	// Check for enough room
	while((buffer->pos+len) > buffer->size)
	{
		SU_BF_Realloc(buffer);
	}

	// Copy data
	memcpy((char*)(buffer->buffer)+buffer->pos,data,len);
	buffer->pos += len;
}

void SU_BF_WriteToReservedBytes(SU_PBuffer buffer,size_t position,void* data,size_t len)
{
	// Copy data
	memcpy((char*)(buffer->buffer)+position,data,len);
}

const void* SU_BF_GetBufferData(SU_PBuffer buffer)
{
	return buffer->buffer;
}

size_t SU_BF_GetBufferLength(SU_PBuffer buffer)
{
	return buffer->pos;
}

