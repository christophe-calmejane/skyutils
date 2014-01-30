/******************************************************************************/
/* Skyutils functions - Chained list,socket,string,utils,web,threads, archive */
/* (c) Christophe CALMEJANE (Ze KiLleR) - 1999-2014                           */
/******************************************************************************/

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

#ifndef __SKY_UTILS_H__
#define __SKY_UTILS_H__

/* Overriding FD_SETSIZE */
#ifdef FD_SETSIZE
#undef FD_SETSIZE
#endif /* FD_SETSIZE */
#define FD_SETSIZE 256

#define SKYUTILS_VERSION "4.06"
#define SKYUTILS_AUTHOR "Christophe Calméjane"

#if defined(__MACH__) || defined(_AIX)
#define __unix__
#endif /* __MACH__ || _AIX */

#if defined(__cplusplus)/* && !defined(__BORLANDC__)*/
extern "C" {
#endif /* __cplusplus */

#ifndef __cplusplus
#ifndef SU_BOOL_TYPE
#undef bool
typedef unsigned int bool;
#define true 1
#define false 0
#define SU_BOOL_TYPE
#endif /* !SU_BOOL_TYPE */
#endif /* !__cplusplus */
#define SU_BOOL unsigned int

#ifndef MAX
#define MAX(a,b) (((a) > (b))?(a):(b))
#endif /* !MAX */
#ifndef MIN
#define MIN(a,b) (((a) < (b))?(a):(b))
#endif /* !MIN */

#ifndef SU_NO_INCLUDES
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <pthread.h>
#include <semaphore.h>
#include <netinet/tcp.h>
#define SU_SOCKET int
#define SU_INVALID_SOCKET (-1)
//#ifdef __APPLE__
#define SU_SOCKLEN_T socklen_t
//#else /* !__APPLE__ */
//#define SU_SOCKLEN_T int
//#endif /* __APPLE */
#define SKYUTILS_API

#else /* _WIN32 */

#ifdef SKYUTILSDLL_EXPORTS
#define SKYUTILS_API __declspec(dllexport)
#else /* !SKYUTILSDLL_EXPORTS */
#ifdef SKYUTILSDLL_IMPORTS
#define SKYUTILS_API __declspec(dllimport)
#else /* !SKYUTILSDLL_EXPORTS && !SKYUTILSDLL_IMPORTS */
#define SKYUTILS_API 
#endif /* SKYUTILSDLL_IMPORTS */
#endif /* SKYUTILSDLL_EXPORTS */
#if defined(_MT) && !defined(_REENTRANT)
#define _REENTRANT
#endif /* _MT */
#ifndef _WINSOCKAPI_
#include <winsock2.h>
#endif /* !_WINSOCKAPI_ */
#include <process.h>
#include <winbase.h>
#include <time.h>
#define SU_SOCKET SOCKET
#define SU_INVALID_SOCKET (SOCKET)(~0)
#define SU_SOCKLEN_T int
#endif /* !_WIN32 */

#endif /* !SU_NO_INCLUDES */

#ifdef MSG_NOSIGNAL
#define SU_MSG_NOSIGNAL MSG_NOSIGNAL
#else /* !MSG_NOSIGNAL */
#define SU_MSG_NOSIGNAL 0
#endif /* MSG_NOSIGNAL */

#ifndef INADDR_NONE
#define INADDR_NONE -1
#endif /* !INADDR_NONE */

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif /* !SOCKET_ERROR */
#define SU_NOT_A_SOCKET 0

#define SU_UDP_MAX_LENGTH 64000

/* Portable types */
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef unsigned __int8 SU_u8;
typedef signed __int8 SU_s8;
typedef unsigned __int16 SU_u16;
typedef signed __int16 SU_s16;
typedef unsigned __int32 SU_u32;
typedef signed __int32 SU_s32;
typedef unsigned __int64 SU_u64;
typedef signed __int64 SU_s64;
#else /* !(_MSC_VER || __BORLANDC__) */
#if defined(__GNUC__) && defined (__linux__) && !defined(_WIN32) && !defined(__MINGW32__)
typedef u_int8_t SU_u8;
typedef int8_t SU_s8;
typedef u_int16_t SU_u16;
typedef int16_t SU_s16;
typedef u_int32_t SU_u32;
typedef int32_t SU_s32;
typedef u_int64_t SU_u64;
typedef int64_t SU_s64;
#else /* Not linux GCC */
typedef unsigned char SU_u8;
typedef signed char SU_s8;
typedef unsigned short SU_u16;
typedef signed short SU_s16;
typedef unsigned int SU_u32;
typedef signed int SU_s32;
typedef unsigned long long SU_u64;
typedef signed long long SU_s64;
#endif /* Linux GCC */
#endif /* _MSC_VER || __BORLANDC__ */


/* **************************************** */
/*            Chained list functions        */
/* **************************************** */
struct SU_SList;

typedef struct SU_SList
{
  struct SU_SList *Next;
  void *Data;
} SU_TList, *SU_PList;

typedef struct SU_SAssoc
{
  struct SU_SAssoc *Next;
  void *Left;
  void *Right;
} SU_TAssoc, *SU_PAssoc;


SKYUTILS_API SU_PList SU_AddElementTail(SU_PList,void *);
SKYUTILS_API SU_PList SU_AddElementHead(SU_PList,void *);
SKYUTILS_API SU_PList SU_AddElementPos(SU_PList List,int Pos,void *Elem); /* First element is at pos 0 */
SKYUTILS_API SU_PList SU_DelElementElem(SU_PList,void *); /* This function does NOT free the element */
SKYUTILS_API SU_PList SU_DelElementTail(SU_PList); /* This function does NOT free the element */
SKYUTILS_API SU_PList SU_DelElementHead(SU_PList); /* This function does NOT free the element */
SKYUTILS_API SU_PList SU_DelElementPos(SU_PList,int); /* This function does NOT free the element */ /* First element is at pos 0 */
SKYUTILS_API void *SU_GetElementTail(SU_PList);
SKYUTILS_API void *SU_GetElementHead(SU_PList);
SKYUTILS_API void *SU_GetElementPos(SU_PList,int); /* First element is at pos 0 */
SKYUTILS_API void SU_FreeList(SU_PList); /* This function does NOT free the elements */
SKYUTILS_API void SU_FreeListElem(SU_PList); /* This function DOES free the elements */
SKYUTILS_API unsigned int SU_ListCount(SU_PList);

SKYUTILS_API SU_PAssoc SU_AddAssocTail(SU_PAssoc,void *,void *);
SKYUTILS_API SU_PAssoc SU_AddAssocHead(SU_PAssoc,void *,void *);
SKYUTILS_API SU_PAssoc SU_AddAssocPos(SU_PAssoc List,int Pos,void *,void *); /* First Assoc is at pos 0 */
SKYUTILS_API SU_PAssoc SU_DelAssocLeft(SU_PAssoc,void *); /* This function does NOT free the Assoc */
SKYUTILS_API SU_PAssoc SU_DelAssocTail(SU_PAssoc); /* This function does NOT free the Assoc */
SKYUTILS_API SU_PAssoc SU_DelAssocHead(SU_PAssoc); /* This function does NOT free the Assoc */
SKYUTILS_API SU_PAssoc SU_DelAssocPos(SU_PAssoc,int); /* This function does NOT free the Assoc */ /* First Assoc is at pos 0 */
SKYUTILS_API bool SU_GetAssocTail(SU_PAssoc,void **Left,void **Right);
SKYUTILS_API bool SU_GetAssocHead(SU_PAssoc,void **Left,void **Right);
SKYUTILS_API bool SU_GetAssocPos(SU_PAssoc,int,void **Left,void **Right); /* First Assoc is at pos 0 */
SKYUTILS_API bool SU_GetAssocLeft(SU_PAssoc,void *Left,void **Right); /* Returns Assoc matching "Left" value */
SKYUTILS_API void SU_FreeAssoc(SU_PAssoc); /* This function does NOT free the Assocs */
SKYUTILS_API void SU_FreeAssocAssoc(SU_PAssoc); /* This function DOES free the Assocs */
SKYUTILS_API unsigned int SU_AssocCount(SU_PAssoc);


/* **************************************** */
/*               Socket functions           */
/* **************************************** */
#ifndef SU_INCLUDE_NO_SOCKS
typedef struct
{
  SU_SOCKET sock;
  struct sockaddr_in SAddr;
  void *User;
} SU_TServerInfo, *SU_PServerInfo;

typedef struct
{
  SU_SOCKET sock;
  struct sockaddr_in SAddr;
  void *User;
} SU_TClientSocket, *SU_PClientSocket;

typedef struct
{
  SU_PClientSocket father; /* father */

  int nb_sons;             /* Count of sons */
  SU_PClientSocket *sons;  /* sons */
} SU_TMultiSocket, *SU_PMultiSocket;

typedef void (SU_MULTISOCK_CB_FUNC)(SU_PMultiSocket socks,SU_PClientSocket son);


SKYUTILS_API int SU_GetPortByName(char *port,char *proto); /* Returns port number from it's name */
SKYUTILS_API char *SU_GetMachineName(char *RemoteHost);    /* Extracts the machine name from a full host */
SKYUTILS_API char *SU_NameOfPort(char *Host);              /* Returns the host name matching the given ip */
SKYUTILS_API char *SU_AdrsOfPort(char *Host);              /* Returns the ip adrs matching the given host */
SKYUTILS_API const char* SU_GetSocketName(SU_SOCKET sock); /* Returns the local computer's ip address for the given socket */
SKYUTILS_API unsigned int SU_GetSocketPort(SU_SOCKET sock); /* Returns the local computer's port associated with given socket */
SKYUTILS_API const char* SU_GetSocketRemoteName(SU_SOCKET sock); /* Returns the remote computer's ip address for the given socket */
SKYUTILS_API unsigned int SU_GetSocketRemotePort(SU_SOCKET sock); /* Returns the remote computer's port associated with given socket */


/* 
 Sends a buffer to socket.
 If 'packet_size' is not 0, sub buffers of this size will be issued to the socket layer
 Returns : 
  0  : Socket closed
  -1 : Read error on socket (see SU_errno)
  >0 : Total of bytes sent
*/
SKYUTILS_API int SU_SendTCPBuffer(SU_SOCKET sock,char *buf,int size,int packet_size);

/* 
 Reads a buffer from socket. If TimeOut is not NULL, select-timeout is used. If WaitForFullBuffer is true, don't return until error, or 'size' bytes has been read
 Returns : 
  0  : Socket closed
  -1 : Read error on socket (see SU_errno)
  -2 : Read timed out (value defined in TimeOut parameter)
  >0 : Total of bytes read
*/
SKYUTILS_API int SU_ReadTCPBuffer(SU_SOCKET sock,char *buf,int size,struct timeval *TimeOut,bool WaitForFullBuffer);

/* 
 Select on multiple socket at the same time, for a read operation. If TimeOut is not NULL, select-timeout is used. CB is called for each socket that is ready for a read.
 Returns : 
  0  : Function completed OK
  -1 : Read error on socket (see SU_errno)
  -2 : Read timed out (value defined in TimeOut parameter)
*/
SKYUTILS_API int SU_SelectMultiSocketForRead(SU_PMultiSocket socks,struct timeval *TimeOut,SU_MULTISOCK_CB_FUNC *CB);

/* 
 Sends a buffer to a multiple socket struct.
 If 'packet_size' is not 0, sub buffers of this size will be issued to the socket layer
 Returns (in rets) : 
  0  : Socket closed
  -1 : Read error on socket (see SU_errno)
  >0 : Total of bytes sent
*/
SKYUTILS_API void SU_SendTCPBufferToMultiSocket(SU_PMultiSocket socks,char *buf,int size,int packet_size,int *rets);

SKYUTILS_API SU_PServerInfo SU_CreateServer(int port,int type,bool ReUseAdrs);  /* Returns NULL on error */
SKYUTILS_API int SU_ServerListen(SU_PServerInfo SI);                            /* SOCKET_ERROR on Error */
SKYUTILS_API SU_PClientSocket SU_ServerAcceptConnection(SU_PServerInfo SI);     /* Returns NULL on error */
SKYUTILS_API int SU_ServerAcceptConnectionWithTimeout(SU_PServerInfo SI,struct timeval *TimeOut,SU_PClientSocket *NewSon); /* Returns -1 on error, 0 on timeout, 1 if son accepted */
SKYUTILS_API void SU_ServerDisconnect(SU_PServerInfo SI);
SKYUTILS_API void SU_FreeSI(SU_PServerInfo SI); /* Disconnect and free SI */
SKYUTILS_API SU_PClientSocket SU_ClientConnect(char *adrs,char *port,int type); /* Returns NULL on error */
SKYUTILS_API int SU_ClientSend(SU_PClientSocket CS,char *msg);                  /* SOCKET_ERROR on Error */
SKYUTILS_API int SU_ClientSendBuf(SU_PClientSocket CS,char *buf,int len);       /* SOCKET_ERROR on Error */
SKYUTILS_API void SU_ClientDisconnect(SU_PClientSocket CS);
SKYUTILS_API void SU_FreeCS(SU_PClientSocket CS); /* Disconnect and free CS */
SKYUTILS_API int SU_UDPSendBroadcast(SU_PServerInfo SI,char *Text,int len,char *port);           /* SOCKET_ERROR on Error */
SKYUTILS_API int SU_UDPSendToAddr(SU_PServerInfo SI,char *Text,int len,char *Addr,char *port);   /* SOCKET_ERROR on Error */
SKYUTILS_API int SU_UDPSendToSin(SU_PServerInfo SI,char *Text,int len,struct sockaddr_in);       /* SOCKET_ERROR on Error */
SKYUTILS_API int SU_UDPReceiveFrom(SU_PServerInfo SI,char *Text,int len,char **ip,int Blocking); /* SOCKET_ERROR on Error */
SKYUTILS_API int SU_UDPReceiveFromSin(SU_PServerInfo SI,char *Text,int len,struct sockaddr_in *,int Blocking); /* SOCKET_ERROR on Error */
SKYUTILS_API int SU_SetSocketOpt(SU_SOCKET sock,int Opt,int value); /* SOCKET_ERROR on Error */
SKYUTILS_API int SU_SetTcpOpt(SU_SOCKET sock,int Opt,int value); /* SOCKET_ERROR on Error */
SKYUTILS_API bool SU_SetSocketBlocking(SU_SOCKET sock,bool Block); /* True on Success */
SKYUTILS_API bool SU_SplitIPv4(const char *IPin,unsigned char IPout[4]); /* Converts an IPv4 as string to an array */
SKYUTILS_API bool SU_SplitMAC(const char *MACin,unsigned char MACout[6]); /* Converts a MAC as string to an array */
SKYUTILS_API bool SU_WakeUpComputer(const char *IP,const char *port,const char *MAC); /* Wakes a computer from LAN */
SKYUTILS_API bool SU_SockInit(int Major,int Minor); /* Inits Socks (MUST BE CALL BEFORE ANY OTHER FUNCTION) */
SKYUTILS_API void SU_SockUninit(void); /* Uninits Socks (MUST BE CALL BEFORE EXITING) */
#ifdef _WIN32
#define SU_CLOSE_SOCKET(x) closesocket(x)
#define SU_ECONNABORTED WSAECONNABORTED
#define SU_EAGAIN WSAEWOULDBLOCK
#define SU_ENOBUFS WSAENOBUFS
#define SU_errno WSAGetLastError()
#define SU_seterrno(x) WSASetLastError(x)
#define SU_ioctl ioctlsocket
#else /* !_WIN32 */
#define SU_CLOSE_SOCKET(x) close(x)
#define SU_ECONNABORTED ECONNABORTED
#define SU_EAGAIN EAGAIN
#define SU_ENOBUFS ENOBUFS
#define SU_seterrno(x) (errno = x)
#define SU_errno errno
#define SU_ioctl ioctl
#endif /* _WIN32 */
#endif /* !SU_INCLUDE_NO_SOCKS */


/* **************************************** */
/*               String functions           */
/* **************************************** */
SKYUTILS_API char *SU_strcat(char *dest,const char *src,size_t len); /* like strncat, but always NULL terminate dest */
SKYUTILS_API char *SU_strcpy(char *dest,const char *src,size_t len); /* like strncpy, but doesn't pad with 0, and always NULL terminate dest */
SKYUTILS_API int SU_snprintf(char *dest,size_t len, const char *format,...); /* like snprintf, but always NULL terminate dest - Returns -1 if string truncated */
SKYUTILS_API char *SU_nocasestrstr(char *text, const char *tofind);  /* like strstr(), but nocase */
SKYUTILS_API bool SU_strwcmp(const char *s,const char *wild); /* True if wild equals s (wild may use '*') */
SKYUTILS_API bool SU_nocasestrwcmp(const char *s,const char *wild); /* Same as strwcmp but without case */
SKYUTILS_API bool SU_strwparse(const char *s,const char *wild,char buf[],int size,char *buf_ptrs[],int *ptrs_count); /* True if wild equals s (wild may use '*') */
SKYUTILS_API bool SU_nocasestrwparse(const char *s,const char *wild,char buf[],int size,char *buf_ptrs[],int *ptrs_count); /* Same as nocasestrwparse but without case */
SKYUTILS_API unsigned int SU_strnlen(const char *s,unsigned int max); /* Returns MAX(length of a string,max) (not including terminating null char) */
SKYUTILS_API bool SU_ReadLine(FILE *fp,char S[],int len); /* Returns false on EOF */
SKYUTILS_API bool SU_ParseConfig(FILE *fp,char Name[],int nsize,char Value[],int vsize); /* Returns false on EOF */
SKYUTILS_API char *SU_TrimLeft(const char *S);
SKYUTILS_API void SU_TrimRight(char *S);
SKYUTILS_API char *SU_strparse(char *s,char delim); /* Like strtok, but if 2 consecutive delim are found, an empty string is returned (s[0] = 0) */
SKYUTILS_API void SU_ExtractFileName(const char Path[],char FileName[],const int len); /* Extracts file name (with suffix) from path */
SKYUTILS_API char *SU_strchrl(const char *s,const char *l,char *found); /* Searchs the first occurence of one char of l[i] in s, and returns it in found */
SKYUTILS_API char *SU_strrchrl(const char *s,const char *l,char *found); /* Same as SU_strchrl but starting from the end of the string */
SKYUTILS_API unsigned char SU_toupper(unsigned char c);
SKYUTILS_API unsigned char SU_tolower(unsigned char c);
SKYUTILS_API char *SU_strtoupper(char *s); /* s IS modified */
SKYUTILS_API char *SU_strtolower(char *s); /* s IS modified */
SKYUTILS_API bool SU_strcasecmp(const char *s,const char *p);
SKYUTILS_API char *SU_strerror(int ErrorCode); /* Returns a static string which describes the error code (errno on linux, GetLastError on windows) */
SKYUTILS_API int SU_htoi(const char *value); /* Like atoi() but with an HEX string as value */
SKYUTILS_API int SU_atoi(const char *value); /* Like atoi() but if value starts with "0x", then value is interpreted as an HEX string */
SKYUTILS_API const void* SU_memmem(const void* haystack, size_t haystacklen, const void* needle, size_t needlelen); /* Like strstr but with memory buffers */

#ifdef _WIN32
#ifdef __BORLANDC__
#define strcasecmp stricmp
#define strncasecmp strnicmp
#define SU_strdup(x) (((x)==NULL)?NULL:strdup(x))
#define SU_strlen(x) (((x)==NULL)?0:strlen(x))
#else /* !__BORLANDC__ */
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define snprintf _snprintf
/* Set ISO C++ conformant names */
#define getpid _getpid
#define unlink _unlink
#define strdup _strdup
#define SU_strdup(x) (((x)==NULL)?NULL:_strdup(x))
#define SU_strlen(x) (((x)==NULL)?0:strlen(x))
#endif /* __BORLANDC__ */
#else /* !_WIN32 */
#define SU_strdup(x) (((x)==NULL)?NULL:strdup(x))
#define SU_strlen(x) (((x)==NULL)?0:strlen(x))
#endif /* _WIN32 */


/* **************************************** */
/*               Utils functions            */
/* **************************************** */
#ifndef FlagOn
#define FlagOn(x,f) (((x) & (f)) == (f))
#endif /* !FlagOn */
#ifndef FlagOff
#define FlagOff(x,f) (((x) & (f)) == 0)
#endif /* !FlagOff */
#ifndef SetFlag
#define SetFlag(x,f) ((x) |= (f))
#endif /* !SetFlag */
#ifndef ClearFlag
#define ClearFlag(x,f) ((x) &= ~(f))
#endif /* !ClearFlag */

SKYUTILS_API bool SU_IsWow64(void); /* Returns true if the process is running in the WOW64 emulator (32 bit process on a 64 bits windows) */
SKYUTILS_API bool SU_IsOS64(void); /* Returns true if the process is running on a 64 bits OS */
SKYUTILS_API FILE *SU_OpenLogFile(const char LogName[]);
SKYUTILS_API void SU_CloseLogFile(FILE *fp);
SKYUTILS_API void SU_WriteToLogFile(FILE *fp,const char Text[]);
/* Checks the http_proxy env var */
SKYUTILS_API void SU_CheckProxyEnv(void);

/* Remove arguments for skyutils, and returns number of remaining arguments for main */
/* This function automatically calls SU_CheckProxyEnv if no proxy found in params */
SKYUTILS_API int SU_GetSkyutilsParams(int argc,char *argv[]);

SKYUTILS_API char *SU_LoadUserHeaderFile(const char FName[]);
SKYUTILS_API char *SU_GetOptionsString(void);
#ifdef __unix__
SKYUTILS_API bool SU_Daemonize(void);
SKYUTILS_API bool SU_SetUserGroup(const char User[],const char Group[]); /* If User and/or Group is != NULL, setuid and setgid are used */
#endif /* __unix__ */
SKYUTILS_API void SU_SetDebugLevel(const char AppName[],const int Level);
SKYUTILS_API int SU_GetDebugLevel(void);
SKYUTILS_API void SU_PrintSyslog(int Level,char *Txt, ...);
SKYUTILS_API void SU_PrintDebug(int Level,char *Txt, ...);
#ifndef DEBUG
#if defined(__unix__) || (_MSC_VER >= 1500) || ((_MSC_VER >= 1400) && !defined(_WIN64)) /* Unix or Visual 2008 or Visual 2005-32bits */
#define SU_PrintDebug(x,...)
#else /* Visual 6 or 2005 64bits (because fucking cl-64bits does not support '...') */
#define SU_PrintDebug()
#endif /* __unix__ || _MSC_VER >= 1500 || (_MSC_VER >= 1500 && !_WIN64) */
#ifdef __unix__
#include <syslog.h>
#define SU_SYSLOG_FN(x,y) syslog(x,y)
#else /* !__unix__ */
extern FILE *SU_LogFile;
#define SU_SYSLOG_FN(x,y) SU_WriteToLogFile(SU_LogFile,y)
#endif /* __unix__ */
#else /* DEBUG */
#define SU_SYSLOG_FN(x,y) printf(y)
#endif /* !DEBUG */
#ifdef _WIN32
#define SU_SLEEP(x) Sleep(x*1000)
#define SU_USLEEP(x) Sleep(x)
#else /* !_WIN32 */
#define SU_SLEEP(x) sleep(x)
#define SU_USLEEP(x) usleep(x*1000)
#endif /* _WIN32 */
#define SU_ANSI_BLACK  "0"
#define SU_ANSI_RED    "1"
#define SU_ANSI_GREEN  "2"
#define SU_ANSI_YELLOW "3"
#define SU_ANSI_BLUE   "4"
#define SU_ANSI_PINK   "5"
#define SU_ANSI_CYAN   "6"
#define SU_ANSI_WHITE  "7"
#if SU_ENABLE_ANSI_CODE
#define SU_ANSI_COLOR(f,b) "\033[3" f ";4" b "m"
#define SU_ANSI_BLINK      "\033[5m"
#define SU_ANSI_HIGHLIGHT  "\033[1m"
#define SU_ANSI_RESET      "\033[0m"
#else /* !SU_ENABLE_ANSI_CODE */
#define SU_ANSI_COLOR(f,b) ""
#define SU_ANSI_BLINK      ""
#define SU_ANSI_HIGHLIGHT  ""
#define SU_ANSI_RESET      ""
#endif /* SU_ENABLE_ANSI_CODE */
#ifdef _WIN32
typedef SU_u64 SU_TICKS;
#else /* !WIN32 */
typedef SU_u64 SU_TICKS;
#endif /* _WIN32 */
typedef SU_u32 SU_CPUSPEED;
SKYUTILS_API SU_CPUSPEED SU_GetCPUSpeed(void);
SKYUTILS_API void SU_GetTicks(SU_TICKS *tick); /* Returned value in processor ticks (Use SU_ElapsedTime to compute time in msec) */
SKYUTILS_API SU_u32 SU_ElapsedTime(SU_TICKS t1,SU_TICKS t2,SU_CPUSPEED speed); /* Returned value in msec */
SKYUTILS_API FILE *SU_fmemopen(void *buf,size_t size,const char *opentype);
SKYUTILS_API void SU_FreeMemory(void *ptr); /* Frees any memory allocated by SkyUtils function */
SKYUTILS_API double SU_GetTimeAsMilli(); /* Returns current time in milli-sec (since 1970) */
SKYUTILS_API SU_s32 SU_roundf(float);
SKYUTILS_API SU_s32 SU_roundd(double);
SKYUTILS_API SU_s64 SU_roundf64(float);
SKYUTILS_API SU_s64 SU_roundd64(double);

#ifdef _WIN32
#define SU_FILE_HANDLE HANDLE
#else /* !_WIN32 */
#define SU_FILE_HANDLE FILE *
#endif /* _WIN32 */
#define SU_FILE_CREATE_FLAGS_NORMAL 0x80
#define SU_FILE_OPEN_ACCESS_READ  1
#define SU_FILE_OPEN_ACCESS_WRITE 2

SKYUTILS_API SU_FILE_HANDLE SU_CreateFile(const char *FilePath,SU_u32 CreateFlags,bool FailIsExists,bool NoCaching); /* If 'FailIsExists' and file already exists, returns NULL */
SKYUTILS_API SU_FILE_HANDLE SU_OpenFile(const char *FilePath,SU_u32 Access,bool CreateIfNotPresent,bool NoCaching,bool Append); /* If 'CreateIfNotPresent' and file does not exist then create the file - If 'CreateIfNotPresent == false' and file does not exist then returns NULL */
SKYUTILS_API SU_u32 SU_ReadFile(SU_FILE_HANDLE Handle,char *Buffer,SU_u32 len);
SKYUTILS_API SU_u32 SU_WriteFile(SU_FILE_HANDLE Handle,char *Buffer,SU_u32 len);
SKYUTILS_API bool SU_CloseFile(SU_FILE_HANDLE Handle);
SKYUTILS_API SU_s64 SU_GetFileSizeFromName(const char *FileName); /* -1 on error */
SKYUTILS_API SU_s64 SU_GetFileSize(SU_FILE_HANDLE Handle); /* -1 on error */
SKYUTILS_API bool SU_FlushFileCache(SU_FILE_HANDLE Handle);
SKYUTILS_API bool SU_GetFileDate(const char *FilePath,SU_u64 *CreateTime,SU_u64 *ModifiedTime);
SKYUTILS_API bool SU_SetFileDate(const char *FilePath,SU_u64 CreateTime,SU_u64 ModifiedTime);
SKYUTILS_API SU_u64 SU_GetSystemTime(void);


/* **************************************** */
/*              Memory functions            */
/* **************************************** */
#ifndef SU_MALLOC_ALIGN_SIZE
#define SU_MALLOC_ALIGN_SIZE 16
#else /* SU_MALLOC_ALIGN_SIZE */
#if SU_MALLOC_ALIGN_SIZE < 8
#error "SU_MALLOC_ALIGN_SIZE must be strictly greater than 8"
#endif /* SU_MALLOC_ALIGN_SIZE < 8 */
#endif /* !SU_MALLOC_ALIGN_SIZE */
typedef void (SU_PRINT_FUNC)(bool Fatal,char *Txt, ...);
SKYUTILS_API void SU_SetPrintFunc(SU_PRINT_FUNC *Func);
typedef void (SU_MEM_STATS_FUNC)(void *ptr,size_t size,time_t time,const char *file,SU_u32 line);

SKYUTILS_API void *SU_malloc(size_t size); /* Allocates a bloc of memory aligned on SU_MALLOC_ALIGN_SIZE */
SKYUTILS_API void *SU_calloc(size_t nbelem,size_t size); /* Allocates a bloc of memory aligned on SU_MALLOC_ALIGN_SIZE, and zeros it */
SKYUTILS_API void *SU_realloc(void *memblock,size_t size); /* Reallocates a bloc of memory aligned on SU_MALLOC_ALIGN_SIZE */
SKYUTILS_API char *SU_strdup_memory(const char *in); /* Dups a string using a bloc of memory aligned on SU_MALLOC_ALIGN_SIZE */
SKYUTILS_API void SU_free(void *memblock);   /* Frees a bloc previously allocated using SU_malloc */
#ifdef SU_MALLOC_TRACE
#undef calloc
#undef strdup
#define malloc(x) SU_malloc_trace(x,__FILE__,__LINE__)
#define calloc(x,y) SU_calloc_trace(x,y,__FILE__,__LINE__)
#define realloc(x,y) SU_realloc_trace(x,y,__FILE__,__LINE__)
#define strdup(x) SU_strdup_trace(x,__FILE__,__LINE__)
#define free(x) SU_free_trace(x,__FILE__,__LINE__)
#define trace_print SU_alloc_trace_print(true)
#define trace_print_count SU_alloc_trace_print(false)
#endif /* SU_MALLOC_TRACE */
SKYUTILS_API void *SU_malloc_trace(size_t size,char *file,SU_u32 line);
SKYUTILS_API void *SU_calloc_trace(size_t nbelem,size_t size,char *file,SU_u32 line);
SKYUTILS_API void *SU_realloc_trace(void *memblock,size_t size,char *file,SU_u32 line);
SKYUTILS_API char *SU_strdup_trace(const char *in,char *file,SU_u32 line);
SKYUTILS_API void SU_free_trace(void *memblock,char *file,SU_u32 line);
SKYUTILS_API void SU_alloc_trace_print(bool detail); /* Print all allocated blocks (if detail is true), and total number of chunks */
SKYUTILS_API void SU_check_memory(void); /* Check memory heap */
SKYUTILS_API void SU_alloc_stats(SU_MEM_STATS_FUNC *Func); /* Call Func for each chunk of memory */
size_t SU_alloc_total_size(void); /* Returns the total size allocated */ /* Only available with SU_xx_trace functions */
SKYUTILS_API void SU_SetMallocConfig(int check,int trace,int print); /* Sets the debug variables, as if "MALLOC_CHECK_", "SU_MALLOC_TRACE" and "SU_MALLOC_PRINT" were set */
SKYUTILS_API void SU_GetMallocConfig(int *check,int *trace,int *print); /* Returns current internal config for debug */
/* MALLOC_CHECK_ environment variable :
 *   0 : If a memory error occurs, execution silently goes on
 *   1 : If a memory error occurs, a debug message is printed in stdout, and execution goes on
 *   2 or not defined : If a memory error occurs, a debug message is printed in stdout, and abort is called
*/
/* SU_MALLOC_TRACE environment variable :
 *   0 or not defined : SU_free_trace does free trace of associated malloc
 *   1 : SU_free_trace keep trace of associated malloc, and display more debug information if block already freed
*/
/* SU_MALLOC_PRINT environment variable :
 *   0 or not defined : Nothing happens
 *   1 : SU_malloc_trace/SU_free_trace functions print memory bloc and file/line from where the function is called
*/


/* **************************************** */
/*                 web functions            */
/* **************************************** */
#define ACT_GET    1
#define ACT_POST   2
#define ACT_PUT    3
#define ACT_DELETE 4

#define URL_BUF_SIZE 2048

typedef struct
{
  int Code;
  char *Location;

  char *Data;      /* NULL if no data */
  int Data_Length; /* -1 if no data */
  int Data_ToReceive;
} SU_TAnswer, *SU_PAnswer;

struct SU_SHTTPActions;

typedef struct
{
  void (*OnSendingCommand)(struct SU_SHTTPActions *); /* User's CallBack just before sending request */
  void (*OnAnswer)(SU_PAnswer,void *); /* User's CallBack just after answer received */
  void (*OnOk)(SU_PAnswer,void *); /* User's CallBack for a 200 reply */
  void (*OnCreated)(SU_PAnswer,void *); /* User's CallBack for a 201 reply */
  void (*OnModified)(SU_PAnswer,void *); /* User's CallBack for a 202 reply */
  void (*OnMoved)(SU_PAnswer,void *); /* User's CallBack for a 302 reply */
  void (*OnForbidden)(SU_PAnswer,void *); /* User's CallBack for a 403 reply */
  void (*OnNotFound)(SU_PAnswer,void *); /* User's CallBack for a 404 reply */
  void (*OnTooBig)(SU_PAnswer,void *); /* User's CallBack for a 413 reply */
  void (*OnUnknownHost)(SU_PAnswer,void *); /* User's CallBack for a 503 reply */
  void (*OnOtherReply)(SU_PAnswer,int,void *); /* User's CallBack for all other replies */
  void (*OnErrorSendingFile)(int,void *); /* User's CallBack for an error sending file (errno code passed) */
} SU_THTTP_CB, *SU_PHTTP_CB;

typedef struct
{
  char *Header; /* Header to send */
  char *FileName; /* File to send (exclude Data) */
  char *Data; /* Data to send (exclude FileName) */
  int Length; /* Length of data */
} SU_THTTPPart, *SU_PHTTPPart;

typedef struct SU_SHTTPActions
{ /* Info to set BEFORE any call to ExecuteActions */
  int  Command; /* ACT_xxx */
  char URL[URL_BUF_SIZE];
  char *URL_Params; /* ACT_GET & ACT_POST */
  char *Post_Data;  /* ACT_POST */
  int  Post_Length; /* ACT_POST */
  char *ContentType; /* Content-type or use default */ /* ACT_GET & ACT_POST */
  char *FileName;   /* ACT_PUT */ /* URL must contain the URL+New file name */ /* If defined for GET or POST, dump result to this file */
  char *Referer;
  SU_PList MultiParts; /* SU_PHTTPPart */ /* MultiParts, exclude Post_Data */ /* ACT_POST */
  void *User;       /* User's info Passed to Callbacks */
  int  Sleep;       /* Time to wait before sending command (sec) */
  SU_THTTP_CB CB;   /* Callbacks structure */

  /* Info used internally */
  char Host[100];
  bool SSL;
} SU_THTTPActions, *SU_PHTTPActions;

typedef struct
{
  char *Name;
  char *Value;
  char *Domain;
  char *Path;
  char *Expire;
  bool Secured;
} SU_TCookie, *SU_PCookie;

typedef struct
{
  char *Type;
  char *Name;
  char *Value;
} SU_TInput, *SU_PInput;

typedef struct
{
  char *Src;
  char *Name;
} SU_TImage, *SU_PImage;

typedef struct
{
  char *Method;
  char *Name;
  char *Action;
  SU_PList Inputs;
} SU_TForm, *SU_PForm;

/* Sets the UserAgent string to send in every web request. Will not be used if SU_LoadUserHeaderFile() is used */
SKYUTILS_API void SU_SetUserAgent(const char UA[]);

/* Sets proxy server,port, user and password values to be used by ExecuteActions (use NULL for proxy to remove use of the proxy) */
SKYUTILS_API void SU_SetProxy(const char Proxy[],const int Port,const char User[], const char Password[]);

/* Sets the socket connection timeout (use 0 to reset default value) */
SKYUTILS_API void SU_SetSocketTimeout(const int Timeout);

/* Returns 0 if ok, -1 if cannot connect to the host, -2 if a timeout occured */
SKYUTILS_API int SU_ExecuteActions(SU_PList Actions);

SKYUTILS_API void SU_FreeAction(SU_PHTTPActions Act);

SKYUTILS_API SU_PInput SU_GetInput(char *html);
SKYUTILS_API SU_PInput SU_GetNextInput(void);
SKYUTILS_API void SU_FreeInput(SU_PInput In);

SKYUTILS_API SU_PImage SU_GetImage(char *html);
SKYUTILS_API SU_PImage SU_GetNextImage(void);
SKYUTILS_API void SU_FreeImage(SU_PImage Im);

SKYUTILS_API void SU_FreeForm(SU_PForm Form);

/* Retrieves the url (into a SU_PHTTPActions struct) of the 'link' from the 'Ans' page associated with the 'URL' request */
SKYUTILS_API SU_PHTTPActions SU_RetrieveLink(const char URL[],const char Ans[],const char link[],const int index);

/* Retrieve link from a frameset */
SKYUTILS_API SU_PHTTPActions SU_RetrieveFrame(const char URL[],const char Ans[],const char framename[]);

/* Retrieve document.forms[num] */
SKYUTILS_API SU_PForm SU_RetrieveForm(const char Ans[],const int num);

SKYUTILS_API char *SU_AddLocationToUrl(const char *URL,const char *Host,const char *Location,bool ssl_mode);

/* Encodes an URL */
SKYUTILS_API char *SU_EncodeURL(const char URL_in[],char URL_out[],int URL_out_len);

/* Skips white spaces before the string, then extracts it */
SKYUTILS_API char *SU_GetStringFromHtml(const char Ans[],const char TextBefore[]);

SKYUTILS_API void SU_FreeCookie(SU_PCookie Cok);
SKYUTILS_API extern SU_PList SW_Cookies; /* SU_PCookie */


/* **************************************** */
/*               Threads functions          */
/* **************************************** */
#ifndef SU_INCLUDE_NO_THREAD
#define SU_THREAD_RET_TYPE unsigned
#ifdef __unix__
#define SU_THREAD_HANDLE pthread_t
#define SU_THREAD_ID pthread_t
#define SU_THREAD_NULL 0
#define SU_THREAD_ROUTINE_TYPE(x) void *(*x)(void *)
#define SU_THREAD_ROUTINE(x,y) void * x(void *y)
#define SU_END_THREAD(x) pthread_exit(&(x))
#define SU_THREAD_RETURN(x) return (void *)(x);
#define SU_PROCESS_SELF (SU_u32)getpid()
#define SU_THREAD_SELF (SU_THREAD_ID)pthread_self()
#define SU_THREAD_KEY_HANDLE pthread_key_t
#define SU_THREAD_ONCE_HANDLE pthread_once_t
#define SU_THREAD_ONCE_INIT PTHREAD_ONCE_INIT
#define SU_THREAD_ONCE(x,y) pthread_once(&x,y)
#define SU_THREAD_GET_SPECIFIC(x) pthread_getspecific(x)
#define SU_THREAD_SET_SPECIFIC(x,y) pthread_setspecific(x,y)
#define SU_THREAD_DESTROY_SPECIFIC(x,y)
#define SU_SEM_HANDLE sem_t
#define SU_SEM_WAIT(x) { while(sem_wait(&(x)) == -1) { if(errno != EINTR) break; } }
#define SU_SEM_POST(x) sem_post(&(x))
#define SU_SEM_TRY_AND_ENTER(x) (SU_SemTryWait(&(x)) == 0)
#define SU_SEM_WAIT_TIMEOUT(x,msec) (SU_SemWaitTimeout(&(x),msec) == 0)
#define SU_MUTEX_HANDLE pthread_mutex_t
#define SU_MUTEX_WAIT(x) pthread_mutex_lock(&(x))
#define SU_MUTEX_POST(x) pthread_mutex_unlock(&(x))
#define SU_CRITICAL pthread_mutex_t
#define SU_CRITICAL_ENTER(x) pthread_mutex_lock(&(x))
#define SU_CRITICAL_LEAVE(x) pthread_mutex_unlock(&(x))
#define SU_CRITICAL_TRY_AND_ENTER(x) (pthread_mutex_trylock(&(x)) == 0)
#define SU_THREAD_PRIORITY_NORMAL 0
#define SU_THREAD_PRIORITY_ABOVE_NORMAL 1
#define SU_THREAD_PRIORITY_BELOW_NORMAL -1
#else /* !__unix__ */
#define SU_THREAD_HANDLE HANDLE
#define SU_THREAD_ID unsigned int
#define SU_THREAD_NULL 0
#define SU_THREAD_ROUTINE_TYPE(x) unsigned (__stdcall *x)(void *)
#define SU_THREAD_ROUTINE(x,y) unsigned __stdcall x(void *y)
#define SU_END_THREAD(x) _endthreadex((x))
#define SU_THREAD_RETURN(x) return (unsigned)(x);
#define SU_PROCESS_SELF (SU_u32)GetCurrentProcessId()
#define SU_THREAD_SELF (SU_THREAD_ID)GetCurrentThreadId()
#define SU_THREAD_KEY_HANDLE DWORD
#define SU_THREAD_ONCE_HANDLE DWORD
#define SU_THREAD_ONCE_INIT 0
#define SU_THREAD_ONCE(x,y) y()
#define SU_THREAD_GET_SPECIFIC(x) TlsGetValue(x)
#define SU_THREAD_SET_SPECIFIC(x,y) TlsSetValue(x,(LPVOID)y)
#define SU_THREAD_DESTROY_SPECIFIC(x,y) x(y)
#define SU_SEM_HANDLE HANDLE
#define SU_SEM_WAIT(x) WaitForSingleObject(x,INFINITE)
#define SU_SEM_POST(x) ReleaseSemaphore(x,1,NULL)
#define SU_SEM_TRY_AND_ENTER(x) (SU_SemTryWait(&(x)) == 0)
#define SU_SEM_WAIT_TIMEOUT(x,msec) (SU_SemWaitTimeout(&(x),msec) == 0)
#define SU_MUTEX_HANDLE HANDLE
#define SU_MUTEX_WAIT(x) WaitForSingleObject(x,INFINITE)
#define SU_MUTEX_POST(x) ReleaseMutex(x)
#define SU_CRITICAL CRITICAL_SECTION
#define SU_CRITICAL_ENTER(x) EnterCriticalSection(&(x))
#define SU_CRITICAL_LEAVE(x) LeaveCriticalSection(&(x))
#define SU_CRITICAL_TRY_AND_ENTER(x) (TryEnterCriticalSection(&(x)) != 0)
#define SU_THREAD_PRIORITY_NORMAL THREAD_PRIORITY_NORMAL
#define SU_THREAD_PRIORITY_ABOVE_NORMAL THREAD_PRIORITY_ABOVE_NORMAL
#define SU_THREAD_PRIORITY_BELOW_NORMAL THREAD_PRIORITY_BELOW_NORMAL
#endif /* __unix__ */

/* Create a new thread - Do not detach the thread if you want to use SU_WaitForThread */
SKYUTILS_API bool SU_CreateThread(SU_THREAD_HANDLE *Handle,SU_THREAD_ID *ThreadId,SU_THREAD_ROUTINE_TYPE(Entry),void *User,bool Detached); /* True on success */

/* Set a thread's priority */
SKYUTILS_API bool SU_SetThreadPriority(SU_THREAD_HANDLE Handle,int Priority);

/* Kill the specified thread */
SKYUTILS_API void SU_KillThread(SU_THREAD_HANDLE Handle);

/* Terminate the specified thread */
SKYUTILS_API void SU_TermThread(SU_THREAD_HANDLE Handle);

/* Suspend a thread */
SKYUTILS_API void SU_SuspendThread(SU_THREAD_HANDLE Handle);

/* Resume a suspended thread */
SKYUTILS_API void SU_ResumeThread(SU_THREAD_HANDLE Handle);

/* Waits for a thread to complete, returning its exit code. Thread must not have been created 'detached' */
SKYUTILS_API void *SU_WaitForThread(SU_THREAD_HANDLE Handle);

/* Create a new semaphore (InitialCount: Taken=0 NotTaken=1) - SemName must be unique (or NULL) */
SKYUTILS_API bool SU_CreateSem(SU_SEM_HANDLE *Handle,int InitialCount,int MaximumCount,const char SemName[]); /* True on success */

/* Free a semaphore */
SKYUTILS_API bool SU_FreeSem(SU_SEM_HANDLE *Handle);

/* Try to lock a semaphore - Returns 0 if sem was available and has been taken, -1 otherwise (if it was already taken) */
SKYUTILS_API int SU_SemTryWait(SU_SEM_HANDLE *sem);

/* Wait for a semaphore for a maximum of 'msec' milliseconds (a negative value will never timeout) - Returns 0 if sem was available and has been taken, -1 if timed out */
SKYUTILS_API int SU_SemWaitTimeout(SU_SEM_HANDLE *sem,int msec);

/* Create a new thread key */
SKYUTILS_API bool SU_CreateThreadKey(SU_THREAD_KEY_HANDLE *Handle,SU_THREAD_ONCE_HANDLE *Once,void (*destroyts)(void *)); /* True on success */

/* Block all signals for the calling thread */
SKYUTILS_API void SU_ThreadBlockSigs(void);

/* Create a new mutex */
SKYUTILS_API bool SU_CreateMutex(SU_MUTEX_HANDLE *Handle,const char MutexName[]); /* True on success */

/* Free a mutex */
SKYUTILS_API bool SU_FreeMutex(SU_MUTEX_HANDLE *Handle);

/* Create a critical section */
#define SU_CRITICAL_TYPE_ANY           1
#define SU_CRITICAL_TYPE_RECURSIVE     2
#define SU_CRITICAL_TYPE_NON_RECURSIVE 3
SKYUTILS_API bool SU_CriticalInit(SU_CRITICAL *Crit,int Type); /* True on success */

/* Free a critical section */
SKYUTILS_API bool SU_CriticalDelete(SU_CRITICAL *Crit);

#endif /* !SU_INCLUDE_NO_THREAD */


/* **************************************** */
/*           dynamic load functions         */
/* **************************************** */
#ifdef SU_USE_DL
#ifdef __unix__
#include <dlfcn.h>
#include <signal.h>
#define SU_DL_HANDLE void *
#define SU_DL_OPEN(x) dlopen(x,RTLD_LAZY)
#define SU_DL_CLOSE(x) dlclose(x)
#define SU_DL_SYM(x,y) dlsym(x,y)
#else /* !__unix__ */
#define SU_DL_HANDLE HMODULE
#define SU_DL_OPEN(x) LoadLibrary(x)
#define SU_DL_CLOSE(x) FreeLibrary(x)
#define SU_DL_SYM(x,y) GetProcAddress(x,y)
#endif /* __unix__ */
/* Loads the symbol "Name" from handle, but if not found, try with "_Name" */
SKYUTILS_API void *SU_DL_GetSym(SU_DL_HANDLE handle, const char Name[]);

#endif /* SU_USE_DL */


/* **************************************** */
/*             Archive  functions           */
/* **************************************** */
#ifdef SU_USE_ARCH

#define SU_ARCH_COMP_NONE 1
#define SU_ARCH_COMP_Z    2
#define SU_ARCH_COMP_BZ   4
#define SU_ARCH_COMP_LZO  8

typedef struct
{
  void *Data;        /* Resource data ('Size' bytes of data in this buffer) */
  SU_u32 Size;       /* Size of the data pointer                            */
  SU_u32 Stamp;      /* Time stamp of the original resource                 */

  SU_u32 Index;      /* Resource Index (any,unique)                         */
  char *Name;        /* Resource Name (NULL if none)                        */
} SU_TRes, *SU_PRes;

struct SU_SArch;
typedef struct SU_SArch SU_TArch, *SU_PArch;

typedef unsigned int SU_AR_COMP_TYPE;

/* *** Reading functions *** */
/* Opens a skyutils archive file (or a binary file [exe/dll] if the archive is selfcontained) */
SKYUTILS_API SU_PArch SU_AR_OpenArchive(const char FileName[]);

/* Reads resource ResNum (0 is the first one) (NULL if failed) */
SKYUTILS_API SU_PRes SU_AR_ReadRes(SU_PArch Arch,const SU_u32 ResNum,bool GetData);

/* Reads resource of Index ResIndex (Index is unique, but can be any value except 0) (NULL if failed) */
SKYUTILS_API SU_PRes SU_AR_ReadResIndex(SU_PArch Arch,const SU_u32 ResIndex,bool GetData);

/* Reads resource named ResName (NULL if failed) */
SKYUTILS_API SU_PRes SU_AR_ReadResName(SU_PArch Arch,const char *ResName,bool GetData);

/* Saves resource ResNum to FileName (0 is the first one) (true on success) */
SKYUTILS_API bool SU_AR_ReadResToFile(SU_PArch Arch,const SU_u32 ResNum,const char FileName[]);

/* Saves resource ResIndex to FileName (Index is unique, but can be any value except 0) (true on success) */
SKYUTILS_API bool SU_AR_ReadResIndexToFile(SU_PArch Arch,const SU_u32 ResIndex,const char FileName[]);

/* Saves resource named ResName to FileName (true on success) */
SKYUTILS_API bool SU_AR_ReadResNameToFile(SU_PArch Arch,const char *ResName,const char FileName[]);

/* *** Writing functions *** */
/* Creates a new archive file. FileName can't be NULL */
SKYUTILS_API SU_PArch SU_AR_CreateArchive(const char FileName[]);

/* Adds a resource to the archive (Data can be freed upon return) (Index must be unique, or 0) (Name must be unique, or NULL) (true on success) */
SKYUTILS_API bool SU_AR_AddRes(SU_PArch Arch,void *Data,SU_u32 Size,time_t Time,SU_AR_COMP_TYPE Type,SU_u32 Index,const char *Name);

/* Adds a file resource to the archive (Index must be unique, or 0) (Name must be unique, or NULL) (true on success) */
SKYUTILS_API bool SU_AR_AddResFile(SU_PArch Arch,const char FileName[],SU_AR_COMP_TYPE Type,SU_u32 Index,const char *Name);

/* *** Other functions *** */
/* Closes a previous opened/created archive (true on success) */
SKYUTILS_API bool SU_AR_CloseArchive(SU_PArch Arch);

/* Frees a previous returned resource */
SKYUTILS_API void SU_AR_FreeRes(SU_PRes Res);

/* Returns supported compression types (as a bit field) */
SKYUTILS_API SU_AR_COMP_TYPE SU_AR_SupportedComps(void);

/* Returns the number of resources in the archive (0 if error or no resources) */
SKYUTILS_API SU_u32 SU_AR_GetResourcesCount(SU_PArch Arch);

#endif /* SU_USE_ARCH */


/* **************************************** */
/*             Registry functions           */
/* **************************************** */
#ifndef SU_INCLUDE_NO_REG
#ifdef _WIN32
#include <winreg.h>
#else /* !_WIN32 */
#define HKEY void *
#endif /* _WIN32 */

#ifndef KEY_WOW64_64KEY
#define KEY_WOW64_64KEY (0x0100)
#endif /* !KEY_WOW64_64KEY */

#define SU_RB_ERR_SUCCESS            0
#define SU_RB_ERR_ACCESS_DENIED      1
#define SU_RB_ERR_WRONG_TYPE         2
#define SU_RB_ERR_INVALID_KEY        3
#define SU_RB_ERR_INVALID_PATH       4
#define SU_RB_ERR_LOCK_FAILED        5
#define SU_RB_ERR_PREMATURE_EOF      6
#define SU_RB_ERR_WRITE_ERROR        7
#define SU_RB_ERR_INVALID_TYPE       8
#define SU_RB_ERR_CORRUPTED          9
#define SU_RB_ERR_MORE_DATA         10
#define SU_RB_ERR_NO_SUCH_KEY       11
#define SU_RB_ERR_REGISTRY_NOT_OPEN 12

#define SU_RB_MODE_NORMAL          0
#define SU_RB_MODE_FORCE_WOW64_KEY 1

SKYUTILS_API int SU_RB_GetLastError();
SKYUTILS_API void SU_RB_SetRegistry64Mode(int Mode); /* Sets Registry compatibility mode (only affects WinXP/Win2003 64 bits edition, nothing on other OS) */
SKYUTILS_API int SU_RB_GetRegistry64Mode(void);

/* 'Key' parameter must be a succession of subkeys separated by '\', and finished by a keyname. To set/get the default value of a subkey, put a trailing '\' on 'Key' */
SKYUTILS_API bool SU_RB_GetStrValue(const char Key[],char *buf,int buf_len,const char Default[]); /* True on success. Call SU_RB_GetLastError() for error */
SKYUTILS_API bool SU_RB_GetStrLength(const char Key[],int *length); /* True on success. Call SU_RB_GetLastError() for error */
SKYUTILS_API bool SU_RB_GetIntValue(const char Key[],int *Value,int Default); /* True on success. Call SU_RB_GetLastError() for error */
SKYUTILS_API bool SU_RB_GetBinValue(const char Key[],char *buf,int buf_len,char *Default,int def_len); /* True on success. Call SU_RB_GetLastError() for error */
SKYUTILS_API bool SU_RB_GetBinLength(const char Key[],int *length); /* True on success. Call SU_RB_GetLastError() for error */
SKYUTILS_API bool SU_RB_SetStrValue(const char Key[],const char Value[]); /* True on success. Call SU_RB_GetLastError() for error */
SKYUTILS_API bool SU_RB_SetIntValue(const char Key[],int Value); /* True on success. Call SU_RB_GetLastError() for error */
SKYUTILS_API bool SU_RB_SetBinValue(const char Key[],const char *Value,int val_len); /* True on success. Call SU_RB_GetLastError() for error */
SKYUTILS_API bool SU_RB_DelKey(const char Key[]); /* Recursively deletes All sub-keys and values of Key (and Key itself) */ /* True on success. Call SU_RB_GetLastError() for error */
SKYUTILS_API bool SU_RB_DelValue(const char Key[]); /* True on success. Call SU_RB_GetLastError() for error */

SKYUTILS_API bool SU_RB_OpenRegistry(const char RegistryPath[]); /* Opens registry for reading/writing. True on success - Only one registry opened at one time, close previous one if opened */
SKYUTILS_API bool SU_RB_CloseRegistry(void); /* Flush all writings since registry was opened and close it. True on success. If failed, nothing was flushed */

SKYUTILS_API HKEY SU_RB_OpenKeys(const char Key[],int Access); /* Opened HKEY or 0 on error */
SKYUTILS_API HKEY SU_RB_CreateKeys(const char Key[]); /* Created HKEY or 0 on error */
SKYUTILS_API bool SU_RB_EnumKey(HKEY Key,int Idx,char *Name,int name_len); /* True on Success. False when no more values available */
SKYUTILS_API bool SU_RB_EnumStrValue(HKEY Key,int Idx,char *Name,int name_len,char *Value,int value_len); /* True on Success. False when no more values available */ /* All values must be of same type in the HKEY (int or string) */
SKYUTILS_API bool SU_RB_EnumIntValue(HKEY Key,int Idx,char *Name,int name_len,int *Value); /* True on Success. False when no more values available */ /* All values must be of same type in the HKEY (int or string) */
SKYUTILS_API void SU_RB_CloseKey(HKEY Key);
#endif /* !SU_INCLUDE_NO_REG */


/* **************************************** */
/*              Debug  functions            */
/* **************************************** */
#define SU_DBG_OUTPUT_NONE    0
#define SU_DBG_OUTPUT_PRINTF  1
#define SU_DBG_OUTPUT_CONSOLE 2
#define SU_DBG_OUTPUT_FILE    4
#define SU_DBG_OUTPUT_SOCKET  8
#define SU_DBG_OUTPUT_POPUP   16

#define SU_DBG_CONSOLE_DEFAULT_WINDOW_NAME "SkyUtils_Debug_Window"

/* ** Global options */
SKYUTILS_API void SU_DBG_SetFlags(const SU_u64 Flags); /* Effective immediatly */
SKYUTILS_API void SU_DBG_SetOutput(const SU_u16 Output); /* Effective immediatly */
/* If PrintTime is set to true, date/time is also printed. CurrentProcessId/CurrentThreadId is printed if PrintProcessId/PrintThreadId is set to true */
SKYUTILS_API void SU_DBG_SetOptions(const bool PrintTime,const bool PrintProcessId,const bool PrintThreadId); /* Effective immediatly */

/* ** Printf Output Options ** */
/* Ansi color is used if AnsiColor is set to true */
SKYUTILS_API void SU_DBG_OUT_PRINTF_SetOptions(const bool AnsiColor); /* Effective immediatly */

/* ** Console Output Options ** */
/* WindowName is the name of the debug console window. Set it to NULL to stop sending messages to it */
SKYUTILS_API void SU_DBG_OUT_CONSOLE_SetOptions(const char WindowName[]); /* Effective immediatly (changes the Console immediatly) */

/* ** File Output Options ** */
/* FileName is the name of the log file. Set it to NULL to close log file. File is deleted if DeletePreviousLog is set to true */
SKYUTILS_API void SU_DBG_OUT_FILE_SetOptions(const char FileName[],bool DeletePreviousLog); /* Effective immediatly (closes old file if any, opens the new one) */

/* ** Socket Output Options ** */
/* HostName:Port is the Host and port to connect to. Set Port to 0, to remove this host from list of socket */
SKYUTILS_API void SU_DBG_OUT_SOCKET_SetOptions(const char HostName[],const int Port); /* Effective immediatly (adds a new socket) */

/* PrintDebug fonction. \n is added a the end of the string */
SKYUTILS_API void SU_DBG_PrintDebug(const SU_u64 Type,char *Txt, ...);

/* **** Debug Env vars **** */
#define SU_DBG_HELP_MESSAGE "SkyUtils Debug : Environment variables HELP (overrides application init on the first 'SU_DBG_PrintDebug' call) :\n\n" \
"   Global env var : SU_DBG_HELP = Print this help\n" \
"                    SU_DBG_OUTPUT = {printf,console,file,socket,popup} (space separated)\n" \
"                    SU_DBG_FLAGS = <Flags> (Flags is a 64bits bitfield defining which flags to output)\n" \
"                    SU_DBG_OPTIONS = {time,process,thread} (space separated)\n" \
"    printf env var : SU_DBG_OUT_PRINTF = {0|1} (AnsiColor boolean)\n" \
"    console env var : SU_DBG_OUT_CONSOLE = <WindowName>\n" \
"    file env var : SU_DBG_OUT_FILE = {0|1} <FileName> (0|1 is DeletePreviousLog boolean)\n" \
"    socket env var : SU_DBG_OUT_SOCKET = <HostName:Port>[ <HostName:Port>] ...\n" \
"    popup env var : N/A\n" \
"\n"

#ifdef _WIN32
/* Special Windows Config Function (pops up a GUI to choose debug options, overriding application init, and env vars) - Returns true if debug options were changed */
/* hInstance must be the Instance of the module who is linked with the skyutils library (use GetModuleHandle(NULL) to retrieve calling process' HINSTANCE) */
SKYUTILS_API bool SU_DBG_ChooseDebugOptions_Windows(HINSTANCE hInstance,HWND Parent);

#endif /* _WIN32 */


/* **************************************** */
/*              Buffer functions            */
/* **************************************** */

/**
 *  @brief Delegate method to ask for new buffer size.
 *  @details This delegate method is called when the growing buffer needs to be resize, asking for the new size it should be allocated to.
 *  @param [in] currentSize Current buffer size.
 *  @param [in] requestSize Minimum requested new size.
 *  @return The new size the buffer should be resized to.
 */
typedef size_t (SU_BF_REQUEST_NEW_BUFFER_SIZE)(size_t currentSize,size_t requestedSize);

/**
 *  @brief SU_Buffer structure to handle growing buffers for serialization/deserialization
 */
struct SU_SBuffer;
typedef struct SU_SBuffer* SU_PBuffer;

/**
 *  @brief Allocates a SU_Buffer object.
 *  @details This method is used to allocate a new SU_Buffer object without initializing it. Call the @link SU_BF_Init @endlink function to initialize it properly.
 *  @return The newly allocated SU_Buffer object.
 */
SKYUTILS_API SU_PBuffer SU_BF_Alloc(void);

/**
 *  @brief Allocates and initialize a SU_Buffer object.
 *  @details This method is used to allocate a new SU_Buffer object and then initialize it.
 *  @param [in] defaultSize Initial size of the growing buffer.
 *  @param [in] requestNewBufferSize Delegate method to be called when the buffer needs resizing. Default resizing policy will be used if NULL is specified.
 *  @return The newly allocated and initialized SU_Buffer object.
 */
SKYUTILS_API SU_PBuffer SU_BF_Create(size_t defaultSize,SU_BF_REQUEST_NEW_BUFFER_SIZE* requestNewBufferSize);

/**
 *  @brief Initializes a SU_Buffer object.
 *  @details This method is used to properly initialize a SU_Buffer object.
 *  @param [in] buffer The SU_Buffer to initialize.
 *  @param [in] defaultSize Initial size of the growing buffer.
 *  @param [in] requestNewBufferSize Delegate method to be called when the buffer needs resizing. Default resizing policy will be used if NULL is specified.
 */
SKYUTILS_API void SU_BF_Init(SU_PBuffer buffer,size_t defaultSize,SU_BF_REQUEST_NEW_BUFFER_SIZE* requestNewBufferSize);

/**
 *  @brief Frees a SU_Buffer object.
 *  @details This method is used to fully free a SU_Buffer object. The passed buffer must be not accessed anymore nor passed to any SU_BF function after this function has been called.
 *  @param [in] buffer The SU_Buffer to free.
 */
SKYUTILS_API void SU_BF_Free(SU_PBuffer buffer);

/**
 *  @brief Wipes the data contained in the growing buffer.
 *  @details This method is used to empty the data contained in the growing buffer. A new serialization/deserialization can begin after this call.
 *  @param [in] buffer The SU_Buffer to be wiped.
 */
SKYUTILS_API void SU_BF_Empty(SU_PBuffer buffer);

/**
 *  @brief Reserves bytes in the growing buffer.
 *  @details This method is used to reserve bytes in the growing buffer to be written later. The reserved position is returned by the method and can be used later as a parameter to the @link SU_BF_WriteToReservedBytes @endlink method.
 *  @param [in] buffer The SU_Buffer to reserve bytes in.
 *  @param [in] len The number of bytes to reserve.
 *  @return The position of the reserved bytes.
 *  @note The returned position value becomes invalid if @link SU_BF_ConsumeBufferLength @endlink is called before using it in a call to @link SU_BF_WriteToReservedBytes @endlink.
 */
SKYUTILS_API size_t SU_BF_ReserveBytes(SU_PBuffer buffer,size_t len);

/**
 *  @brief Consumes bytes in the growing buffer.
 *  @details This method is used to consume a number of bytes at the begining of the growing buffer. Those bytes are removed from the buffer and the remaining number of bytes in the buffer is returned by the method.
 *  @param [in] buffer The SU_Buffer to consume data from.
 *  @param [in] len The number of bytes at the begining of the buffer to consume.
 *  @return The remaining number of bytes in the growing buffer.
 */
SKYUTILS_API size_t SU_BF_ConsumeBufferLength(SU_PBuffer buffer,size_t len);

/**
 *  @brief Adds bytes in the growing buffer.
 *  @details This method is used to write a number of bytes at the end of the growing buffer. If there is not enough room in the buffer to hold the bytes, it will be reallocated according to the configured policy.
 *  @param [in] buffer The SU_Buffer to add bytes to.
 *  @param [in] data The pointer to the data to add from.
 *  @param [in] len The number of bytes to add.
 */
SKYUTILS_API void SU_BF_AddToBuffer(SU_PBuffer buffer,void* data,size_t len);

/**
 *  @brief Writes bytes to a previously reserved area.
 *  @details This method is used to write a number of bytes to a previously reserved area in the growing buffer. Use the @link SU_BF_ReserveBytes @endlink method to reserve an area in the buffer and to get its position.
 *  @param [in] buffer The SU_Buffer to write bytes to.
 *  @param [in] position The previously reserved area position.
 *  @param [in] data The pointer to the data to write from.
 *  @param [in] len The number of bytes to write.
 */
SKYUTILS_API void SU_BF_WriteToReservedBytes(SU_PBuffer buffer,size_t position,void* data,size_t len);

/**
 *  @brief Gets a read-only pointer to the growing buffer data.
 *  @details This method is used to retrieve a read-only pointer to the whole growing buffer data.
 *  @param [in] buffer The SU_Buffer to get a pointer from.
 *  @return The pointer to the data.
 */
SKYUTILS_API const void* SU_BF_GetBufferData(SU_PBuffer buffer);

/**
 *  @brief Gets the number of bytes in the growing buffer data.
 *  @details This method is used to retrieve the number of bytes in the whole growing buffer data.
 *  @param [in] buffer The SU_Buffer to get the number of bytes from.
 *  @return The number of bytes of data.
 */
SKYUTILS_API size_t SU_BF_GetBufferLength(SU_PBuffer buffer);

/**
 *  @brief Gets the current maximum size the growing buffer.
 *  @details This method is used to retrieve the current maximum size of the growing buffer.
 *  @param [in] buffer The SU_Buffer to get the maximum size from.
 *  @return The maximum size of the buffer.
 */
SKYUTILS_API size_t SU_BF_GetAllocatedLength(SU_PBuffer buffer);

/**
 *  @brief Forces a resize of the growing buffer.
 *  @details This method is used to force a reallocation of the growing buffer. If declared in @link SU_BF_Create @endlink or @link SU_BF_Init @endlink, the @link SU_BF_REQUEST_NEW_BUFFER_SIZE @endling delegate method will be called.
 *  @param [in] buffer The SU_Buffer to force a resize.
 */
SKYUTILS_API void SU_BF_ForceResize(SU_PBuffer buffer);


/* **************************************** */
/*              MISC   functions            */
/* **************************************** */

/* Dummy functions used by configure, to check correct version of skyutils */
/* Remove old ones if compatibility has been broken */
SKYUTILS_API void SU_Dummy401(void);


#if defined(__cplusplus)/* && !defined(__BORLANDC__)*/
}
#endif /* __cplusplus */

#endif /* !__SKY_UTILS_H__ */
