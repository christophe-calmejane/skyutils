/****************************************************************/
/* Utils unit                                                   */
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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#ifdef _WIN32
#include <sys/timeb.h>
#include <time.h>
#ifdef __BORLANDC__
#define _ftime ftime
#define _timeb timeb
#endif /* __BORLANDC__ */
#endif /* _WIN32 */
#ifdef __GNUC__
#include <sys/time.h>
#include <time.h>
#endif /* __GNUC__ */

#ifdef _WIN32
#pragma warning( disable: 4100 4152)
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

extern char *SW_UserHeader;
char *SU_DebugAppName = NULL;

#ifdef DEBUG
int SU_DebugLevel = 9;
#else /* !DEBUG */
int SU_DebugLevel = 0;
#endif /* DEBUG */

SKYUTILS_API FILE *SU_OpenLogFile(const char LogName[])
{
  FILE *fp;

  fp = fopen(LogName,"at");
	return fp;
}

SKYUTILS_API void SU_CloseLogFile(FILE *fp)
{
  if(fp != NULL)
    fclose(fp);
}

SKYUTILS_API void SU_WriteToLogFile(FILE *fp,const char Text[])
{
  struct tm *TM;
  time_t Tim;

  if(fp != NULL)
  {
    Tim = time(NULL);
    TM = localtime(&Tim);
    fprintf(fp,"[%.4d/%.2d/%.2d-%.2d:%.2d:%.2d] %s\n",TM->tm_year+1900,TM->tm_mon+1,TM->tm_mday,TM->tm_hour,TM->tm_min,TM->tm_sec,Text);
    fflush(fp);
  }
}

/* Skip the username and password, if present here.  The function
   should be called *not* with the complete URL, but with the part
   right after the protocol.

   If no username and password are found, return 0.  */
static int skip_uname (const char *url)
{
  const char *p;
  for (p = url; *p && *p != '/'; p++)
    if (*p == '@')
      break;
  /* If a `@' was found before the first occurrence of `/', skip
     it.  */
  if (*p == '@')
    return (int)(p - url + 1);
  else
    return 0;
}

static void parse_uname (const char *url, char *user, char *passwd)
{
  const char *p, *col;

  /* Is there an `@' character?  */
  for (p = url; *p && *p != '/'; p++)
    if (*p == '@')
      break;
  /* If not, return.  */
  if (*p != '@')
    return;

  /* Else find the username and password.  */
  for (p = col = url; *p != '@'; p++)
    {
      if (*p == ':')
        {
          memcpy (user, url, p - url);
          user[p - url] = '\0';
          col = p + 1;
        }
    }
  memcpy (passwd, col, p - col);
  passwd[p - col] = '\0';
}

/* Checks the http_proxy env var */
SKYUTILS_API void SU_CheckProxyEnv(void)
{
  char *proxy_env,*tok;
  char  proxy_server_name[256];
  char  proxy_server_user[256];
  char  proxy_server_password[256];
  int   proxy_server_port=8080;

  proxy_env = getenv("http_proxy");
  if((proxy_env != NULL) && (strlen(proxy_env)>0))
  {
    char *proxy_save;
    char *proxy_val;

    memset(proxy_server_name,0,256);
    memset(proxy_server_user,0,256);
    memset(proxy_server_password,0,256);
    /*
     * Proxy URL is in the form :  [http://][user:password@]server:port[/]
     * Skip the "http://" if it's present
     */
    if(!strncasecmp(proxy_env,"http://",7)) proxy_env+=7;
    proxy_save = SU_strdup(proxy_env);

    /*
     * Allow a username and password to be specified (i.e. just skip
     * them for now).
     */
    proxy_val = proxy_env+skip_uname (proxy_env);
    tok=strtok(proxy_val,":");
    if(tok) strncpy(proxy_server_name,tok,256);
    tok=strtok(NULL,"/");
    if(tok) proxy_server_port=atoi(tok);

    /* Parse the username and password (if existing).  */
    parse_uname (proxy_save, proxy_server_user, proxy_server_password);

    SU_SetProxy(proxy_server_name,proxy_server_port,proxy_server_user,proxy_server_password);
    free(proxy_save);
  }
}

