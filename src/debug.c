/****************************************************************/
/* Debug unit                                                   */
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


#include "debug.h"
#include <stdarg.h>
#ifdef _WIN32
#include <sys/timeb.h>
#include <time.h>
#endif
#ifdef __GNUC__
#include <sys/time.h>
#include <time.h>
#endif

#ifndef SU_TRACE_INTERNAL
#undef malloc
#undef calloc
#undef realloc
#undef strdup
#undef free
#endif /* !SU_TRACE_INTERNAL */

/* *** Global Debug Variables *** */
SU_u64 SU_DBG_Flags = 0;
SU_u16 SU_DBG_Output = SU_DBG_OUTPUT_NONE;
bool SU_DBG_OPT_Time = false;
bool SU_DBG_OPT_ThreadId = false;
bool SU_DBG_OPT_ProcessId = false;
bool SU_DBG_InitDone = false;
bool SU_DBG_SockInitDone = false;
SU_DBG_TOutputName SU_DBG_OutputNames[]={{SU_DBG_OUTPUT_PRINTF,"printf"},{SU_DBG_OUTPUT_CONSOLE,"console"},{SU_DBG_OUTPUT_FILE,"file"},{SU_DBG_OUTPUT_SOCKET,"socket"},{SU_DBG_OUTPUT_POPUP,"popup"},{0,NULL}};

/* *** Output Specific variables *** */
  /* Printf output */
  bool SU_DBG_OUT_PRINTF_Color = false;
  /* File output */
  FILE *SU_DBG_OUT_FILE_File = NULL;
  char *SU_DBG_OUT_FILE_FileName = NULL;
  bool SU_DBG_OUT_FILE_DeletePreviousLog = false;
  /* Console output */
#ifdef _WIN32
  HWND SU_DBG_OUT_CONSOLE_Hwnd = 0;
  UINT SU_DBG_OUT_CONSOLE_Msg = 0;
  char *SU_DBG_OUT_CONSOLE_Name = NULL;
#endif /* _WIN32 */
  /* Socket output */
  SU_SOCKET SU_DBG_OUT_SOCKET_Socks[SU_DBG_MAX_SOCKETS] = {SU_NOT_A_SOCKET,SU_NOT_A_SOCKET,SU_NOT_A_SOCKET,SU_NOT_A_SOCKET};

