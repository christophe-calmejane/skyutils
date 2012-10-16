/****************************************************************/
/* Win32 registry functions                                     */
/* (c) Christophe CALMEJANE (Ze KiLleR) - 1999-2011             */
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


#include "skyutils.h"
static int SU_RB_LastError = SU_RB_ERR_SUCCESS;
static int SU_RB_Access64Mode = 0;

#ifdef _WIN32
#pragma warning( disable: 4100)
#endif /* _WIN32 */

#ifndef SU_TRACE_INTERNAL
#ifdef SU_MALLOC_TRACE
#undef malloc
#undef calloc
#undef realloc
#undef strdup
#undef free
#endif /* SU_MALLOC_TRACE */
#endif /* !SU_TRACE_INTERNAL */

#ifdef _WIN32

#include <winreg.h>

SKYUTILS_API HKEY SU_RB_OpenKeys(const char Key[],int Access)
{
  HKEY H,H2;
  LONG res;
  char *tmp,*p,*q;

  if(Key == NULL)
  {
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    return 0;
  }
  tmp = SU_strdup(Key);
  p = SU_strparse(tmp,'\\');
  if(p == NULL)
  {
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    free(tmp);
    return 0;
  }
  if(strcmp(p,"HKEY_CLASSES_ROOT") == 0)
    H = HKEY_CLASSES_ROOT;
  else if(strcmp(p,"HKEY_CURRENT_USER") == 0)
    H = HKEY_CURRENT_USER;
  else if(strcmp(p,"HKEY_LOCAL_MACHINE") == 0)
    H = HKEY_LOCAL_MACHINE;
  else if(strcmp(p,"HKEY_USERS") == 0)
    H = HKEY_USERS;
  else
  {
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    free(tmp);
    return 0;
  }

  p = SU_strparse(NULL,'\\');
  if(p == NULL)
  {
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    free(tmp);
    return 0;
  }
  q = SU_strparse(NULL,'\\');
  while(q != NULL)
  {
    res = RegOpenKeyEx(H,p,0,SU_RB_Access64Mode | KEY_READ | Access,&H2);
    RegCloseKey(H);
    H = H2;
    if(res != ERROR_SUCCESS)
    {
      free(tmp);
      switch(res)
      {
        case ERROR_ACCESS_DENIED:
          break;
        case ERROR_FILE_NOT_FOUND:
          break;
        default:
          break;
      }
      if(res == ERROR_ACCESS_DENIED)
        SU_RB_LastError = SU_RB_ERR_ACCESS_DENIED;
      else
        SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
      return 0;
    }
    p = q;
    q = SU_strparse(NULL,'\\');
  }
  free(tmp);
  SU_RB_LastError = SU_RB_ERR_SUCCESS;
  return H;
}

SKYUTILS_API HKEY SU_RB_CreateKeys(const char Key[])
{
  HKEY H,H2;
  LONG res;
  DWORD Ret;
  char *tmp,*p,*q;

  if(Key == NULL)
  {
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    return 0;
  }
  tmp = SU_strdup(Key);
  p = SU_strparse(tmp,'\\');
  if(p == NULL)
  {
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    free(tmp);
    return 0;
  }
  if(strcmp(p,"HKEY_CLASSES_ROOT") == 0)
    H = HKEY_CLASSES_ROOT;
  else if(strcmp(p,"HKEY_CURRENT_USER") == 0)
    H = HKEY_CURRENT_USER;
  else if(strcmp(p,"HKEY_LOCAL_MACHINE") == 0)
    H = HKEY_LOCAL_MACHINE;
  else if(strcmp(p,"HKEY_USERS") == 0)
    H = HKEY_USERS;
  else
  {
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    free(tmp);
    return 0;
  }

  p = SU_strparse(NULL,'\\');
  if(p == NULL)
  {
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    free(tmp);
    return 0;
  }
  q = SU_strparse(NULL,'\\');
  while(q != NULL)
  {
    res = RegCreateKeyEx(H,p,0,"",REG_OPTION_NON_VOLATILE,SU_RB_Access64Mode | KEY_ALL_ACCESS,NULL,&H2,&Ret);
    RegCloseKey(H);
    H = H2;
    if(res != ERROR_SUCCESS)
    {
      free(tmp);
      if(res == ERROR_ACCESS_DENIED)
        SU_RB_LastError = SU_RB_ERR_ACCESS_DENIED;
      else
        SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
      return 0;
    }
    p = q;
    q = SU_strparse(NULL,'\\');
  }
  free(tmp);
  SU_RB_LastError = SU_RB_ERR_SUCCESS;
  return H;
}

SKYUTILS_API bool SU_RB_GetStrValue(const char Key[],char *buf,int buf_len,const char Default[])
{
  HKEY Handle;
  char *p;
  DWORD VType;
  DWORD Size;
  LONG R;

  SU_strcpy(buf,Default,buf_len);
  Handle = SU_RB_OpenKeys(Key,0);
  if(Handle == 0)
    return true; /* Returns true even if key not there, with the Default value */
  p = strrchr(Key,'\\');
  if(p == NULL)
  {
    RegCloseKey(Handle);
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    return false;
  }
  p++;
  Size = buf_len;
  if(RegQueryValueEx(Handle,p,NULL,&VType,NULL,NULL) == ERROR_SUCCESS) /* Only check for Type if no error */
  {
    if(VType != REG_SZ)
    {
      RegCloseKey(Handle);
      SU_RB_LastError = SU_RB_ERR_WRONG_TYPE;
      return false;
    }
  }
  R = RegQueryValueEx(Handle,p,NULL,NULL,(LPBYTE)buf,&Size);
  RegCloseKey(Handle);
  if(R != ERROR_SUCCESS)
  {
    if(R == ERROR_MORE_DATA)
    {
      SU_RB_LastError = SU_RB_ERR_MORE_DATA;
      return false;
    }
  }
  return true; /* Returns true even if key not there, with the Default value */
}