/* Remove arguments for skyutils, and returns number of remaining arguments for smssend */
SKYUTILS_API int SU_GetSkyutilsParams(int argc,char *argv[])
{
  int i = 1,nb;
  char *pos;
  int Port = 0;
  int Timeout = 0;
  int Lv = 0;
  char *ProxyName = NULL;
  char *UserName = NULL;
  char *Password = NULL;
  bool proxy = false;

  nb = argc;
  while(i < argc)
  {
    if(strcmp(argv[i],"--") == 0) /* SkyUtils arguments */
    {
      nb = i;
      i++;
      while(i < argc)
      {
        if(strcmp(argv[i],"--") == 0) /* No more SkyUtils arguments */
          break;
        else if(strncmp(argv[i],"-d",2) == 0) /* Debug level */
        {
          Lv = atoi(argv[i] + 2);
          SU_SetDebugLevel(argv[0],Lv);
        }
        else if(strncmp(argv[i],"-t",2) == 0) /* Socket timeout */
        {
          Timeout = atoi(argv[i] + 2);
          SU_SetSocketTimeout(Timeout);
        }
        else if(strncmp(argv[i],"-h",2) == 0) /* User's header file */
        {
          SW_UserHeader = SU_LoadUserHeaderFile(argv[i] + 2);
        }
        else if(strncmp(argv[i],"-p",2) == 0) /* Proxy name:port */
        {
          pos = strchr(argv[i],':');
          if(pos == NULL)
            printf("SkyUtils_SU_GetSkyutilsParams Warning : Error parsing proxy argument for skyutils, disabling proxy\n");
          else
          {
            Port = atoi(pos+1);
            pos[0] = 0;
            ProxyName = argv[i] + 2;
          }
        }
        else if(strncmp(argv[i],"-u",2) == 0) /* Proxy user:pass */
        {
          pos = strchr(argv[i],':');
          if(pos == NULL)
            printf("SkyUtils_SU_GetSkyutilsParams Warning : Error parsing proxy username argument for skyutils, disabling proxy\n");
          else
          {
            Password = pos + 1;
            pos[0] = 0;
            UserName = argv[i] + 2;
          }
        }
        i++;
      }
      break;
    }
    i++;
  }
  if((ProxyName == NULL) && (UserName != NULL))
  {
    printf("SkyUtils_SU_GetSkyutilsParams Warning : Username for proxy specified, but no proxy given, disabling proxy\n");
  }
  else if(ProxyName != NULL)
  {
    SU_SetProxy(ProxyName,Port,UserName,Password);
    proxy = true;
  }
  if(!proxy)
    SU_CheckProxyEnv();
  return nb;
}

SKYUTILS_API char *SU_LoadUserHeaderFile(const char FName[])
{
  FILE *fp;
  char *buf;
  char S[1024];
  int len;

  fp = fopen(FName,"rt");
  if(fp == NULL)
  {
    printf("SkyUtils_SU_LoadUserHeaderFile Warning : Cannot load user's header file %s\n",FName);
    return NULL;
  }
  buf = NULL;
  len = 1; /* For the \0 */
  while(SU_ReadLine(fp,S,sizeof(S)))
  {
    if(S[0] == 0)
      continue;
    len += (int)(strlen(S) + 2); /* +2 for \n */
    if(buf == NULL)
    {
      buf = (char *) malloc(len);
      SU_strcpy(buf,S,len);
    }
    else
    {
      buf = (char *) realloc(buf,len);
      SU_strcat(buf,S,len);
    }
    SU_strcat(buf,"\x0D" "\x0A",len);
  }
  fclose(fp);
  return buf;
}

SKYUTILS_API char *SU_GetOptionsString(void)
{
  return "-pproxy:port -uusername:password -tTimeout -dDebugLevel -hHeaderFile";
}

