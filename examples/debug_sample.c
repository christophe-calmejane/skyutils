#include <skyutils.h>

#define DBG_FLG_1 1
#define DBG_FLG_2 2
#define DBG_FLG_3 4
#define DBG_FLG_4 8
#define DBG_FLG_5 16
#define DBG_FLG_6 32
#define DBG_FLG_7 64
#define DBG_FLG_8 128
#define DBG_FLG_9 256

int main(int argc,char *argv[])
{
  /* Setting env var will override those values at the first SU_DBG_PrintDebug call */
  /* For example (see skyutils.h for list of all env var) :
    SU_DBG_OUTPUT="printf socket file" (will use printf, socket and file output)
    SU_DBG_FLAGS=3 (will print only DBG_FLG_1 AND DBG_FLG_2)
  */
  SU_DBG_SetFlags(DBG_FLG_1 | DBG_FLG_3 | DBG_FLG_8);
  //SU_DBG_SetOutput(SU_DBG_OUTPUT_PRINTF | SU_DBG_OUTPUT_FILE);
  SU_DBG_OUT_PRINTF_SetOptions(true);
  //SU_DBG_OUT_FILE_SetOptions("/tmp/plop",true);

#ifdef _WIN32
  /* Use special windows gui */
  SU_DBG_ChooseDebugOptions_Windows(GetModuleHandle(NULL),NULL);
#endif /* _WIN32 */

  SU_DBG_PrintDebug(DBG_FLG_1,"1-hello %s",argv[0]);
  SU_DBG_PrintDebug(DBG_FLG_2,"2-hello %s",argv[0]);
  SU_DBG_PrintDebug(DBG_FLG_3,"3-hello %s",argv[0]);
  SU_DBG_PrintDebug(DBG_FLG_4,"4-hello %s",argv[0]);
  SU_DBG_PrintDebug(DBG_FLG_5,"5-hello %s",argv[0]);
  SU_DBG_PrintDebug(DBG_FLG_6,"6-hello %s",argv[0]);
  SU_DBG_PrintDebug(DBG_FLG_7,"7-hello %s",argv[0]);
  SU_DBG_PrintDebug(DBG_FLG_8,"8-hello %s",argv[0]);
  SU_DBG_PrintDebug(DBG_FLG_9,"9-hello %s",argv[0]);

  return 0;
}

