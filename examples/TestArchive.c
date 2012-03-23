/* Compilation command line
    cl TestArchive.c -I../src -DSU_USE_ARCH -c
    link TestArchive.obj ../src/windows/skyutils/debug/skyutils.lib /nodefaultlib:libcmtd
*/

#ifndef SU_USE_ARCH
#error "SU_USE_ARCH not defined ! Must enable it to compile this file"
#endif /* !SU_USE_ARCH */
#include <skyutils.h>

int main(int argc, char *argv[])
{
  SU_PArch Arch;
  unsigned int i;

  Arch = SU_AR_OpenArchive(argv[0]);

  if(Arch == NULL)
  {
    printf("Failed to open self contained archive\n");
    return -1;
  }

  printf("%d resources in archive: \n",SU_AR_GetResourcesCount(Arch));
  for(i=0;i<SU_AR_GetResourcesCount(Arch);i++)
  {
    SU_PRes res = SU_AR_ReadRes(Arch,i,false);
    if(res == NULL)
    {
      printf("Failed to read resource %d\n",i);
      continue;
    }
    printf(" - %d : %s\n",res->Index,res->Name?res->Name:"NoName");
  }
  SU_AR_CloseArchive(Arch);

  return 0;
}