#ifdef __unix__
SKYUTILS_API bool SU_Daemonize(void)
{
  pid_t pid, sid;
  int fd;

  /* Fork to let the parent exit */
  pid = fork();

  if(pid == -1 )
  {
    perror("Daemonize error : Couldn't fork");
    return false;
  }

  /* Now father exits */
  if(pid != 0)
    exit(0);

  /* Son is trying to become a session and group leader, by running in a new session */
  sid = setsid();

  if(sid == -1)
  {
    perror("Daemonize error : Couldn't setsid");
    return false;
  }

  /* We are now a group leader and a session leader, with no controlling terminal.
     We gonna fork again, and the parent will exit.
     So the son, as a non-session group leader won't be able to acquire a controlling terminal anymore. */
  pid = fork ();

  if(pid == -1)
  {
    perror("Daemonize error : Couldn't fork");
    return false;
  }

  if(pid != 0 )
    exit(0);

  /* Son will now change it's working dir to /, in order to ensure that our daemon doesn't keep any
     directory in use (it would allow admin to unmount filesystem) */
  if(chdir("/") == -1)
  {
    perror("Daemonize error : Couldn't chdir('/')");
    return false;
  }

  /* Set the umask to 0 in order to be sure we create the files with right permissions */
  umask(0);

  /* Now close fd 0, 1 and 2 and opens 0 as /dev/null */
  fd=0;
  close(0);
  fd = open("/dev/null",O_RDONLY);
  if(fd == -1)
  {
    perror("Daemonize error : Couldn't open /dev/null");
    return false;
  }
  else if(fd != 0 )
  {
    perror("Daemonize warning : Trying to open /dev/null for stdin but returned file descriptor is not 0.");
    close(fd);
  }

  fd=1;
  close(1);
  fd = open("/dev/null",O_WRONLY);
  if(fd == -1)
  {
    perror("Daemonize error : Couldn't open /dev/null");
    return false;
  }
  else if(fd != 1 )
  {
    perror("Daemonize warning : Trying to open /dev/null for stdout but returned file descriptor is not 1.");
    close(fd);
  }

  fd=2;
  close(2);
  fd = open("/dev/null",O_WRONLY);
  if(fd == -1)
  {
    perror("Daemonize error : Couldn't open /dev/null");
    return false;
  }
  else if(fd != 2 )
  {
    perror("Daemonize warning : Trying to open /dev/null for stdout but returned file descriptor is not 2.");
    close(fd);
  }

  /* We now are a daemon stdin, stderr are closed */
  return true;
}

SKYUTILS_API bool SU_SetUserGroup(const char User[],const char Group[])
{
  struct passwd *pw;
  struct group *gr;

  if(Group != NULL)
  {
    gr = getgrnam(Group);
    if(gr == NULL)
    {
      fprintf(stderr,"SkyUtils_SetUserGroup Warning : Group %s not found in /etc/passwd\n",Group);
      return false;
    }
    if(setgid(gr->gr_gid) != 0)
    {
      fprintf(stderr,"SkyUtils_SetUserGroup Warning : Cannot setgid to group %s : %s\n",Group,strerror(errno));
      return false;
    }
  }

  if(User != NULL)
  {
    pw = getpwnam(User);
    if(pw == NULL)
    {
      fprintf(stderr,"SkyUtils_SetUserGroup Warning : User %s not found in /etc/passwd\n",User);
      return false;
    }
    if(setuid(pw->pw_uid) != 0)
    {
      fprintf(stderr,"SkyUtils_SetUserGroup Warning : Cannot setuid to user %s : %s\n",User,strerror(errno));
      return false;
    }
  }

  return true;
}
#endif /* __unix__ */

SKYUTILS_API void SU_PrintSyslog(int Level,char *Txt, ...)
{
  va_list argptr;
  char Str[4096];

  va_start(argptr,Txt);
#ifdef _WIN32
  _vsnprintf(Str,sizeof(Str),Txt,argptr);
#else /* !_WIN32 */
  vsnprintf(Str,sizeof(Str),Txt,argptr);
#endif /* _WIN32 */
  va_end(argptr);
  SU_SYSLOG_FN(Level,Str);
}