SKYUTILS_API bool SU_RB_GetStrLength(const char Key[],int *length)
{
  HKEY Handle;
  char *p;
  DWORD VType;
  LONG R;

  Handle = SU_RB_OpenKeys(Key,0);
  if(Handle == 0)
  {
    SU_RB_LastError = SU_RB_ERR_NO_SUCH_KEY;
    return false;
  }
  p = strrchr(Key,'\\');
  if(p == NULL)
  {
    RegCloseKey(Handle);
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    return false;
  }
  p++;
  if(RegQueryValueEx(Handle,p,NULL,&VType,NULL,NULL) == ERROR_SUCCESS) /* Only check for Type if no error */
  {
    if(VType != REG_SZ)
    {
      RegCloseKey(Handle);
      SU_RB_LastError = SU_RB_ERR_WRONG_TYPE;
      return false;
    }
  }
  R = RegQueryValueEx(Handle,p,NULL,NULL,NULL,(unsigned long *)length);
  RegCloseKey(Handle);
  if(R != ERROR_SUCCESS)
  {
    if(R == ERROR_MORE_DATA)
    {
      SU_RB_LastError = SU_RB_ERR_MORE_DATA;
      return false;
    }
  }
  return true;
}

SKYUTILS_API bool SU_RB_GetIntValue(const char Key[],int *Value,int Default)
{
  HKEY Handle;
  char *p;
  int Val;
  DWORD VType;
  DWORD Size;
  LONG R;

  *Value = Default;
  Handle = SU_RB_OpenKeys(Key,0);
  if(Handle == 0)
    return true; /* Returns true even if key not there, with the Default value */
  p = strrchr(Key,'\\');
  if(p == NULL)
  {
    RegCloseKey(Handle);
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    return false;
  }
  p++;
  Size = sizeof(Val);
  if(RegQueryValueEx(Handle,p,NULL,&VType,NULL,NULL) == ERROR_SUCCESS) /* Only check for Type if no error */
  {
    if(VType != REG_DWORD)
    {
      RegCloseKey(Handle);
      SU_RB_LastError = SU_RB_ERR_WRONG_TYPE;
      return false;
    }
  }
  R = RegQueryValueEx(Handle,p,NULL,NULL,(BYTE *)&Val,&Size);
  if(R == ERROR_SUCCESS)
    *Value = Val;
  RegCloseKey(Handle);
  return true; /* Returns true even if key not there, with the Default value */
}

SKYUTILS_API bool SU_RB_GetBinValue(const char Key[],char *buf,int buf_len,char *Default,int def_len)
{
  HKEY Handle;
  char *p;
  DWORD VType;
  DWORD Size;
  LONG R;

  if(Default != NULL)
  {
    memcpy(buf,Default,buf_len);
    if(buf_len < def_len)
    {
      SU_RB_LastError = SU_RB_ERR_MORE_DATA;
      return false;
    }
  }
  Handle = SU_RB_OpenKeys(Key,0);
  if(Handle == 0)
    return true; /* Returns true even if key not there, with the Default value */
  p = strrchr(Key,'\\');
  if(p == NULL)
  {
    RegCloseKey(Handle);
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    return false;
  }
  p++;
  Size = buf_len;
  if(RegQueryValueEx(Handle,p,NULL,&VType,NULL,NULL) == ERROR_SUCCESS) /* Only check for Type if no error */
  {
    if(VType != REG_BINARY)
    {
      RegCloseKey(Handle);
      SU_RB_LastError = SU_RB_ERR_WRONG_TYPE;
      return false;
    }
  }
  R = RegQueryValueEx(Handle,p,NULL,NULL,(LPBYTE)buf,&Size);
  RegCloseKey(Handle);
  if(R != ERROR_SUCCESS)
  {
    if(R == ERROR_MORE_DATA)
    {
      SU_RB_LastError = SU_RB_ERR_MORE_DATA;
      return false;
    }
  }
  return true; /* Returns true even if key not there, with the Default value */
}

SKYUTILS_API bool SU_RB_GetBinLength(const char Key[],int *length)
{
  HKEY Handle;
  char *p;
  DWORD VType;
  LONG R;

  Handle = SU_RB_OpenKeys(Key,0);
  if(Handle == 0)
  {
    SU_RB_LastError = SU_RB_ERR_NO_SUCH_KEY;
    return false;
  }
  p = strrchr(Key,'\\');
  if(p == NULL)
  {
    RegCloseKey(Handle);
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    return false;
  }
  p++;
  if(RegQueryValueEx(Handle,p,NULL,&VType,NULL,NULL) == ERROR_SUCCESS) /* Only check for Type if no error */
  {
    if(VType != REG_BINARY)
    {
      RegCloseKey(Handle);
      SU_RB_LastError = SU_RB_ERR_WRONG_TYPE;
      return false;
    }
  }
  R = RegQueryValueEx(Handle,p,NULL,NULL,NULL,(unsigned long *)length);
  RegCloseKey(Handle);
  if(R != ERROR_SUCCESS)
  {
    if(R == ERROR_MORE_DATA)
    {
      SU_RB_LastError = SU_RB_ERR_MORE_DATA;
      return false;
    }
  }
  return true;
}

SKYUTILS_API bool SU_RB_SetStrValue(const char Key[],const char Value[])
{
  HKEY Handle;
  char *p;
  LONG R;

  Handle = SU_RB_CreateKeys(Key);
  if(Handle == 0)
    return false;
  p = strrchr(Key,'\\');
  if(p == NULL)
  {
    RegCloseKey(Handle);
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    return false;
  }
  p++;
  R = RegSetValueEx(Handle,p,0,REG_SZ,(const BYTE *)Value,(DWORD)(strlen(Value)+1));
  RegCloseKey(Handle);
  if(R == ERROR_SUCCESS)
  {
    SU_RB_LastError = SU_RB_ERR_SUCCESS;
    return true;
  }
  else
  {
    SU_RB_LastError = SU_RB_ERR_ACCESS_DENIED;
    return false;
  }
}

