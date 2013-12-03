/* Compilation command line
    cl TestBuffer.c -I../include -c
    link TestBuffer.obj ../src/windows/skyutils/debug/skyutils.lib /nodefaultlib:libcmtd
*/

#include <skyutils/skyutils.h>

int main(int argc, char *argv[])
{
	SU_PBuffer buffer;
	size_t reserved;
	char ch = 0;
	size_t remaining;
	
	buffer = SU_BF_Create(5,SU_BF_DEFAULT_POLICY);

	SU_BF_AddToBuffer(buffer,"Hello",strlen("Hello"));
	reserved = SU_BF_ReserveBytes(buffer,1);
	SU_BF_AddToBuffer(buffer,"World",strlen("World"));
	SU_BF_AddToBuffer(buffer,&ch,sizeof(ch));
	SU_BF_WriteToReservedBytes(buffer,reserved," ",strlen(" "));
	printf("Buffer (%d bytes): %s\n",SU_BF_GetBufferLength(buffer),SU_BF_GetBufferData(buffer));
	
	remaining = SU_BF_ConsumeBufferLength(buffer,strlen("Hello "));
	printf("Remaining (%d bytes): %s\n",remaining,SU_BF_GetBufferData(buffer));
	
	SU_BF_Empty(buffer);
	printf("After empty: %d bytes\n",SU_BF_GetBufferLength(buffer));
	
	SU_BF_Free(buffer);
	return 0;
}