void SU_DBG_Init(void)
{
  char *tmp,*env;
  int i;

  if(SU_DBG_InitDone)
    return;

  env = getenv("SU_DBG_HELP");
  if(env != NULL)
  {
    printf(SU_DBG_HELP_MESSAGE);
  }

  env = getenv("SU_DBG_FLAGS");
  if(env != NULL)
    SU_DBG_Flags = SU_atoi(env);

  env = getenv("SU_DBG_OPTIONS");
  if(env != NULL)
  {
    char *env_dup = SU_strdup(env);

    /* Reset all options */
    SU_DBG_OPT_Time = false;
    SU_DBG_OPT_ThreadId = false;
    SU_DBG_OPT_ProcessId = false;

    tmp = strtok(env_dup," ");
    while(tmp != NULL)
    {
      if(SU_strcasecmp(tmp,"time"))
      {
        SU_DBG_OPT_Time = true;
      }
      else if(SU_strcasecmp(tmp,"thread"))
      {
        SU_DBG_OPT_ThreadId = true;
      }
      else if(SU_strcasecmp(tmp,"process"))
      {
        SU_DBG_OPT_ProcessId = true;
      }
      else
        printf("SU_DBG_Init : Unknown OPTION name SU_DBG_OPTIONS env var : %s\n",tmp);
      tmp = strtok(NULL," ");
    }
    free(env_dup);
  }

  env = getenv("SU_DBG_OUTPUT");
  if(env != NULL)
  {
    char *env_dup = SU_strdup(env);

    SU_DBG_Output = SU_DBG_OUTPUT_NONE;
    tmp = strtok(env_dup," ");
    while(tmp != NULL)
    {
      bool found = false;
      i = 0;
      while(SU_DBG_OutputNames[i].Name != NULL)
      {
        if(SU_strcasecmp(tmp,SU_DBG_OutputNames[i].Name))
        {
          SetFlag(SU_DBG_Output,SU_DBG_OutputNames[i].Output);
          found = true;
          break;
        }
        i++;
      }
      if(!found)
        printf("SU_DBG_Init : Unknown OUTPUT type in SU_DBG_OUTPUT env var : %s\n",tmp);
      tmp = strtok(NULL," ");
    }
    free(env_dup);

    if(SU_DBG_Output & SU_DBG_OUTPUT_PRINTF)
    {
      env = getenv("SU_DBG_OUT_PRINTF");
      if(env != NULL)
      {
        SU_DBG_OUT_PRINTF_SetOptions((bool)atoi(env));
      }
    }
    if(SU_DBG_Output & SU_DBG_OUTPUT_CONSOLE)
    {
#ifdef _WIN32
      env = getenv("SU_DBG_OUT_CONSOLE");
      if(env != NULL)
      {
        SU_DBG_OUT_CONSOLE_SetOptions(env);
      }
      else
        SU_DBG_OUT_CONSOLE_SetOptions("");
#else /* !_WIN32 */
      printf("SU_DBG_Init : Console output is only supported on pure WIN32 applications\n");
      ClearFlag(SU_DBG_Output,SU_DBG_OUTPUT_CONSOLE);
#endif /* _WIN32 */
    }
    if(SU_DBG_Output & SU_DBG_OUTPUT_FILE)
    {
      env = getenv("SU_DBG_OUT_FILE");
      if(env != NULL)
      {
        char *t1,*t2;

        env_dup = SU_strdup(env);
        t1 = strtok(env_dup," ");
        t2 = strtok(NULL," ");
        if((t1 != NULL) && (t2 != NULL))
          SU_DBG_OUT_FILE_SetOptions(t2,(bool)atoi(t1));
        free(env_dup);
      }
    }
    if(SU_DBG_Output & SU_DBG_OUTPUT_SOCKET)
    {
      env = getenv("SU_DBG_OUT_SOCKET");
      if(env != NULL)
      {
        char *env_dup = SU_strdup(env);

        tmp = strtok(env_dup," ");
        while(tmp != NULL)
        {
          char *t;
          /* tmp = host:port */
          t = strchr(tmp,':');
          if(t != NULL)
          {
            t[0] = 0;
            t++;
            SU_DBG_OUT_SOCKET_SetOptions(tmp,SU_atoi(t));
          }
          tmp = strtok(NULL," ");
        }
        free(env_dup);
      }
    }
  }
}

SKYUTILS_API void SU_DBG_SetFlags(const SU_u64 Flags)
{
  SU_DBG_Flags = Flags;
}

SKYUTILS_API void SU_DBG_SetOutput(const SU_u16 Output)
{
  SU_DBG_Output = Output;
}

SKYUTILS_API void SU_DBG_SetOptions(const bool PrintTime,const bool PrintProcessId,const bool PrintThreadId)
{
  SU_DBG_OPT_Time = PrintTime;
  SU_DBG_OPT_ThreadId = PrintThreadId;
  SU_DBG_OPT_ProcessId = PrintProcessId;
}

/* If PrintTime is set to true, date/time is also printed */
SKYUTILS_API void SU_DBG_OUT_PRINTF_SetOptions(const bool AnsiColor)
{
  SU_DBG_OUT_PRINTF_Color = AnsiColor;
}

/* WindowName is the name of the debug console window. Set it to NULL to stop sending messages to it */
SKYUTILS_API void SU_DBG_OUT_CONSOLE_SetOptions(const char WindowName[])
{
#ifdef _WIN32
  if(WindowName == NULL)
  {
    SU_DBG_OUT_CONSOLE_Hwnd = 0;
    return;
  }
  if(SU_DBG_OUT_CONSOLE_Name != NULL)
    free(SU_DBG_OUT_CONSOLE_Name);
  if(WindowName[0] == 0)
    SU_DBG_OUT_CONSOLE_Name = SU_strdup(SU_DBG_CONSOLE_DEFAULT_WINDOW_NAME);
  else
    SU_DBG_OUT_CONSOLE_Name = SU_strdup((char *)WindowName);
  SU_DBG_OUT_CONSOLE_Msg = RegisterWindowMessage(SU_DBG_OUT_CONSOLE_Name);
  SU_DBG_OUT_CONSOLE_Hwnd = FindWindow(NULL,SU_DBG_OUT_CONSOLE_Name);
  if(SU_DBG_OUT_CONSOLE_Hwnd == NULL)
  {
    printf("SU_DBG_OUT_CONSOLE_SetOptions : Cannot find console window with name : %s\n",SU_DBG_OUT_CONSOLE_Name);
  }
#endif /* _WIN32 */
}