SKYUTILS_API bool SU_RB_SetIntValue(const char Key[],int Value)
{
  HKEY Handle;
  char *p;
  LONG R;

  Handle = SU_RB_CreateKeys(Key);
  if(Handle == 0)
    return false;
  p = strrchr(Key,'\\');
  if(p == NULL)
  {
    RegCloseKey(Handle);
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    return false;
  }
  p++;
  R = RegSetValueEx(Handle,p,0,REG_DWORD,(BYTE *)&Value,sizeof(Value));
  RegCloseKey(Handle);
  if(R == ERROR_SUCCESS)
  {
    SU_RB_LastError = SU_RB_ERR_SUCCESS;
    return true;
  }
  else
  {
    SU_RB_LastError = SU_RB_ERR_ACCESS_DENIED;
    return false;
  }
}

SKYUTILS_API bool SU_RB_SetBinValue(const char Key[],const char *Value,int val_len)
{
  HKEY Handle;
  char *p;
  LONG R;

  Handle = SU_RB_CreateKeys(Key);
  if(Handle == 0)
    return false;
  p = strrchr(Key,'\\');
  if(p == NULL)
  {
    RegCloseKey(Handle);
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    return false;
  }
  p++;
  R = RegSetValueEx(Handle,p,0,REG_BINARY,(const BYTE *)Value,val_len);
  RegCloseKey(Handle);
  if(R == ERROR_SUCCESS)
  {
    SU_RB_LastError = SU_RB_ERR_SUCCESS;
    return true;
  }
  else
  {
    SU_RB_LastError = SU_RB_ERR_ACCESS_DENIED;
    return false;
  }
}

typedef LONG (WINAPI *REGDELETEKEYEXPROC)(HKEY,LPCTSTR,REGSAM,DWORD);
typedef DWORD (WINAPI *SHDELETEKEYPROC)(HKEY,LPCTSTR);
HMODULE _SU_RB_Mod_Advapi32 = NULL;
HMODULE _SU_RB_Mod_Shlwapi = NULL;
REGDELETEKEYEXPROC _SU_RB_rdkEx = NULL;
SHDELETEKEYPROC _SU_RB_shdk = NULL;

LONG _SU_RB_DelKey_Internal(HKEY Handle,char *p)
{
  if(_SU_RB_Mod_Advapi32 == NULL)
  {
    _SU_RB_Mod_Advapi32 = LoadLibrary("ADVAPI32\0");
    if(_SU_RB_Mod_Advapi32)
      _SU_RB_rdkEx = (REGDELETEKEYEXPROC)GetProcAddress(_SU_RB_Mod_Advapi32,"RegDeleteKeyExA\0");
  }

  if(_SU_RB_rdkEx)
    return _SU_RB_rdkEx(Handle,p,SU_RB_Access64Mode,0);
  else
  {
    if(_SU_RB_Mod_Shlwapi == NULL)
    {
      _SU_RB_Mod_Shlwapi = LoadLibrary("SHLWAPI\0");
      if(_SU_RB_Mod_Shlwapi)
        _SU_RB_shdk = (SHDELETEKEYPROC)GetProcAddress(_SU_RB_Mod_Shlwapi,"SHDeleteKeyA\0");
    }

    if(_SU_RB_shdk)
      return _SU_RB_shdk(Handle,p);
  }
  return ERROR_SUCCESS;
}

SKYUTILS_API bool SU_RB_DelKey(const char Key[])
{
  HKEY Handle;
  char *p;
  LONG R;

  Handle = SU_RB_OpenKeys(Key,KEY_SET_VALUE);
  if(Handle == 0)
    return false;
  p = strrchr(Key,'\\');
  if(p == NULL)
  {
    RegCloseKey(Handle);
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    return false;
  }
  p++;
  R = _SU_RB_DelKey_Internal(Handle,p);
  RegCloseKey(Handle);
  if(R == ERROR_SUCCESS)
  {
    SU_RB_LastError = SU_RB_ERR_SUCCESS;
    return true;
  }
  else
  {
    SU_RB_LastError = SU_RB_ERR_ACCESS_DENIED;
    return false;
  }
}

SKYUTILS_API bool SU_RB_DelValue(const char Key[])
{
  HKEY Handle;
  char *p;
  LONG R;

  Handle = SU_RB_OpenKeys(Key,KEY_SET_VALUE);
  if(Handle == 0)
    return false;
  p = strrchr(Key,'\\');
  if(p == NULL)
  {
    RegCloseKey(Handle);
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    return false;
  }
  p++;
  R = RegDeleteValue(Handle,p);
  RegCloseKey(Handle);
  if(R == ERROR_SUCCESS)
  {
    SU_RB_LastError = SU_RB_ERR_SUCCESS;
    return true;
  }
  else
  {
    SU_RB_LastError = SU_RB_ERR_ACCESS_DENIED;
    return false;
  }
}

SKYUTILS_API void SU_RB_CloseKey(HKEY Key)
{
  RegCloseKey(Key);
}

SKYUTILS_API bool SU_RB_EnumKey(HKEY Key,int Idx,char *Name,int name_len) /* True on Success. False when no more values available */
{
  LONG ret = RegEnumKey(Key,Idx,Name,name_len);

  return(ret == ERROR_SUCCESS);
}

SKYUTILS_API bool SU_RB_EnumStrValue(HKEY Key,int Idx,char *Name,int name_len,char *Value,int value_len) /* True on Success. False when no more values available */
{
  int len = name_len;
  int len2 = value_len;
  LONG ret = RegEnumValue(Key,Idx,Name,(ULONG *)&len,NULL,NULL,(LPBYTE)Value,(ULONG *)&len2);

  return(ret == ERROR_SUCCESS);
}

