#include <skyutils/skyutils.h>

#define REG_FILE "/tmp/registry.sur"

#define KEY_PATH_BASE "HKEY_CURRENT_USER\\Software\\SkyUtils\\Sample\\"
#define KEY_PATH_INT KEY_PATH_BASE "IntKey\\"
#define KEY_PATH_STR KEY_PATH_BASE "StrKey\\"
#define KEY_PATH_ENUM KEY_PATH_BASE "EnumKey\\"
#define KEY_INT KEY_PATH_INT "IntValue"
#define KEY_STR KEY_PATH_STR "StrValue"

#define DEFAULT_INT 15
#define SET_INT 13
#define DEFAULT_STR "Hello"
#define SET_STR "World"

int main()
{
  HKEY key;
  char buf[1024];
  int val,i;

  /* Opening registry */
  printf("Opening registry '%s'\n",REG_FILE);
  if(!SU_RB_OpenRegistry(REG_FILE))
  {
    printf("Error opening registry file : '%s'\n",REG_FILE);
    return -1;
  }

  /* Sample INT value */
  SU_RB_GetIntValue(KEY_INT, DEFAULT_INT, &val);
  printf("Get INT value from '%s' (default is %d) : %d\n",KEY_INT,DEFAULT_INT,val);

  printf("Call to Set INT value in '%s' with value %d\n",KEY_INT,SET_INT);
  SU_RB_SetIntValue(KEY_INT,SET_INT);

  SU_RB_GetIntValue(KEY_INT, DEFAULT_INT, &val);
  printf("Get INT value from '%s' (default is %d) : %d\n",KEY_INT,DEFAULT_INT,val);
  
  /* Sample STR value */
  SU_RB_GetStrValue(KEY_STR,buf,sizeof(buf),DEFAULT_STR);
  printf("Get STR value from '%s' (default is %s) : %s\n",KEY_STR,DEFAULT_STR,buf);

  printf("Call to Set STR value in '%s' with value %s\n",KEY_STR,SET_STR);
  SU_RB_SetStrValue(KEY_STR,SET_STR);

  SU_RB_GetStrValue(KEY_STR,buf,sizeof(buf),DEFAULT_STR);
  printf("Get STR value from '%s' (default is %s) : %s\n",KEY_STR,DEFAULT_STR,buf);
  
  /* Sample Default SubKey Value */
  SU_RB_GetIntValue(KEY_PATH_BASE, DEFAULT_INT, &val);
  printf("Get SubKey default INT value from '%s' (default is %d) : %d\n",KEY_PATH_BASE,DEFAULT_INT,val);

  printf("Call to Set INT value in Default SubKey '%s' with value %d\n",KEY_PATH_BASE,SET_INT);
  SU_RB_SetIntValue(KEY_PATH_BASE,SET_INT);

  SU_RB_GetIntValue(KEY_PATH_BASE, DEFAULT_INT, &val);
  printf("Get SubKey default INT value from '%s' (default is %d) : %d\n",KEY_PATH_BASE,DEFAULT_INT,val);
  
  /* Sample Open/Create Keys */
  key = SU_RB_OpenKeys(KEY_PATH_ENUM,0);
  printf("Trying to Open existing Key to '%s' : %s\n",KEY_PATH_ENUM,key == NULL?"failed":"success");
  
  /* Sample Enum Keys */
  printf("Creating INT values in '%s'\n",KEY_PATH_ENUM);
  SU_RB_SetIntValue(KEY_PATH_ENUM "val1",1);
  SU_RB_SetIntValue(KEY_PATH_ENUM "val2",2);
  SU_RB_SetIntValue(KEY_PATH_ENUM "val3",3);
  key = SU_RB_OpenKeys(KEY_PATH_ENUM,0);
  printf("Re-trying to Open existing Key to '%s' : %s\n",KEY_PATH_ENUM,key == NULL?"failed":"success");
  printf("Enumerating INT values in '%s'\n",KEY_PATH_ENUM);
  i = 0;
  while(SU_RB_EnumIntValue(key,i,buf,sizeof(buf),&val))
  {
    printf("\tFound INT value at idx %d : %s=%d\n",i,buf,val);
    i++;
  }
  
  /* Sample Delete Value/Keys */
  printf("Deleting Value at '%s'\n",KEY_INT);
  SU_RB_DelValue(KEY_INT); /* Manually delete INT sample value */
  printf("Deleting Keys to '%s'\n",KEY_PATH_STR);
  SU_RB_DelKey(KEY_PATH_STR); /* Delete recursively STR sample value */
  
  /* Close registry */
  printf("Closing registry '%s'\n",REG_FILE);
  if(!SU_RB_CloseRegistry())
  {
    printf("Error closing last opened registry file\n");
    return -1;
  }
}