/* FileName is the name of the log file. Set it to NULL to close log file. File is deleted if DeletePreviousLog is set to true */
SKYUTILS_API void SU_DBG_OUT_FILE_SetOptions(const char FileName[],bool DeletePreviousLog)
{
  SU_DBG_OUT_FILE_DeletePreviousLog = DeletePreviousLog;
  if((FileName == NULL) || (FileName[0] == 0))
  {
    if(SU_DBG_OUT_FILE_File != NULL)
      SU_CloseLogFile(SU_DBG_OUT_FILE_File);
    SU_DBG_OUT_FILE_File = NULL;
    return;
  }
  if(SU_DBG_OUT_FILE_File != NULL)
    SU_CloseLogFile(SU_DBG_OUT_FILE_File);
  if(SU_DBG_OUT_FILE_FileName != NULL)
    free(SU_DBG_OUT_FILE_FileName);
  if(SU_DBG_OUT_FILE_DeletePreviousLog)
    remove(FileName);
  SU_DBG_OUT_FILE_FileName = SU_strdup(FileName);
  SU_DBG_OUT_FILE_File = SU_OpenLogFile(SU_DBG_OUT_FILE_FileName);
  if(SU_DBG_OUT_FILE_File == NULL)
  {
    printf("SU_DBG_OUT_FILE_SetOptions : Cannot open debug file for writing : %s\n",FileName);
  }
}

/* HostName:Port is the Host and port to connect to. Set Port to 0, to remove this host from list of socket */
SKYUTILS_API void SU_DBG_OUT_SOCKET_SetOptions(const char HostName[],const int Port)
{
  int i;
  char buf[100];
  SU_PClientSocket CS;

  if(!SU_DBG_SockInitDone)
  {
    if(SU_SockInit(2,2))
      SU_DBG_SockInitDone = true;
    else
    {
      printf("SU_DBG_OUT_SOCKET_SetOptions : Failed to initialize WinSocks\n");
      return;
    }
  }
  for(i=0;i<SU_DBG_MAX_SOCKETS;i++)
  {
    if(SU_DBG_OUT_SOCKET_Socks[i] == SU_NOT_A_SOCKET)
    {
      SU_snprintf(buf,sizeof(buf),"%d",Port);
      CS = SU_ClientConnect((char *)HostName,buf,SOCK_STREAM);
      if(CS == NULL)
      {
        printf("SU_DBG_OUT_SOCKET_SetOptions : Cannot connect to %s:%s\n",HostName,buf);
        return;
      }
      SU_DBG_OUT_SOCKET_Socks[i] = CS->sock;
      free(CS);
      return;
    }
  }
  printf("SU_DBG_OUT_SOCKET_SetOptions : No more socket available, increase SU_DBG_MAX_SOCKETS (%d) in skyutils/debug.c\n",SU_DBG_MAX_SOCKETS);
  return;
}

#define MAXCOLORS 8
char *SU_DBG_Colors[MAXCOLORS] = {"0","1","2","3","4","5","6","7"};
char *SU_DBG_GetColorFromFlag(SU_u64 val)
{
  int i = 1;

  val>>=1;
  while(val)
  {
    i++;
    val>>=1;
  }
  if(i >= MAXCOLORS)
    return "8";
  return SU_DBG_Colors[i];
}