SKYUTILS_API bool SU_RB_EnumIntValue(HKEY Key,int Idx,char *Name,int name_len,int *Value) /* True on Success. False when no more values available */
{
  int len = name_len;
  int len2 = sizeof(Value);
  LONG ret = RegEnumValue(Key,Idx,Name,(ULONG *)&len,NULL,NULL,(unsigned char *)Value,(ULONG *)&len2);

  return(ret == ERROR_SUCCESS);
}

SKYUTILS_API bool SU_RB_OpenRegistry(const char RegistryPath[])
{
  SU_RB_LastError = SU_RB_ERR_SUCCESS;
  return true;
}

SKYUTILS_API bool SU_RB_CloseRegistry(void)
{
  SU_RB_LastError = SU_RB_ERR_SUCCESS;
  return true;
}

#else /* !_WIN32 */

#define SU_RB_TYPE_INT 1
#define SU_RB_TYPE_STR 2
#define SU_RB_TYPE_BUF 3

/* ********* */
#include <sys/file.h>

typedef struct
{
  char *Name;
  SU_PList Values; /* SU_RB_PValue */
  SU_PList Nodes; /* SU_RB_PNode */
} SU_RB_TNode, *SU_RB_PNode;

typedef struct
{
  char *Name;
  int Type;
  void *Value;
} SU_RB_TValue, *SU_RB_PValue;

/* ********* */
static SU_RB_PNode _SU_RB_RootNode = NULL; /* Root node */
static FILE *_SU_RB_RegFile = NULL;

/* ********* */

SU_RB_PNode _SU_RB_OpenNode(SU_RB_PNode Node,const char Name[])
{
  SU_PList Ptr;

  SU_RB_LastError = SU_RB_ERR_SUCCESS;
  if(Node == NULL)
    Node = _SU_RB_RootNode;

  if(Node == NULL)
  {
    SU_RB_LastError = SU_RB_ERR_REGISTRY_NOT_OPEN;
    return NULL;
  }
  
  Ptr = Node->Nodes;
  while(Ptr != NULL)
  {
    if(SU_strcasecmp(Name,((SU_RB_PNode)Ptr->Data)->Name))
      return (SU_RB_PNode)Ptr->Data;
    Ptr = Ptr->Next;
  }
  return NULL;
}

void _SU_RB_FreeValue(SU_RB_PValue Val)
{
  switch(Val->Type)
  {
    case SU_RB_TYPE_INT :
      break;
    case SU_RB_TYPE_STR :
      if(Val->Value)
        free(Val->Value); /* Free string */
      break;
  }
  if(Val->Name)
    free(Val->Name);
  free(Val);
}

void _SU_RB_FreeNode(SU_RB_PNode Node)
{
  SU_PList Ptr;
  
  Ptr = Node->Values;
  while(Ptr != NULL)
  {
    _SU_RB_FreeValue((SU_RB_PValue)Ptr->Data);
    Ptr = Ptr->Next;
  }
  SU_FreeList(Node->Values);

  Ptr = Node->Nodes;
  while(Ptr != NULL)
  {
    _SU_RB_FreeNode((SU_RB_PNode)Ptr->Data);
    Ptr = Ptr->Next;
  }
  SU_FreeList(Node->Nodes);
  
  if(Node->Name)
    free(Node->Name);
  free(Node);
}

bool _SU_RB_DeleteKey(SU_RB_PNode Node,const char Name[])
{
  SU_PList Ptr,Ptr2 = NULL;

  if(Node == NULL)
    return false;
  
  Ptr = Node->Nodes;
  while(Ptr != NULL)
  {
    if(SU_strcasecmp(Name,((SU_RB_PNode)Ptr->Data)->Name))
    {
      _SU_RB_FreeNode((SU_RB_PNode)Ptr->Data);
      Ptr = SU_DelElementHead(Ptr);
      if(Ptr2 == NULL) /* Head of the list */
        Node->Nodes = Ptr;
      else
        Ptr2->Next = Ptr;
      return true;
    }
    Ptr2 = Ptr;
    Ptr = Ptr->Next;
  }
  return false;
}

bool _SU_RB_DeleteValue(SU_RB_PNode Node,const char Name[])
{
  SU_PList Ptr,Ptr2 = NULL;

  if(Node == NULL)
    return false;
  
  Ptr = Node->Values;
  while(Ptr != NULL)
  {
    if(SU_strcasecmp(Name,((SU_RB_PValue)Ptr->Data)->Name))
    {
      _SU_RB_FreeValue((SU_RB_PValue)Ptr->Data);
      Ptr = SU_DelElementHead(Ptr);
      if(Ptr2 == NULL) /* Head of the list */
        Node->Values = Ptr;
      else
        Ptr2->Next = Ptr;
      return true;
    }
    Ptr2 = Ptr;
    Ptr = Ptr->Next;
  }
  return false;
}

SU_RB_PNode _SU_RB_CreateNode(SU_RB_PNode Node,const char Name[])
{
  SU_RB_PNode NewNode;

  SU_RB_LastError = SU_RB_ERR_SUCCESS;
  NewNode = _SU_RB_OpenNode(Node,Name); /* Try to open existing node */
  if(NewNode != NULL)
    return NewNode;

  if(Node == NULL)
    Node = _SU_RB_RootNode;
  
  if(Node == NULL)
  {
    SU_RB_LastError = SU_RB_ERR_REGISTRY_NOT_OPEN;
    return NULL;
  }
  
  NewNode = (SU_RB_PNode) malloc(sizeof(SU_RB_TNode));
  memset(NewNode,0,sizeof(SU_RB_TNode));

  NewNode->Name = SU_strdup(Name);
  Node->Nodes = SU_AddElementTail(Node->Nodes,NewNode);
  return NewNode;
}

