#include <stdio.h>
#include <errno.h>
#include <string.h>

#define EA_VERSION "1.0"
#define EA_NAME "ExtractArchiveFromBinary"
#define EA_AUTHOR "Christophe Calmejane"
#define EA_DATE "2004'09"
#define EA_COPYRIGHT EA_NAME " v" EA_VERSION " (c) " EA_AUTHOR " - " EA_DATE

int main(int argc, char *argv[])
{
  char  buf[65536];
  unsigned long int size,ofs;
  FILE *f;
  FILE *fout;
  unsigned long int total,packet;

  printf(EA_COPYRIGHT "\n");

  if(argc != 3)
  {
    fprintf(stderr,"Usage : " EA_NAME " <BinaryFile.exe> <OutputFile.ska>\n");
    return -1;
  }

  printf(" - Opening binary and output file ... ");
  fflush(stdout);

  if((fout = fopen(argv[2],"wb")) == NULL)
  {
    fprintf(stderr,"error opening output file %s (%s)\n",argv[2],strerror(errno));
    printf("Operation aborted !\n");
    return -2;
  }

  if((f = fopen(argv[1],"rb")) == NULL)
  {
    fprintf(stderr,"error opening binary file %s (%s)\n",argv[1],strerror(errno));
    printf("Operation aborted !\n");
    return -3;
  }
  printf("done\n");

  printf(" - Extracting resource from binary file %s ... ",argv[1]);
  fflush(stdout);

  fseek(f,-4,SEEK_END);
  if(fread(&ofs,4,1,f) != 1)
  {
    fclose(fout);
    fclose(f);
    fprintf(stderr,"error reading binary file %s (%s)\n",argv[1],strerror(errno));
    printf("Operation aborted !\n");
    return -4;
  }
  size = ftell(f) - ofs - 4;
  fseek(f,ofs,SEEK_SET);
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
  printf("done\n");

  fclose(fout);
  fclose(f);

  printf("Successfully extracted resource from binary file\n");

  return 0;
}