#undef SU_PrintDebug
SKYUTILS_API void SU_PrintDebug(int Level,char *Txt, ...)
{
  va_list argptr;
  char Str[4096];

  if(Level <= SU_DebugLevel)
  {
    va_start(argptr,Txt);
#ifdef _WIN32
    _vsnprintf(Str,sizeof(Str),Txt,argptr);
#else /* !_WIN32 */
    vsnprintf(Str,sizeof(Str),Txt,argptr);
#endif /* _WIN32 */
    va_end(argptr);
    printf("%s(%d) : %s",SU_DebugAppName,Level,Str);
  }
}

SKYUTILS_API void SU_SetDebugLevel(const char AppName[],const int Level)
{
  if(SU_DebugAppName != NULL)
    free(SU_DebugAppName);
  if(AppName == NULL)
    SU_DebugAppName = "SkyUtils";
  else
    SU_DebugAppName = SU_strdup(AppName);
  SU_DebugLevel = Level;
}

SKYUTILS_API int SU_GetDebugLevel(void)
{
  return SU_DebugLevel;
}

#ifdef SU_USE_DL
SKYUTILS_API void *SU_DL_GetSym(SU_DL_HANDLE handle, const char Name[])
{
  char buf[500];
  void *func;

  func = SU_DL_SYM(handle,Name);
  if(func == NULL)
  {
    SU_snprintf(buf,sizeof(buf),"_%s",Name);
    func = SU_DL_SYM(handle,buf);
  }
  return func;
}
#endif /* SU_USE_DL */


/* PRECISE COUNTER FUNCTIONS */
#if defined(_MSC_VER) || defined(__BORLANDC__)
#ifdef _WIN64
SKYUTILS_API SU_CPUSPEED SU_GetCPUSpeed(void)
{
  return 0;
}

SKYUTILS_API void SU_GetTicks(SU_TICKS *tick)
{
  *tick = GetTickCount();
}

SKYUTILS_API SU_u32 SU_ElapsedTime(SU_TICKS t1,SU_TICKS t2,SU_CPUSPEED speed)
{
  return (SU_u32)(t2 - t1);
}
#else /* !_WIN64 */
#ifndef _M_IX86
#error Unsupported architecture : Win32 && !ix86 (contact zekiller[AT]skytech[DOT]org)
#endif /* !_M_IX86 */

SKYUTILS_API SU_CPUSPEED SU_GetCPUSpeed(void)
{
  SU_u32 speed,time_low,time_high;
  SU_u32 delai;
  DWORD t1,t2;

  t1 = GetTickCount();
  __asm {
    rdtsc
    mov time_low,eax
    mov time_high,edx
  }
  Sleep(2000);
  __asm {
    rdtsc
    sub eax,time_low
    sbb edx,time_high
    mov time_low,eax
    mov time_high,edx
  }
  t2 = GetTickCount();
  delai = (t2 - t1); /* msec */
  __asm {
    mov eax,time_low
    mov edx,time_high
    div delai
    mov speed,eax
  }

  return speed; /* kHtz */
}

SKYUTILS_API void SU_GetTicks(SU_TICKS *tick)
{
  __asm {
    mov edi,tick
    rdtsc
    mov [edi],eax
    mov [edi+4],edx
  }
}

