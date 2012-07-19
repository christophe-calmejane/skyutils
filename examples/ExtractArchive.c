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

void strreplace(char* str, const char* char_to_be_replaced, char char_to_replace) {
	const char* replaced_cursor = char_to_be_replaced;
	while(*replaced_cursor) {
		char* str_cursor = str;
		while(str_cursor)
		{
			str_cursor = strchr(str_cursor, *replaced_cursor);
			if(str_cursor)
			{
				*str_cursor = char_to_replace;
				str_cursor++;
			}
		}
		replaced_cursor++;
	}
}

int maine(int argc, char *argv[])
{
  SU_PArch Arch;
  SU_PRes Res;
  int i,count;
  char buf[1024];
  int current_param = 1 ;
  FILE* file_for_names = NULL ;

  printf(EA_COPYRIGHT "\n");

  if(argc < 2)
  {
    fprintf(stderr,"Usage : " EA_NAME " <ArchiveFile.ska> [[<ArchiveFile.lst>]]\n");
    return -1;
  }

  if(argc == 3)
  {
    printf(" - Opening names file ... ");
    file_for_names = fopen(argv[2],"r");
    if(file_for_names == NULL)
    {
      fprintf(stderr,"error opening names file %s (%s)\n",argv[2],strerror(errno));
      printf("Operation aborted !\n");
      return -7;
    }
    printf("done\n");
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
	  buf[0]='\0';
      if(file_for_names)
	  {
        char line_read[2048] = "";
        char line_to_find[2048] = "";
        bool end_of_file = false;
        rewind(file_for_names);
        snprintf(line_to_find,sizeof(line_to_find),"|%d|%s",Res->Index,Res->Name);
        while(!end_of_file)
        {
          if(strstr(line_read, line_to_find)) {
            break;
          }
          end_of_file = !SU_ReadLine(file_for_names,line_read,sizeof(line_read));
        }

        if(!end_of_file)
        {
          char* pos = strchr(line_read,'|');
		  SU_strcpy(buf,line_read,pos-line_read+1);
		  buf[pos-line_read+1]='\0' ;
        }
	  }

      if(buf[0]=='\0')
      {
        snprintf(buf,sizeof(buf),"%s",Res->Name);
        strreplace(buf, "\\/:*?\"<>|", '_') ;
	  }
      printf("   - Extracting resource %d (Index=%d Name=%s) to %s ... ",i,Res->Index,Res->Name,buf);
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