SKYUTILS_API HKEY SU_RB_OpenKeys(const char Key[],int Access) /* Opened HKEY or 0 on error */
{
  SU_RB_PNode Node = NULL;
  char *tmp,*p,*q;

  if(Key == NULL)
  {
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    return 0;
  }
  tmp = SU_strdup(Key);

  p = SU_strparse(tmp,'\\');
  if(p == NULL)
  {
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    free(tmp);
    return 0;
  }
  q = SU_strparse(NULL,'\\');
  while(q != NULL)
  {
    Node = _SU_RB_OpenNode(Node,p);
    if(Node == NULL)
    {
      free(tmp);
      if(SU_RB_LastError == SU_RB_ERR_SUCCESS)
        SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
      return 0;
    }
    p = q;
    q = SU_strparse(NULL,'\\');
  }
  free(tmp);
  SU_RB_LastError = SU_RB_ERR_SUCCESS;
  return (HKEY) Node;
}

SKYUTILS_API HKEY SU_RB_CreateKeys(const char Key[]) /* Created HKEY or 0 on error */
{
  SU_RB_PNode Node = NULL;
  char *tmp,*p,*q;

  if(Key == NULL)
  {
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    return 0;
  }
  tmp = SU_strdup(Key);

  p = SU_strparse(tmp,'\\');
  if(p == NULL)
  {
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    free(tmp);
    return 0;
  }
  q = SU_strparse(NULL,'\\');
  while(q != NULL)
  {
    Node = _SU_RB_CreateNode(Node,p);
    if(Node == NULL)
    {
      free(tmp);
      if(SU_RB_LastError == SU_RB_ERR_SUCCESS)
        SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
      return 0;
    }
    p = q;
    q = SU_strparse(NULL,'\\');
  }
  free(tmp);
  SU_RB_LastError = SU_RB_ERR_SUCCESS;
  return (HKEY) Node;
}

SKYUTILS_API bool SU_RB_EnumKey(HKEY Key,int Idx,char *Name,int name_len) /* True on Success. False when no more values available */
{
  fprintf(stderr,"SU_RB_EnumKey : TO DO !! Abort\n");
  abort();
}

SKYUTILS_API bool SU_RB_EnumStrValue(HKEY Key,int Idx,char *Name,int name_len,char *Value,int value_len) /* True on Success. False when no more values available */
{
  SU_RB_PNode Node = (SU_RB_PNode)Key;
  SU_RB_PValue Val;

  if((Node == NULL) || (Node->Values == NULL))
    return false;

  Val = (SU_RB_PValue)SU_GetElementPos(Node->Values,Idx);
  if(Val == NULL)
    return false;
  if(Val->Type != SU_RB_TYPE_STR)
    return false;
  SU_strcpy(Name,Val->Name,name_len);
  SU_strcpy(Value,(char *)Val->Value,value_len);
  return true;
}

SKYUTILS_API bool SU_RB_EnumIntValue(HKEY Key,int Idx,char *Name,int name_len,int *Value) /* True on Success. False when no more values available */
{
  SU_RB_PNode Node = (SU_RB_PNode)Key;
  SU_RB_PValue Val;

  if((Node == NULL) || (Node->Values == NULL))
    return false;

  Val = (SU_RB_PValue)SU_GetElementPos(Node->Values,Idx);
  if(Val == NULL)
    return false;
  if(Val->Type != SU_RB_TYPE_INT)
    return false;
  SU_strcpy(Name,Val->Name,name_len);
  *Value = (int)Val->Value;
  return true;
}

SKYUTILS_API void SU_RB_CloseKey(HKEY Key)
{
  return;
}

bool _SU_RB_ReadValue(SU_RB_PValue Value)
{
  int nb;
  char *str;

  /* Name */
  if(fread(&nb,1,sizeof(nb),_SU_RB_RegFile) != sizeof(nb))
  {
    SU_RB_LastError = SU_RB_ERR_PREMATURE_EOF;
    return false;
  }
  Value->Name = (char *) malloc(nb+1);
  Value->Name[nb] = 0;
  if(fread(Value->Name,1,nb,_SU_RB_RegFile) != nb)
  {
    SU_RB_LastError = SU_RB_ERR_PREMATURE_EOF;
    return false;
  }

  /* Type */
  if(fread(&Value->Type,1,sizeof(Value->Type),_SU_RB_RegFile) != sizeof(Value->Type))
  {
    SU_RB_LastError = SU_RB_ERR_PREMATURE_EOF;
    return false;
  }

  /* Value */
  switch(Value->Type)
  {
    case SU_RB_TYPE_INT :
      if(fread(&nb,1,sizeof(nb),_SU_RB_RegFile) != sizeof(nb))
      {
        SU_RB_LastError = SU_RB_ERR_PREMATURE_EOF;
        return false;
      }
      Value->Value = (void *)nb;
      break;
    case SU_RB_TYPE_STR :
      if(fread(&nb,1,sizeof(nb),_SU_RB_RegFile) != sizeof(nb))
      {
        SU_RB_LastError = SU_RB_ERR_PREMATURE_EOF;
        return false;
      }
      str = (char *) malloc(nb+1);
      str[nb] = 0;
      Value->Value = str;
      if(fread(str,1,nb,_SU_RB_RegFile) != nb)
      {
        SU_RB_LastError = SU_RB_ERR_PREMATURE_EOF;
        return false;
      }
      break;
    default :
      SU_RB_LastError = SU_RB_ERR_INVALID_TYPE;
      return false;
  }
  
  return true;
}