SKYUTILS_API void SU_DBG_PrintDebug(const SU_u64 Type,char *Txt, ...)
{
  va_list argptr;
  char Str[8192];
  char TimeStr[300];
  char ProcessStr[100];
  char ThreadStr[100];
  int i;

  if(!SU_DBG_InitDone)
  {
    SU_DBG_Init();
    SU_DBG_InitDone = true;
  }

  if((Type & SU_DBG_Flags) && (SU_DBG_Output != SU_DBG_OUTPUT_NONE))
  {
    if(SU_DBG_OPT_ProcessId)
    {
      SU_snprintf(ProcessStr,sizeof(ProcessStr),"[%8x] ",SU_PROCESS_SELF);
    }
    else
      ProcessStr[0] = 0;
    if(SU_DBG_OPT_ThreadId)
    {
      SU_snprintf(ThreadStr,sizeof(ThreadStr),"[%8x] ",SU_THREAD_SELF);
    }
    else
      ThreadStr[0] = 0;
    if(SU_DBG_OPT_Time)
    {
      struct tm *TM;
      time_t Tim;
      int usec = 0;
#ifdef _WIN32
      struct _timeb _gtodtmp; 
      _ftime(&_gtodtmp); 
      usec = _gtodtmp.millitm;
#else
      struct timeval td;
      gettimeofday(&td,NULL);
      usec = td.tv_usec / 1000;
#endif

      Tim = time(NULL);
      TM = localtime(&Tim);
      SU_snprintf(TimeStr,sizeof(TimeStr),"[%.2d:%.2d:%.2d.%.3d] ",TM->tm_hour,TM->tm_min,TM->tm_sec,usec);
    }
    else
      TimeStr[0] = 0;

    va_start(argptr,Txt);
#ifdef _WIN32
    _vsnprintf(Str,sizeof(Str),Txt,argptr);
#else /* !_WIN32 */
    vsnprintf(Str,sizeof(Str),Txt,argptr);
#endif /* _WIN32 */
    va_end(argptr);
    if(SU_DBG_Output & SU_DBG_OUTPUT_PRINTF)
    {
#ifdef SU_ENABLE_ANSI_CODE
      if(SU_DBG_OUT_PRINTF_Color)
        printf("%s%s%s\033[3%s;4%sm""%s\n"SU_ANSI_RESET,ProcessStr,ThreadStr,TimeStr,SU_DBG_GetColorFromFlag(Type),SU_ANSI_BLACK,Str);
      else
#endif /* SU_ENABLE_ANSI_CODE */
        printf("%s%s%s%s\n",ProcessStr,ThreadStr,TimeStr,Str);
      fflush(stdout);
    }
#ifdef _WIN32
    if(SU_DBG_Output & SU_DBG_OUTPUT_CONSOLE)
    {
      static int __RetryCount = 0;
      if(SU_DBG_OUT_CONSOLE_Hwnd == 0)
      {
        if(__RetryCount >= 50)
        {
          SU_DBG_OUT_CONSOLE_Hwnd = FindWindow(NULL,SU_DBG_OUT_CONSOLE_Name);
          __RetryCount = 0;
        }
        else
          __RetryCount++;
      }
      if(SU_DBG_OUT_CONSOLE_Hwnd != 0)
      {
        ATOM atom;
        char Str2[255]; /* 255 is the max size of strings in GlobalAddAtom */

        SU_snprintf(Str2,sizeof(Str2),"%s%s%s%s",ProcessStr,ThreadStr,TimeStr,Str);
        atom = GlobalAddAtom(Str2);
        if(atom != 0)
        {
          /*PostMessage(SU_DBG_OUT_CONSOLE_Hwnd,SU_DBG_OUT_CONSOLE_Msg,atom,(LPARAM)Type);*/
          SendMessage(SU_DBG_OUT_CONSOLE_Hwnd,SU_DBG_OUT_CONSOLE_Msg,atom,(LPARAM)Type);
        }
        else
        {
          int err = SU_errno;
          printf("SU_DBG_PrintDebug : Failed to create GlobalAddAtom (%d:%s)",err,SU_strerror(err));
#ifdef _WIN32
          {
            char buf[512];
            SU_snprintf(buf,sizeof(buf),"SU_DBG_PrintDebug : Failed to create GlobalAddAtom (%d:%s)",err,SU_strerror(err));
            MessageBox(NULL,buf,"SU_DBG Error",MB_OK);
          }
#endif /* _WIN32 */
        }
      }
    }
#endif /* _WIN32 */
    if(SU_DBG_Output & SU_DBG_OUTPUT_FILE)
    {
      char Str2[8500];

      SU_snprintf(Str2,sizeof(Str2),"%s%s%s",ProcessStr,ThreadStr,Str);
      SU_WriteToLogFile(SU_DBG_OUT_FILE_File,Str2);
    }
    if(SU_DBG_Output & SU_DBG_OUTPUT_SOCKET)
    {
      int res,sent = 0;
      fd_set wfds;
      struct timeval tv;
      SU_u32 len;
      char Str2[8500];

      SU_snprintf(Str2,sizeof(Str2),"%s%s%s%s",ProcessStr,ThreadStr,TimeStr,Str);

      len = strlen(Str2);
      for(i=0;i<SU_DBG_MAX_SOCKETS;i++)
      {
        if(SU_DBG_OUT_SOCKET_Socks[i] != SU_NOT_A_SOCKET)
        {
          FD_ZERO(&wfds);
          FD_SET(SU_DBG_OUT_SOCKET_Socks[i],&wfds);
          tv.tv_sec = 10;
          tv.tv_usec = 0;
          res = select(SU_DBG_OUT_SOCKET_Socks[i]+1,NULL,&wfds,NULL,&tv);
          if(!res)
          {
            printf("SU_DBG_PrintDebug : Timed out while sending debug message size, closing link\n");
            SU_CLOSE_SOCKET(SU_DBG_OUT_SOCKET_Socks[i]);
            SU_DBG_OUT_SOCKET_Socks[i] = SU_NOT_A_SOCKET;
            continue;
          }
          /* Sending Size of message */
          sent = send(SU_DBG_OUT_SOCKET_Socks[i],(const char *)&len,sizeof(len),SU_MSG_NOSIGNAL);
          if(sent != sizeof(len))
          {
            int err = SU_errno;
            printf("SU_DBG_PrintDebug : Error sending debug message size, closing link (%d:%s)\n",err,SU_strerror(err));
#ifdef _WIN32
            {
              char buf[512];
              SU_snprintf(buf,sizeof(buf),"SU_DBG_PrintDebug : Error sending debug message size, closing link (%d:%s)\n",err,SU_strerror(err));
              MessageBox(NULL,buf,"SU_DBG Error",MB_OK);
            }
#endif /* _WIN32 */
            SU_CLOSE_SOCKET(SU_DBG_OUT_SOCKET_Socks[i]);
            SU_DBG_OUT_SOCKET_Socks[i] = SU_NOT_A_SOCKET;
            continue;
          }
          /* Sending Type of message */
          FD_ZERO(&wfds);
          FD_SET(SU_DBG_OUT_SOCKET_Socks[i],&wfds);
          tv.tv_sec = 10;
          tv.tv_usec = 0;
          res = select(SU_DBG_OUT_SOCKET_Socks[i]+1,NULL,&wfds,NULL,&tv);
          if(!res)
          {
            printf("SU_DBG_PrintDebug : Timed out while sending debug type size, closing link\n");
            SU_CLOSE_SOCKET(SU_DBG_OUT_SOCKET_Socks[i]);
            SU_DBG_OUT_SOCKET_Socks[i] = SU_NOT_A_SOCKET;
            continue;
          }
          /* Sending Size of message */
          sent = send(SU_DBG_OUT_SOCKET_Socks[i],(const char *)&Type,sizeof(Type),SU_MSG_NOSIGNAL);
          if(sent != sizeof(Type))
          {
            int err = SU_errno;
            printf("SU_DBG_PrintDebug : Error sending debug type size, closing link (%d:%s)\n",err,SU_strerror(err));
#ifdef _WIN32
            {
              char buf[512];
              SU_snprintf(buf,sizeof(buf),"SU_DBG_PrintDebug : Error sending debug type size, closing link (%d:%s)\n",err,SU_strerror(err));
              MessageBox(NULL,buf,"SU_DBG Error",MB_OK);
            }
#endif /* _WIN32 */
            SU_CLOSE_SOCKET(SU_DBG_OUT_SOCKET_Socks[i]);
            SU_DBG_OUT_SOCKET_Socks[i] = SU_NOT_A_SOCKET;
            continue;
          }
          /* Sending Message itself */
          FD_ZERO(&wfds);
          FD_SET(SU_DBG_OUT_SOCKET_Socks[i],&wfds);
          tv.tv_sec = 10;
          tv.tv_usec = 0;
          res = select(SU_DBG_OUT_SOCKET_Socks[i]+1,NULL,&wfds,NULL,&tv);
          if(!res)
          {
            printf("SU_DBG_PrintDebug : Timed out while sending debug message, closing link\n");
            SU_CLOSE_SOCKET(SU_DBG_OUT_SOCKET_Socks[i]);
            SU_DBG_OUT_SOCKET_Socks[i] = SU_NOT_A_SOCKET;
            continue;
          }
          /* Sending Size of message */
          sent = send(SU_DBG_OUT_SOCKET_Socks[i],Str2,(int)len,SU_MSG_NOSIGNAL);
          if(sent != (signed)len)
          {
            int err = SU_errno;
            printf("SU_DBG_PrintDebug : Error sending debug message, closing link (%d:%s)\n",err,SU_strerror(err));
#ifdef _WIN32
            {
              char buf[512];
              SU_snprintf(buf,sizeof(buf),"SU_DBG_PrintDebug : Error sending debug message, closing link (%d:%s)\n",err,SU_strerror(err));
              MessageBox(NULL,buf,"SU_DBG Error",MB_OK);
            }
#endif /* _WIN32 */
            SU_CLOSE_SOCKET(SU_DBG_OUT_SOCKET_Socks[i]);
            SU_DBG_OUT_SOCKET_Socks[i] = SU_NOT_A_SOCKET;
            continue;
          }
        }
      }
    }
#ifdef _WIN32
    if(SU_DBG_Output & SU_DBG_OUTPUT_POPUP)
    {
      char Str2[8500];

      SU_snprintf(Str2,sizeof(Str2),"%s%s%s%s",ProcessStr,ThreadStr,TimeStr,Str);
      MessageBox(GetTopWindow(NULL),Str2,"Skyutils debug",MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
    }
#endif /* _WIN32 */
  }
}



void OXDO_Printf_Console(const SU_u64 Type, char *Txt, ...)
{

#ifdef _WIN32
	va_list argptr;
		char Str[8192];

		//return;
    va_start(argptr,Txt);
#ifdef _WIN32
    _vsnprintf(Str,sizeof(Str),Txt,argptr);
#else /* !_WIN32 */
    vsnprintf(Str,sizeof(Str),Txt,argptr);
#endif /* _WIN32 */
    va_end(argptr);
	if(SU_DBG_Output & SU_DBG_OUTPUT_CONSOLE)
    {
      static int __RetryCount = 0;
      if(SU_DBG_OUT_CONSOLE_Hwnd == 0)
      {
        if(__RetryCount >= 50)
        {
#ifdef _WIN32
          SU_DBG_OUT_CONSOLE_Hwnd = FindWindow(NULL,SU_DBG_OUT_CONSOLE_Name);
#endif
          __RetryCount = 0;
        }
        else
          __RetryCount++;
      }
      if(SU_DBG_OUT_CONSOLE_Hwnd != 0)
      {
        ATOM atom;
        char Str2[255]; /* 255 is the max size of strings in GlobalAddAtom */

        SU_snprintf(Str2,sizeof(Str2),"%s%",Str);
        atom = GlobalAddAtom(Str2);
        if(atom != 0)
        {
          /*PostMessage(SU_DBG_OUT_CONSOLE_Hwnd,SU_DBG_OUT_CONSOLE_Msg,atom,(LPARAM)Type);*/
          SendMessage(SU_DBG_OUT_CONSOLE_Hwnd,SU_DBG_OUT_CONSOLE_Msg,atom,(LPARAM)Type);
        }
        else
        {
          int err = SU_errno;
          printf("SU_DBG_PrintDebug : Failed to create GlobalAddAtom (%d:%s)",err,SU_strerror(err));
#ifdef _WIN32
          {
            char buf[512];
            SU_snprintf(buf,sizeof(buf),"SU_DBG_PrintDebug : Failed to create GlobalAddAtom (%d:%s)",err,SU_strerror(err));
            MessageBox(NULL,buf,"SU_DBG Error",MB_OK);
          }
#endif /* _WIN32 */
        }
      }
    }
#endif
}
