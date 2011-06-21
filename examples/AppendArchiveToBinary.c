#include <stdio.h>
#include <errno.h>
#include <string.h>

#define AA_VERSION "1.1"
#define AA_NAME "AppendArchiveToBinary"
#define AA_AUTHOR "Christophe Calmejane"
#define AA_DATE "2004'07"
#define AA_COPYRIGHT AA_NAME " v" AA_VERSION " (c) " AA_AUTHOR " - " AA_DATE

int main(int argc, char *argv[])
{
  char  buf[65536];
  unsigned long int size,ofs;
  FILE *f;
  FILE *fout;
  unsigned long int total,packet;

  printf(AA_COPYRIGHT "\n");

  if(argc != 3)
  {
    fprintf(stderr,"Usage : " AA_NAME " <BinaryFile.exe> <ArchiveFile.ska>\n");
    return -1;
  }

  printf(" - Opening binary and resource file ... ");
  fflush(stdout);

  if((f = fopen(argv[2],"rb")) == NULL)
  {
    fprintf(stderr,"error opening resource file %s (%s)\n",argv[2],strerror(errno));
    printf("Operation aborted !\n");
    return -2;
  }

  if((fout = fopen(argv[1],"ab")) == NULL)
  {
    fprintf(stderr,"error opening binary file %s (%s)\n",argv[1],strerror(errno));
    printf("Operation aborted !\n");
    return -3;
  }
  printf("done\n");

  printf(" - Adding resource %s to binary file %s ... ",argv[2],argv[1]);
  fflush(stdout);

  fseek(f,0,SEEK_END);
  fseek(fout,0,SEEK_END);
  size = ftell(f);
  rewind(f);
  ofs = ftell(fout);
  total = 0;
  while(total < size)
  {
    if((size - total) > sizeof(buf))
      packet = sizeof(buf);
    else
      packet = size - total;
    fread(buf,packet,1,f);
    fwrite(buf,packet,1,fout);
    total += packet;
  }
  fwrite(&ofs,4,1,fout);
  printf("done\n");

  fclose(fout);
  fclose(f);

  printf("Successfully added resource to binary file\n");

  return 0;
}