bool _SU_RB_ReadNode(SU_RB_PNode Node)
{
  int nb,i;
  bool ret = true;
  SU_RB_PNode NewNode;
  SU_RB_PValue NewValue;
  
  /* Name */
  if(fread(&nb,1,sizeof(nb),_SU_RB_RegFile) != sizeof(nb))
  {
    SU_RB_LastError = SU_RB_ERR_PREMATURE_EOF;
    return false;
  }
  Node->Name = (char *) malloc(nb+1);
  Node->Name[nb] = 0;
  if(fread(Node->Name,1,nb,_SU_RB_RegFile) != nb)
  {
    SU_RB_LastError = SU_RB_ERR_PREMATURE_EOF;
    return false;
  }
  
  /* Values */
  if(fread(&nb,1,sizeof(nb),_SU_RB_RegFile) != sizeof(nb))
  {
    SU_RB_LastError = SU_RB_ERR_PREMATURE_EOF;
    return false;
  }
  Node->Values = NULL;
  for(i=0;i<nb;i++)
  {
    NewValue = (SU_RB_PValue) malloc(sizeof(SU_RB_TValue));
    if(_SU_RB_ReadValue(NewValue) == false)
      ret = false;
    Node->Values = SU_AddElementTail(Node->Values,NewValue);
  }
  
  /* Nodes */
  if(fread(&nb,1,sizeof(nb),_SU_RB_RegFile) != sizeof(nb))
  {
    SU_RB_LastError = SU_RB_ERR_PREMATURE_EOF;
    return false;
  }
  Node->Nodes = NULL;
  for(i=0;i<nb;i++)
  {
    NewNode = (SU_RB_PNode) malloc(sizeof(SU_RB_TNode));
    memset(NewNode,0,sizeof(SU_RB_TNode));
    if(_SU_RB_ReadNode(NewNode) == false)
      ret = false;
    Node->Nodes = SU_AddElementTail(Node->Nodes,NewNode);
  }
  
  return ret;
}

SKYUTILS_API bool SU_RB_OpenRegistry(const char RegistryPath[])
{
  /* If a registry is opened, close it */
  if(_SU_RB_RegFile != NULL)
  {
    if(SU_RB_CloseRegistry() == false)
      return false;
  }

  /* Create root node */  
  _SU_RB_RootNode = (SU_RB_PNode) malloc(sizeof(SU_RB_TNode));
  memset(_SU_RB_RootNode,0,sizeof(SU_RB_TNode));
  
  /* Open the registry, and lock it */
  _SU_RB_RegFile = fopen(RegistryPath,"r+b");
  if(_SU_RB_RegFile == NULL) /* Cannot open ? Create registry */
  {
    _SU_RB_RegFile = fopen(RegistryPath,"w+b");
    if(_SU_RB_RegFile == NULL)
    {
      SU_RB_LastError = SU_RB_ERR_INVALID_PATH;
      return false;
    }
    if(flock(fileno(_SU_RB_RegFile),LOCK_EX | LOCK_NB) != 0)
    {
      fclose(_SU_RB_RegFile);
      SU_RB_LastError = SU_RB_ERR_LOCK_FAILED;
      return false;
    }
    _SU_RB_RootNode->Name = SU_strdup("Root");
    SU_RB_LastError = SU_RB_ERR_SUCCESS;
    return true;
  }
  if(flock(fileno(_SU_RB_RegFile),LOCK_EX | LOCK_NB) != 0)
  {
    fclose(_SU_RB_RegFile);
    SU_RB_LastError = SU_RB_ERR_LOCK_FAILED;
    return false;
  }
  
  /* Load registry */
  if(_SU_RB_ReadNode(_SU_RB_RootNode) == false) /* Error reading registry */
  {
    SU_RB_LastError = SU_RB_ERR_CORRUPTED;
    return false;
  }
  
  SU_RB_LastError = SU_RB_ERR_SUCCESS;
  return true;
}

bool _SU_RB_WriteValue(SU_RB_PValue Value)
{
  int nb;

  /* Name */
  nb = strlen(Value->Name);
  if(fwrite(&nb,1,sizeof(nb),_SU_RB_RegFile) != sizeof(nb))
  {
    SU_RB_LastError = SU_RB_ERR_WRITE_ERROR;
    return false;
  }
  if(fwrite(Value->Name,1,nb,_SU_RB_RegFile) != nb)
  {
    SU_RB_LastError = SU_RB_ERR_WRITE_ERROR;
    return false;
  }

  /* Type */
  if(fwrite(&Value->Type,1,sizeof(Value->Type),_SU_RB_RegFile) != sizeof(Value->Type))
  {
    SU_RB_LastError = SU_RB_ERR_WRITE_ERROR;
    return false;
  }

  /* Value */
  switch(Value->Type)
  {
    case SU_RB_TYPE_INT :
      nb = (int)(Value->Value);
      if(fwrite(&nb,1,sizeof(nb),_SU_RB_RegFile) != sizeof(nb))
      {
        SU_RB_LastError = SU_RB_ERR_WRITE_ERROR;
        return false;
      }
      break;
    case SU_RB_TYPE_STR :
      nb = strlen((char *)(Value->Value));
      if(fwrite(&nb,1,sizeof(nb),_SU_RB_RegFile) != sizeof(nb))
      {
        SU_RB_LastError = SU_RB_ERR_WRITE_ERROR;
        return false;
      }
      if(fwrite((char *)(Value->Value),1,nb,_SU_RB_RegFile) != nb)
      {
        SU_RB_LastError = SU_RB_ERR_WRITE_ERROR;
        return false;
      }
      if(Value->Value)
        free(Value->Value); /* Free string */
      break;
    default :
      SU_RB_LastError = SU_RB_ERR_INVALID_TYPE;
      return false;
  }
  
  if(Value->Name)
    free(Value->Name);
  free(Value);
  return true;
}