SKYUTILS_API SU_u32 SU_ElapsedTime(SU_TICKS t1,SU_TICKS t2,SU_CPUSPEED speed)
{
  SU_u32 val;
  __asm {
    lea esi,t1
    lea edi,t2
    mov eax,[edi]
    mov edx,[edi+4]
    sub eax,[esi]
    sbb edx,[esi+4]
    div speed
    mov val,eax
  }
  return val;
}
#endif /* _WIN64 */
#else /* !_MSC_VER */
//#ifdef __i386__
#if 0 /* Temporary disabled */
SKYUTILS_API SU_CPUSPEED inline SU_GetCPUSpeed(void)
{
  SU_u32 speed,time_low,time_high;
  SU_u32 delai;
  struct timeval t1,t2;
  struct timezone tz = {0,0};
  SU_u64 v1,v2;

  gettimeofday(&t1,&tz);
  asm volatile (
    "rdtsc\n"
    "movl %%eax,%0\n"
    "movl %%edx,%1\n"
    : "=r" (time_low), "=r" (time_high)
  );
  sleep(2);
  asm volatile (
    "rdtsc\n"
    "subl %0,%%eax\n"
    "sbbl %1,%%edx\n"
    "movl %%eax,%0\n"
    "movl %%edx,%1\n"
    : "=r" (time_low), "=r" (time_high)
    : "r" (time_low), "r" (time_high)
  );
  gettimeofday(&t2,&tz);
  v1 = (t1.tv_sec * 1000) + (t1.tv_usec / 1000);
  v2 = (t2.tv_sec * 1000) + (t2.tv_usec / 1000);
  delai = (SU_u32)(v2 - v1); /* msec */
  asm volatile (
    "movl %1,%%eax\n"
    "movl %2,%%edx\n"
    "divl %3\n"
    "movl %%eax,%0\n"
    : "=r" (speed)
    : "r" (time_low), "r" (time_high), "r" (delai)
  );

  return speed; /* kHtz */
}

SKYUTILS_API void inline SU_GetTicks(SU_TICKS *tick)
{
  unsigned int __a,__d;
  asm volatile(
    "rdtsc\n"
    : "=a" (__a), "=d" (__d)
  );

  *tick = ((unsigned long)__a) | (((unsigned long)__d)<<32);
}

SKYUTILS_API SU_u32 inline SU_ElapsedTime(SU_TICKS t1,SU_TICKS t2,SU_CPUSPEED speed)
{
  SU_u32 val;
  if(speed == 0)
  {
    printf("SkyUtils_ElapsedTime : CPU speed is 0 !! Have you called SU_GetCPUSpeed ??\n");
    return 0;
  }
  asm volatile (
    "lea (%1),%%esi\n"
    "lea (%2),%%edi\n"
    "movl 0,%%ebx\n"
    "movl (%%edi),%%eax\n"
    "movl (%%edi,%%ebx,4),%%edx\n"
    "subl (%%esi),%%eax\n"
    "sbbl (%%esi,%%ebx,4),%%edx\n"
    "divl %3\n"
    : "=a" (val)
    : "r" (t1), "r" (t2), "r" (speed)
  );
  return val;
}

#else /* !__i386__ */
SKYUTILS_API SU_CPUSPEED SU_GetCPUSpeed(void)
{
  return 0;
}

SKYUTILS_API void SU_GetTicks(SU_TICKS *tick)
{
  struct timeval t1;
  struct timezone tz = {0,0};

  gettimeofday(&t1,&tz);
  *tick = (t1.tv_sec * 1000) + (t1.tv_usec / 1000);
}

SKYUTILS_API SU_u32 SU_ElapsedTime(SU_TICKS t1,SU_TICKS t2,SU_CPUSPEED speed)
{
  return t2 - t1;
}
#endif /* __i386__ */
#endif /* _WIN32 */

SKYUTILS_API double SU_GetTimeAsMilli()
{
  double usec = 0;
#ifdef _WIN32
  struct _timeb _gtodtmp; 
  _ftime(&_gtodtmp); 
  usec = (double)_gtodtmp.millitm + (double)_gtodtmp.time * 1000;
#else
  struct timeval td;
  gettimeofday(&td,NULL);
  usec = td.tv_usec / 1000 + td.tv_sec * 1000;
#endif
  return usec;
}


SKYUTILS_API FILE *SU_fmemopen(void *buf,size_t size,const char *opentype)
{
  FILE *f;

  if(opentype[0] != 'r')
    return NULL;

  f = tmpfile();
  fwrite(buf,1,size,f);
  rewind(f);

  return f;
}

SKYUTILS_API void SU_FreeMemory(void *ptr)
{
  free(ptr);
}

#ifdef _WIN32

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
LPFN_ISWOW64PROCESS _SU_fnIsWow64Process = NULL;
typedef void (WINAPI *LPFN_GETNATIVESYSTEMINFO) (LPSYSTEM_INFO);
LPFN_GETNATIVESYSTEMINFO _SU_fnGetNativeSystemInfo = NULL;
HMODULE _SU_Mod_Kernel32 = NULL;
 
