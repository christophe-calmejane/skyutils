/* Compilation command line
    cl CreateArchive.c -I../src -DSU_USE_ARCH -c
    link CreateArchive.obj ../src/windows/skyutils/debug/skyutils.lib /nodefaultlib:libcmtd
*/

#ifndef SU_USE_ARCH
#error "SU_USE_ARCH not defined ! Must enable it to compile this file"
#endif /* !SU_USE_ARCH */
#include <skyutils.h>

#define AA_VERSION "1.1"
#define AA_NAME "CreateArchive"
#define AA_AUTHOR "Christophe Calmejane"
#define AA_DATE "2004'07"
#define AA_COPYRIGHT AA_NAME " v" AA_VERSION " (c) " AA_AUTHOR " - " AA_DATE

void PrintTypes(SU_AR_COMP_TYPE Types)
{
  fprintf(stderr,"Supported compression types :\n");
  if((Types & SU_ARCH_COMP_NONE) == SU_ARCH_COMP_NONE)
  {
    printf(" -> %d = No compression\n",SU_ARCH_COMP_NONE);
  }
  if((Types & SU_ARCH_COMP_Z) == SU_ARCH_COMP_Z)
  {
    printf(" -> %d = Z-lib compression\n",SU_ARCH_COMP_Z);
  }
  if((Types & SU_ARCH_COMP_BZ) == SU_ARCH_COMP_BZ)
  {
    printf(" -> %d = BZ-lib compression\n",SU_ARCH_COMP_BZ);
  }
  if((Types & SU_ARCH_COMP_LZO) == SU_ARCH_COMP_LZO)
  {
    printf(" -> %d = LZO-lib compression\n",SU_ARCH_COMP_LZO);
  }
}

int main(int argc, char *argv[])
{
  SU_PArch Arch;
  int i,count;
  SU_AR_COMP_TYPE Types,type;

  printf(AA_COPYRIGHT "\n");

  Types = SU_AR_SupportedComps();
  if(argc < 4)
  {
    fprintf(stderr,"Usage : " AA_NAME " <ArchiveFile.ska> <CompressionType> <File> [<File> ...]\n");
    PrintTypes(Types);
    return -1;
  }

  type = atoi(argv[2]);
  if((type & Types) == 0)
  {
    fprintf(stderr,"Unsupported compression type : %d\n",type);
    PrintTypes(Types);
    return -2;
  }

  printf(" - Creating resource file ... ");
  fflush(stdout);

  Arch = SU_AR_CreateArchive(argv[1]);
  if(Arch == NULL)
  {
    fprintf(stderr,"error creating resource file %s (%s)\n",argv[1],strerror(errno));
    printf("Operation aborted !\n");
    return -3;
  }
  printf("done\n");

  count = argc - 3;
  printf(" - Adding %d files to archive ... \n",count);

  for(i=0;i<count;i++)
  {
    printf("   - Adding %s ...",argv[3+i]);
    fflush(stdout);
    if(!SU_AR_AddResFile(Arch,argv[3+i],type,i+1,NULL))
    {
      fprintf(stderr,"error adding file to resource\n");
      printf("Operation aborted !\n");
      return -4;
    }
    printf("done\n");
  }

  printf(" - Creating resource file ... ");
  fflush(stdout);
  if(!SU_AR_CloseArchive(Arch))
  {
    fprintf(stderr,"error flushing resource\n");
    printf("Operation aborted !\n");
    return -5;
  }
  printf("done\n");

  printf("Successfully created archive %s, containing %d files\n",argv[1],count);

  return 0;
}