bool _SU_RB_WriteNode(SU_RB_PNode Node)
{
  int nb;
  SU_PList Ptr;
  bool ret = true;
  
  /* Name */
  nb = strlen(Node->Name);
  if(fwrite(&nb,1,sizeof(nb),_SU_RB_RegFile) != sizeof(nb))
  {
    SU_RB_LastError = SU_RB_ERR_WRITE_ERROR;
    return false;
  }
  if(fwrite(Node->Name,1,nb,_SU_RB_RegFile) != nb)
  {
    SU_RB_LastError = SU_RB_ERR_WRITE_ERROR;
    return false;
  }
  
  /* Values */
  nb = SU_ListCount(Node->Values);
  if(fwrite(&nb,1,sizeof(nb),_SU_RB_RegFile) != sizeof(nb))
  {
    SU_RB_LastError = SU_RB_ERR_WRITE_ERROR;
    return false;
  }
  Ptr = Node->Values;
  while(Ptr != NULL)
  {
    if(_SU_RB_WriteValue((SU_RB_PValue)Ptr->Data) == false)
      ret = false;
    Ptr = Ptr->Next;
  }
  SU_FreeList(Node->Values);
  
  /* Nodes */
  nb = SU_ListCount(Node->Nodes);
  if(fwrite(&nb,1,sizeof(nb),_SU_RB_RegFile) != sizeof(nb))
  {
    SU_RB_LastError = SU_RB_ERR_WRITE_ERROR;
    return false;
  }
  Ptr = Node->Nodes;
  while(Ptr != NULL)
  {
    if(_SU_RB_WriteNode((SU_RB_PNode)Ptr->Data) == false)
      ret = false;
    Ptr = Ptr->Next;
  }
  SU_FreeList(Node->Nodes);
  
  if(Node->Name)
    free(Node->Name);
  free(Node);
  
  return ret;
}

SKYUTILS_API bool SU_RB_CloseRegistry(void)
{
  if(_SU_RB_RegFile == NULL)
  {
    SU_RB_LastError = SU_RB_ERR_SUCCESS;
    return true;
  }

  /* Rewind file pointer */
  rewind(_SU_RB_RegFile);

  /* Save registry */
  _SU_RB_WriteNode(_SU_RB_RootNode);
  _SU_RB_RootNode = NULL;
  
  /* Unlock, and close file */
  flock(fileno(_SU_RB_RegFile),LOCK_UN);
  fclose(_SU_RB_RegFile);
  
  _SU_RB_RegFile = NULL;
  SU_RB_LastError = SU_RB_ERR_SUCCESS;
  return true;
}

SU_RB_PValue _SU_RB_ReadStrValue(SU_RB_PNode Node,const char Key[],char *buf,int buf_len)
{
  SU_PList Ptr;
  SU_RB_PValue Val;

  if(Node == NULL)
    return NULL;
  
  Ptr = Node->Values;
  while(Ptr != NULL)
  {
    Val = (SU_RB_PValue)Ptr->Data;
    if(SU_strcasecmp(Key,Val->Name))
    {
      if(Val->Type != SU_RB_TYPE_STR)
      {
        SU_RB_LastError = SU_RB_ERR_WRONG_TYPE;
        return NULL;
      }
      if(buf != NULL)
        SU_strcpy(buf,(char *)Val->Value,buf_len);
      return Val;
    }
    Ptr = Ptr->Next;
  }
  return NULL;
}

SU_RB_PValue _SU_RB_ReadIntValue(SU_RB_PNode Node,const char Key[],int *val)
{
  SU_PList Ptr;
  SU_RB_PValue Val;

  if(Node == NULL)
    return NULL;
  
  Ptr = Node->Values;
  while(Ptr != NULL)
  {
    Val = (SU_RB_PValue)Ptr->Data;
    if(SU_strcasecmp(Key,Val->Name))
    {
      if(Val->Type != SU_RB_TYPE_INT)
      {
        SU_RB_LastError = SU_RB_ERR_WRONG_TYPE;
        return NULL;
      }
      if(val != NULL)
        *val = (int)Val->Value;
      return Val;
    }
    Ptr = Ptr->Next;
  }
  return NULL;
}

bool _SU_RB_SetStrValue(SU_RB_PNode Node,const char Key[],const char Value[])
{
  SU_RB_PValue Val = _SU_RB_ReadStrValue(Node,Key,NULL,0); /* Try to read existing Value */
  
  if(Val == NULL)
  {
    Val = (SU_RB_PValue) malloc(sizeof(SU_RB_TValue));
    memset(Val,0,sizeof(SU_RB_TValue));
    
    Val->Name = SU_strdup(Key);
    Node->Values = SU_AddElementTail(Node->Values,Val);
  }
  if((Val->Type == SU_RB_TYPE_STR) && (Val->Value != NULL))
  {
    free(Val->Value);
    Val->Value = NULL;
  }
  Val->Type = SU_RB_TYPE_STR;
  if(Value != NULL)
    Val->Value = (void *)SU_strdup(Value);
  return true;
}

bool _SU_RB_SetIntValue(SU_RB_PNode Node,const char Key[],const int Value)
{
  SU_RB_PValue Val = _SU_RB_ReadStrValue(Node,Key,NULL,0); /* Try to read existing Value */
  
  if(Val == NULL)
  {
    Val = (SU_RB_PValue) malloc(sizeof(SU_RB_TValue));
    memset(Val,0,sizeof(SU_RB_TValue));
    
    Val->Name = SU_strdup(Key);
    Node->Values = SU_AddElementTail(Node->Values,Val);
  }
  if((Val->Type == SU_RB_TYPE_STR) && (Val->Value != NULL))
  {
    free(Val->Value);
    Val->Value = NULL;
  }
  Val->Type = SU_RB_TYPE_INT;
  Val->Value = (void *)Value;
  return true;
}

