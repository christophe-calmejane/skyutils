/* Compilation command line
    cl ExtractArchive.c -I../src -DSU_USE_ARCH -c
    link ExtractArchive.obj ../src/windows/skyutils/debug/skyutils.lib /nodefaultlib:libcmtd
*/

#ifndef SU_USE_ARCH
#error "SU_USE_ARCH not defined ! Must enable it to compile this file"
#endif /* !SU_USE_ARCH */
#include <skyutils.h>

#define EA_VERSION "1.0"
#define EA_NAME "ExtractArchive"
#define EA_AUTHOR "Christophe Calmejane"
#define EA_DATE "2004'09"
#define EA_COPYRIGHT EA_NAME " v" EA_VERSION " (c) " EA_AUTHOR " - " EA_DATE

int main(int argc, char *argv[])
{
  SU_PArch Arch;
  SU_PRes Res;
  int i,count;
  char buf[1024];

  printf(EA_COPYRIGHT "\n");

  if(argc != 2)
  {
    fprintf(stderr,"Usage : " EA_NAME " <ArchiveFile.ska>\n");
    return -1;
  }

  printf(" - Opening resource file ... ");
  fflush(stdout);

  Arch = SU_AR_OpenArchive(argv[1]);
  if(Arch == NULL)
  {
    fprintf(stderr,"error opening resource file %s (%s)\n",argv[1],strerror(errno));
    printf("Operation aborted !\n");
    return -3;
  }
  printf("done\n");

  count = SU_AR_GetResourcesCount(Arch);
  printf(" - %d files from archive ... \n",count);

  for(i=0;i<count;i++)
  {
    Res = SU_AR_ReadRes(Arch,i,false);
    if(Res == NULL)
    {
      fprintf(stderr,"error reading infos\n");
      printf("Operation aborted !\n");
      return -4;
    }
    if(Res->Name)
    {
      snprintf(buf,sizeof(buf),"%s",Res->Name);
      printf("   - Extracting resource %d (Index=%d Name=%s) to %s ... ",i,Res->Index,Res->Name,Res->Name);
    }
    else
    {
      GetTempFileName(".","EA",0,buf);
      printf("   - Extracting resource %d (Index=%d) to %s ... ",i,Res->Index,buf);
    }
    fflush(stdout);
    if(!SU_AR_ReadResToFile(Arch,i,buf))
    {
      fprintf(stderr,"error reading resource\n");
      printf("Operation aborted !\n");
      return -5;
    }
    printf("done\n");
  }

  if(!SU_AR_CloseArchive(Arch))
  {
    fprintf(stderr,"error closing resource\n");
    return -6;
  }

  printf("Successfully extracted archive %s\n",argv[1]);

  return 0;
}