SKYUTILS_API bool SU_IsWow64(void)
{
  BOOL bIsWow64 = FALSE;

  if(_SU_fnIsWow64Process == NULL)
  {
    if(_SU_Mod_Kernel32 == NULL)
      _SU_Mod_Kernel32 = LoadLibrary("KERNEL32\0");
    if(_SU_Mod_Kernel32)
      _SU_fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(_SU_Mod_Kernel32,"IsWow64Process\0");
  }
  if(_SU_fnIsWow64Process)
  {
    if(!_SU_fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
    {
      return FALSE;
    }
  }
  return (bool)bIsWow64;
}

SKYUTILS_API bool SU_IsOS64(void)
{
  bool bIs64 = false;

  if(_SU_fnGetNativeSystemInfo == NULL)
  {
    if(_SU_Mod_Kernel32 == NULL)
      _SU_Mod_Kernel32 = LoadLibrary("KERNEL32\0");
    if(_SU_Mod_Kernel32)
      _SU_fnGetNativeSystemInfo = (LPFN_GETNATIVESYSTEMINFO)GetProcAddress(_SU_Mod_Kernel32,"GetNativeSystemInfo\0");
  }
  if(_SU_fnGetNativeSystemInfo)
  {
    SYSTEM_INFO si;
    _SU_fnGetNativeSystemInfo(&si);
    return si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64;
  }
  return bIs64;
}

#else /* !_WIN32 */

SKYUTILS_API bool SU_IsWow64(void)
{
  return false;
}

#include <sys/utsname.h>
SKYUTILS_API bool SU_IsOS64(void)
{
  struct utsname name;

  if(uname(&name) == 0)
  {
    if(SU_strcasecmp(name.machine,"x86_64"))
      return true;
  }

  return false;
}

#endif /* _WIN32 */

#include <math.h>
SKYUTILS_API SU_s32 SU_roundf(float v)
{
  double r = floor(v);
  if((v-r) > 0.5)
    return (SU_s32)(r+1);
  return (SU_s32)(r);
}

SKYUTILS_API SU_s32 SU_roundd(double v)
{
  double r = floor(v);
  if((v-r) > 0.5)
    return (SU_s32)(r+1);
  return (SU_s32)(r);
}

SKYUTILS_API SU_s64 SU_roundf64(float v)
{
  double r = floor(v);
  if((v-r) > 0.5)
    return (SU_s64)(r+1);
  return (SU_s64)(r);
}

SKYUTILS_API SU_s64 SU_roundd64(double v)
{
  double r = floor(v);
  if((v-r) > 0.5)
    return (SU_s64)(r+1);
  return (SU_s64)(r);
}

#ifdef _WIN32
SKYUTILS_API SU_FILE_HANDLE SU_CreateFile(const char *FilePath,SU_u32 CreateFlags,bool FailIsExists,bool NoCaching)
{
  DWORD flags = FailIsExists ? CREATE_NEW : CREATE_ALWAYS;
  DWORD cacheFlags = NoCaching ? FILE_FLAG_NO_BUFFERING|FILE_FLAG_WRITE_THROUGH : 0;
  
  return CreateFile(FilePath,GENERIC_WRITE,0,NULL,flags,CreateFlags|cacheFlags,NULL);
}

SKYUTILS_API SU_FILE_HANDLE SU_OpenFile(const char *FilePath,SU_u32 Access,bool CreateIfNotPresent,bool NoCaching,bool Append)
{
  DWORD access = ((Access & SU_FILE_OPEN_ACCESS_READ) ? GENERIC_READ : 0) | ((Access & SU_FILE_OPEN_ACCESS_WRITE) ? GENERIC_WRITE : 0) | (Append ? FILE_APPEND_DATA : 0);
  DWORD flags = CreateIfNotPresent ? OPEN_ALWAYS : OPEN_EXISTING;
  DWORD cacheFlags = NoCaching ? FILE_FLAG_NO_BUFFERING|FILE_FLAG_WRITE_THROUGH : 0;
  SU_FILE_HANDLE Handle;

  Handle = CreateFile(FilePath,access,0,NULL,flags,cacheFlags,NULL);
  if(Handle == INVALID_HANDLE_VALUE)
    return NULL;
  return Handle;
}

SKYUTILS_API SU_u32 SU_ReadFile(SU_FILE_HANDLE Handle,char *Buffer,SU_u32 len)
{
  DWORD NumberOfBytesRead = len;

  if(ReadFile(Handle,Buffer,len,&NumberOfBytesRead,NULL) == 0)
    return 0;
  return NumberOfBytesRead;
}

SKYUTILS_API SU_u32 SU_WriteFile(SU_FILE_HANDLE Handle,char *Buffer,SU_u32 len)
{
  DWORD NumberOfBytesWritten;

  if(WriteFile(Handle,Buffer,len,&NumberOfBytesWritten,NULL) == 0)
    return 0;
  return NumberOfBytesWritten;
}

SKYUTILS_API bool SU_CloseFile(SU_FILE_HANDLE Handle)
{
  return CloseHandle(Handle) != 0;
}

SKYUTILS_API SU_s64 SU_GetFileSize(SU_FILE_HANDLE Handle)
{
  LARGE_INTEGER size;
  if(GetFileSizeEx(Handle,&size) == 0)
    return -1;
  return (SU_s64)(size.QuadPart);
}

SKYUTILS_API bool SU_FlushFileCache(SU_FILE_HANDLE Handle)
{
  return FlushFileBuffers(Handle) != 0;
}

SKYUTILS_API bool SU_GetFileDate(const char *FilePath,SU_u64 *CreateTime,SU_u64 *ModifiedTime)
{
  FILETIME CreationTime,AccessTime,WriteTime;
  SU_FILE_HANDLE file;

  file = CreateFile(FilePath,FILE_WRITE_ATTRIBUTES,0,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL);
  if(file == INVALID_HANDLE_VALUE)
    return false;

  if(GetFileTime(file,&CreationTime,&AccessTime,&WriteTime) == 0)
  {
    CloseHandle(file);
    return false;
  }

  if(CreateTime)
  {
    *CreateTime = ((SU_u64)CreationTime.dwHighDateTime << 32) + CreationTime.dwLowDateTime;
  }

  if(ModifiedTime)
  {
    *ModifiedTime = ((SU_u64)WriteTime.dwHighDateTime << 32) + WriteTime.dwLowDateTime;
  }

  CloseHandle(file);
  return true;
}

SKYUTILS_API bool SU_SetFileDate(const char *FilePath,SU_u64 CreateTime,SU_u64 ModifiedTime)
{
  FILETIME CreationTime,WriteTime;
  FILETIME *lpCreationTime = NULL;
  FILETIME *lpWriteTime = NULL;
  SU_FILE_HANDLE file;

  file = CreateFile(FilePath,FILE_WRITE_ATTRIBUTES,0,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL);
  if(file == INVALID_HANDLE_VALUE)
    return false;

  if(CreateTime != 0)
  {
    CreationTime.dwHighDateTime = (DWORD)(CreateTime >> 32);
    CreationTime.dwLowDateTime = (DWORD)(CreateTime & 0xFFFFFFFF);
    lpCreationTime = &CreationTime;
  }

  if(ModifiedTime != 0)
  {
    WriteTime.dwHighDateTime = (DWORD)(ModifiedTime >> 32);
    WriteTime.dwLowDateTime = (DWORD)(ModifiedTime & 0xFFFFFFFF);
    lpWriteTime = &WriteTime;
  }

  if(SetFileTime(file,lpCreationTime,NULL,lpWriteTime) == 0)
  {
    CloseHandle(file);
    return false;
  }

  CloseHandle(file);
  return true;
}

SKYUTILS_API SU_u64 SU_GetSystemTime(void)
{
  SYSTEMTIME SystemTime;
  FILETIME SystemTimeAsFileTime;

  GetSystemTime(&SystemTime);
  SystemTimeToFileTime(&SystemTime,&SystemTimeAsFileTime);

  return ((SU_u64)SystemTimeAsFileTime.dwHighDateTime << 32) + SystemTimeAsFileTime.dwLowDateTime;
}

#else /* !_WIN32 */

SKYUTILS_API SU_FILE_HANDLE SU_CreateFile(const char *FilePath,SU_u32 CreateFlags,bool FailIsExists,bool NoCaching)
{
  if(FailIsExists) /* Must check file existance */
  {
    FILE *fp = fopen(FilePath,"rb");
    if(fp != NULL)
    {
      fclose(fp);
      return NULL;
    }
  }
  return fopen(FilePath,"wb");
}

SKYUTILS_API SU_FILE_HANDLE SU_OpenFile(const char *FilePath,SU_u32 Access,bool CreateIfNotPresent,bool NoCaching,bool Append)
{
  FILE *fp = NULL;

  switch(Access)
  {
    case SU_FILE_OPEN_ACCESS_READ:
      fp = fopen(FilePath,"rb");
      break;
    case SU_FILE_OPEN_ACCESS_WRITE:
      if(Append)
        fp = fopen(FilePath,"ab");
      else
        fp = fopen(FilePath,"wb");
      break;
    case (SU_FILE_OPEN_ACCESS_READ|SU_FILE_OPEN_ACCESS_WRITE):
      fp = fopen(FilePath,"rwb");
      break;
  }
  if(fp == NULL && CreateIfNotPresent)
  {
    fp = fopen(FilePath,"wb");
    if(fp == NULL)
      return NULL;
    fclose(fp);
    fp = SU_OpenFile(FilePath,Access,false,NoCaching,Append); /* Re-open the newly created file with correct flags but without CreateIfNotPresent to prevent infinite loop if FilePath is invalid */
  }
  return fp;
}

SKYUTILS_API SU_u32 SU_ReadFile(SU_FILE_HANDLE Handle,char *Buffer,SU_u32 len)
{
  return fread(Buffer,1,len,Handle);
}

SKYUTILS_API SU_u32 SU_WriteFile(SU_FILE_HANDLE Handle,char *Buffer,SU_u32 len)
{
  return fwrite(Buffer,1,len,Handle);
}

SKYUTILS_API bool SU_CloseFile(SU_FILE_HANDLE Handle)
{
  return fclose(Handle) == 0;
}

SKYUTILS_API SU_s64 SU_GetFileSize(SU_FILE_HANDLE Handle)
{
  SU_s64 saf = (SU_s64)ftell(Handle);
  SU_s64 value;

  fseek(Handle,0,SEEK_END);
  value = (SU_s64)ftell(Handle);
  fseek(Handle,saf,SEEK_SET);
  return value;
}

SKYUTILS_API bool SU_FlushFileCache(SU_FILE_HANDLE Handle)
{
  return true;
}

SKYUTILS_API bool SU_GetFileDate(const char *FilePath,SU_u64 *CreateTime,SU_u64 *ModifiedTime)
{
  struct stat st;

  if(stat(FilePath,&st) != 0) /* Error? */
    return false;

  if(CreateTime)
  {
    *CreateTime = (SU_u64)(st.st_ctime);
  }

  if(ModifiedTime)
  {
    *ModifiedTime = (SU_u64)(st.st_mtime);
  }

  return true;
}

SKYUTILS_API bool SU_SetFileDate(const char *FilePath,SU_u64 CreateTime,SU_u64 ModifiedTime)
{
  return false;
}

SKYUTILS_API SU_u64 SU_GetSystemTime(void)
{
  return (SU_u64)time(NULL);
}

#endif /* _WIN32 */

SKYUTILS_API SU_s64 SU_GetFileSizeFromName(const char *FileName)
{
  SU_FILE_HANDLE fp = SU_OpenFile(FileName,SU_FILE_OPEN_ACCESS_READ,false,false,false);
  SU_s64 size;

  if(fp == NULL)
    return -1;

  size = SU_GetFileSize(fp);
  SU_CloseFile(fp);
  return size;
}


SKYUTILS_API void SU_Dummy303(void) {}