SKYUTILS_API bool SU_RB_GetStrValue(const char Key[],char *buf,int buf_len,const char Default[])
{
  SU_RB_PNode Handle;
  char *p;

  SU_strcpy(buf,Default,buf_len);
  Handle = SU_RB_OpenKeys(Key,0);
  if(Handle == NULL)
    return true; /* Returns true even if key not there, with the Default value */
  p = strrchr(Key,'\\');
  if(p == NULL)
  {
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    return false;
  }
  p++;
  SU_RB_LastError = SU_RB_ERR_SUCCESS;
  if(_SU_RB_ReadStrValue(Handle,p,buf,buf_len) == NULL)
  {
    if(SU_RB_LastError == SU_RB_ERR_WRONG_TYPE)
      return false;
  }
  return true; /* Returns true even if key not there, with the Default value */
}

SKYUTILS_API bool SU_RB_GetStrLength(const char Key[],int *length)
{
  fprintf(stderr,"SU_RB_GetStrLength : TO DO !! Abort\n");
  abort();
}

SKYUTILS_API bool SU_RB_GetIntValue(const char Key[],int *Value,int Default)
{
  SU_RB_PNode Handle;
  char *p;
  int val;

  *Value = Default;
  Handle = SU_RB_OpenKeys(Key,0);
  if(Handle == NULL)
    return true; /* Returns true even if key not there, with the Default value */
  p = strrchr(Key,'\\');
  if(p == NULL)
  {
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    return Default;
  }
  p++;
  SU_RB_LastError = SU_RB_ERR_SUCCESS;
  if(_SU_RB_ReadIntValue(Handle,p,&val) == NULL)
  {
    if(SU_RB_LastError == SU_RB_ERR_WRONG_TYPE)
      return false;
  }
  else
    *Value = val;
  return true; /* Returns true even if key not there, with the Default value */
}

SKYUTILS_API bool SU_RB_GetBinValue(const char Key[],char *buf,int buf_len,char *Default,int def_len)
{
  fprintf(stderr,"SU_RB_GetBinValue : TO DO !! Abort\n");
  abort();
}

SKYUTILS_API bool SU_RB_GetBinLength(const char Key[],int *length)
{
  fprintf(stderr,"SU_RB_GetBinLength : TO DO !! Abort\n");
  abort();
}

SKYUTILS_API bool SU_RB_SetStrValue(const char Key[],const char Value[])
{
  SU_RB_PNode Handle;
  char *p;

  Handle = SU_RB_CreateKeys(Key);
  if(Handle == NULL)
    return false;
  p = strrchr(Key,'\\');
  if(p == NULL)
  {
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    return false;
  }
  p++;
  if(_SU_RB_SetStrValue(Handle,p,Value))
  {
    SU_RB_LastError = SU_RB_ERR_SUCCESS;
    return true;
  }
  else
  {
    SU_RB_LastError = SU_RB_ERR_ACCESS_DENIED;
    return false;
  }
}

SKYUTILS_API bool SU_RB_SetIntValue(const char Key[],int Value)
{
  SU_RB_PNode Handle;
  char *p;

  Handle = SU_RB_CreateKeys(Key);
  if(Handle == NULL)
    return false;
  p = strrchr(Key,'\\');
  if(p == NULL)
  {
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    return false;
  }
  p++;
  if(_SU_RB_SetIntValue(Handle,p,Value))
  {
    SU_RB_LastError = SU_RB_ERR_SUCCESS;
    return true;
  }
  else
  {
    SU_RB_LastError = SU_RB_ERR_ACCESS_DENIED;
    return false;
  }
}

SKYUTILS_API bool SU_RB_SetBinValue(const char Key[],const char *Value,int val_len)
{
  fprintf(stderr,"SU_RB_SetBinValue : TO DO !! Abort\n");
  abort();
}

SKYUTILS_API bool SU_RB_DelKey(const char Key[])
{
  SU_RB_PNode Handle;
  char *p;
  char *key = SU_strdup(Key);
  int len;

  len = strlen(key);
  if(key[len-1] == '\\') /* Remove trailing '\' */
    key[len-1] = 0;

  Handle = SU_RB_OpenKeys(key,0);
  if(Handle == NULL)
  {
    free(key);
    return false;
  }
  p = strrchr(key,'\\');
  if(p == NULL)
  {
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    free(key);
    return false;
  }
  p++;
  if(_SU_RB_DeleteKey(Handle,p))
  {
    SU_RB_LastError = SU_RB_ERR_SUCCESS;
    free(key);
    return true;
  }
  else
  {
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    free(key);
    return false;
  }
}

SKYUTILS_API bool SU_RB_DelValue(const char Key[])
{
  SU_RB_PNode Handle;
  char *p;

  Handle = SU_RB_OpenKeys(Key,0);
  if(Handle == NULL)
    return false;
  p = strrchr(Key,'\\');
  if(p == NULL)
  {
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    return false;
  }
  p++;
  if(_SU_RB_DeleteValue(Handle,p))
  {
    SU_RB_LastError = SU_RB_ERR_SUCCESS;
    return true;
  }
  else
  {
    SU_RB_LastError = SU_RB_ERR_INVALID_KEY;
    return false;
  }
}

#endif /* _WIN32 */

SKYUTILS_API int SU_RB_GetLastError(void)
{
  return SU_RB_LastError;
}

SKYUTILS_API void SU_RB_SetRegistry64Mode(int Mode)
{
  switch(Mode)
  {
    case SU_RB_MODE_NORMAL :
      SU_RB_Access64Mode = 0;
      break;
    case SU_RB_MODE_FORCE_WOW64_KEY :
      if(SU_IsWow64())
        SU_RB_Access64Mode = KEY_WOW64_64KEY;
      break;
  }
}

SKYUTILS_API int SU_RB_GetRegistry64Mode(void)
{
  int ret = SU_RB_MODE_NORMAL;

  switch(SU_RB_Access64Mode)
  {
    case KEY_WOW64_64KEY :
      ret = SU_RB_MODE_FORCE_WOW64_KEY;
      break;
  }
  return ret;
}
