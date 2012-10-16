/****************************************************************/
/* Web unit                                                     */
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


/* TO DO :
 * Purger les cookies expirés
*/

#include "skyutils.h"

#ifdef _WIN32
#pragma warning( disable: 4100 4127)
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

#define SW_DEFAULT_USER_AGENT "Mozilla/6.0 (compatible; MSIE 5.01; Windows NT)"
#define SW_DEFAULT_HEADER "Connection: Keep-Alive" "\x0D" "\x0A" "Accept-Language: fr-FR, en" "\x0D" "\x0A" "Accept-Charset: iso-8859-1,*,utf-8" "\x0D" "\x0A" "Accept: text/html, text/plain, text/*, image/gif, image/jpg, image/png, */*" "\x0D" "\x0A"
#define DEFAULT_PORT 80
#define DEFAULT_SSL_PORT 443
#define SOCKET_TIME_OUT 60

#ifdef __unix__
extern int SU_DebugLevel;
#endif /* __unix__ */

char *SW_GetInput_String;
char *SW_GetImage_String;
char *SW_Proxy_String = NULL;
char *SW_Proxy_User = NULL;
char *SW_Proxy_Password = NULL;
char *SW_UserHeader = NULL;
char *SW_UserAgent = NULL;
int   SW_Proxy_Port = 0;
int   SW_SocketTimeout = SOCKET_TIME_OUT;
SKYUTILS_API SU_PList SW_Cookies = NULL; /* SU_PCookie */

int SU_Dump_PageNum = 0;

void DumpPage(const char fname[],const char *buf,const int size)
{
  FILE *fp;
  char FN[50];

  if(fname == NULL)
  {
    SU_snprintf(FN,sizeof(FN),"Dump%d.html",SU_Dump_PageNum++);
    printf("SkyUtils_DumpPage : Dumping to %s\n",FN);
    fp = fopen(FN,"wt");
  }
  else
    fp = fopen(fname,"wb");
  if(fp == NULL)
    return;
#ifdef __unix__
  /* this debug view is useful if u need debug dynamic input name and need to know
   * what page is taken from server */
  if(SU_DebugLevel >= 10)
    fwrite(buf,size,1,stdout);
#endif /* __unix__ */
  fwrite(buf,size,1,fp);
  fclose(fp);
}

/* ** SSL CODE ** */
#ifdef SU_USE_SSL
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#define SSL_RETRY_LIMIT 20
static SSL_CTX *SU_SSL_ctx = NULL;

void SU_SSL_Init()
{
  unsigned char Seed[1024];
  SU_u32 i,j,pid;
  SSL_load_error_strings();
  SSL_library_init();

#ifdef _WIN32
  pid = (SU_u32) GetTickCount() ^ (SU_u32) time(NULL);
#else
  pid = (((SU_u32) getpid()) << 16) ^ (SU_u32) time(NULL);
#endif

  for(i=0;i<sizeof(Seed);i++)
  {
    for(j=0,Seed[i]=0;j<8;j++)
    {
      Seed[i] |= (pid & 1) << j;
      pid = ((((pid >> 7) ^ (pid >> 6) ^ (pid >> 2) ^ (pid >> 0)) & 1) << 31) | (pid >> 1);
    }
  }

  RAND_seed(Seed,sizeof(Seed));
}

SSL_CTX *SU_SSL_InitializeCTX(char *pcError)
{
  char errormsg[1024];
  SSL_CTX *sslctx;

  sslctx = SSL_CTX_new(SSLv3_client_method());
  if(sslctx == NULL)
  {
    ERR_error_string(ERR_get_error(), errormsg);
    SU_snprintf(pcError, 1024, "SSL_CTX_new(): %s", errormsg);
    return NULL;
  }

  SSL_CTX_set_verify(sslctx, SSL_VERIFY_NONE, NULL);

  return sslctx;
}

SSL *SU_SSL_Create(SU_SOCKET Sock,char *pcError)
{
  char errormsg[1024];
  int iError;
  SSL *ssl;

  if(!SU_SSL_ctx)
  {
    SU_SSL_Init();
    SU_SSL_ctx = SU_SSL_InitializeCTX(pcError);
    if(SU_SSL_ctx == NULL)
      return NULL;
    SSL_CTX_set_options(SU_SSL_ctx, SSL_OP_ALL);
    SSL_CTX_set_default_verify_paths(SU_SSL_ctx);
  }

  ssl = SSL_new(SU_SSL_ctx);
  if(ssl == NULL)
  {
    ERR_error_string(ERR_get_error(),errormsg);
    SU_snprintf(pcError, 1024, "SSL_new(): %s", errormsg);
    return NULL;
  }

  iError = SSL_set_fd(ssl,Sock);
  if(iError == 0)
  {
    ERR_error_string(ERR_get_error(), errormsg);
    SU_snprintf(pcError, 1024, "SSL_connect(): %s", errormsg);
    SSL_free(ssl);
    return NULL;
  }
  return ssl;
}

SSL *SU_SSL_Connect(SU_SOCKET Sock,char *pcError)
{
  int iError;
  char errormsg[1024];

  SSL *ssl = SU_SSL_Create(Sock,pcError);
  if(ssl == NULL)
    return NULL;

  iError = SSL_connect(ssl);
  if(iError <= 0)
  {
    ERR_error_string(ERR_get_error(), errormsg);
    SU_snprintf(pcError, 1024, "SSL_connect(): %s", errormsg);
    SSL_free(ssl);
    return NULL;
  }

  return ssl;
}

int SU_SSL_Write(SSL *ssl, char *pcData, int iLength, char *pcError)
{
  char errormsg[1024];
  int iNSent;
  int iOffset;
  int iToSend;
  int iWRetries;

  iToSend = iLength;
  iOffset = iWRetries = 0;
  do
  {
    iNSent = SSL_write(ssl, &pcData[iOffset], iToSend);

    switch (SSL_get_error(ssl, iNSent))
    {
    case SSL_ERROR_NONE:
      iToSend -= iNSent;
      iOffset += iNSent;
      break;

    case SSL_ERROR_SSL:
      if(pcError)
        SU_snprintf(pcError, 1024, "SSL_write(): SSL_ERROR_SSL");
      return -1;
      break;

    case SSL_ERROR_WANT_READ:
      if(pcError)
        SU_snprintf(pcError, 1024, "SSL_write(): SSL_ERROR_WANT_READ");
      return -1;
      break;

    case SSL_ERROR_WANT_WRITE:
      if (++iWRetries >= SSL_RETRY_LIMIT)
      {
        if(pcError)
          SU_snprintf(pcError, 1024, "SSL_write(): SSL_ERROR_WANT_WRITE: Retry limit reached!");
        return -1;
      }
      break;

    case SSL_ERROR_WANT_X509_LOOKUP:
      if(pcError)
        SU_snprintf(pcError, 1024, "SSL_write(): SSL_ERROR_WANT_X509_LOOKUP");
      return -1;
      break;

    case SSL_ERROR_SYSCALL:
      if (ERR_peek_error())
      {
        ERR_error_string(ERR_get_error(), errormsg);
      }
      if (iNSent == -1)
      {
        if(pcError)
          SU_snprintf(pcError, 1024, "SSL_write(): SSL_ERROR_SYSCALL: Underlying I/O error: %s",strerror(errno));
      }
      else
      {
        if(pcError)
          SU_snprintf(pcError, 1024, "SSL_write(): SSL_ERROR_SYSCALL: Unexpected EOF.");
      }
      return -1;
      break;

    case SSL_ERROR_ZERO_RETURN:
      if(pcError)
        SU_snprintf(pcError, 1024, "SSL_write(): SSL_ERROR_ZERO_RETURN: The SSL connection has been closed.");
      return -1;
      break;

    default:
      if(pcError)
        SU_snprintf(pcError, 1024, "SSL_write(): Undefined error.");
      return -1;
      break;
    }
  } while (iToSend > 0);

  return iOffset;
}

int SU_SSL_Read(SSL *ssl, char *pcData, int iLength, char *pcError)
{
  char errormsg[1024];
  int iDone;
  int iNRead;
  int iRRetries;

  iDone = iRRetries = 0;
  iNRead = SSL_read(ssl, pcData, iLength);
  while (iDone == 0)
  {
    iDone = 1;
    switch (SSL_get_error(ssl, iNRead))
    {
    case SSL_ERROR_NONE:
      return iNRead;
      break;

    case SSL_ERROR_SSL:
      ERR_error_string(ERR_get_error(), errormsg);
      if(pcError)
        SU_snprintf(pcError, 1024, "SSL_read(): SSL_ERROR_SSL: %s",errormsg);
      return -1;
      break;

    case SSL_ERROR_WANT_READ:
      if (++iRRetries < SSL_RETRY_LIMIT)
      {
        if(pcError)
          SU_snprintf(pcError, 1024, "SSL_read(): SSL_ERROR_WANT_READ");
        iNRead = SSL_read(ssl, pcData, iLength);
        iDone = 0;
      }
      else
      {
        if(pcError)
          SU_snprintf(pcError, 1024, "SSL_read(): SSL_ERROR_WANT_READ: Retry limit reached!");
        return -1;
      }
      break;

    case SSL_ERROR_WANT_WRITE:
      if(pcError)
        SU_snprintf(pcError, 1024, "SSL_read(): SSL_ERROR_WANT_WRITE");
      return -1;
      break;

    case SSL_ERROR_WANT_X509_LOOKUP:
      if(pcError)
        SU_snprintf(pcError, 1024, "SSL_read(): SSL_ERROR_WANT_X509_LOOKUP");
      return -1;
      break;

    case SSL_ERROR_SYSCALL:
      if (ERR_peek_error())
      {
        ERR_error_string(ERR_get_error(), errormsg);
      }
      if (iNRead == -1)
      {
        if(pcError)
          SU_snprintf(pcError, 1024, "SSL_read(): SSL_ERROR_SYSCALL: Underlying I/O error: %s", strerror(errno));
        return -1;
      }
      else
      {
        if(pcError)
          SU_snprintf(pcError, 1024, "SSL_read(): SSL_ERROR_SYSCALL: Unexpected EOF. (%d)",iNRead);
        return -1;
      }
      break;

    case SSL_ERROR_ZERO_RETURN:
      if(pcError)
        SU_snprintf(pcError, 1024, "SSL_read(): SSL_ERROR_ZERO_RETURN: The SSL connection has been closed.");
      return 0;
      break;

    default:
      if(pcError)
        SU_snprintf(pcError, 1024, "SSL_read(): Undefined error.");
      return -1;
      break;
    }
  }
  return -1;
}

void SU_SSL_SessionCleanup(SSL *ssl)
{
  if(ssl != NULL)
  {
    SSL_shutdown(ssl);
    SSL_free(ssl);
  }
}
/* ** END OF SSL CODE ** */
#else /* !SU_USE_SSL */
typedef void SSL;
#endif /* SU_USE_SSL */

SKYUTILS_API void SU_SetSocketTimeout(const int Timeout)
{
  if(Timeout == 0)
    SW_SocketTimeout = SOCKET_TIME_OUT;
  else
    SW_SocketTimeout = Timeout;
}

SKYUTILS_API void SU_SetProxy(const char Proxy[],const int Port,const char User[], const char Password[])
{
  if(SW_Proxy_String != NULL)
    free(SW_Proxy_String);
  if((Proxy != NULL)&&(strlen(Proxy)>0))
    SW_Proxy_String = SU_strdup(Proxy);
  else
    SW_Proxy_String = NULL;
  SW_Proxy_Port = Port;
  if(SW_Proxy_User != NULL)
    free(SW_Proxy_User);
  if((User != NULL)&&(strlen(User)>0))
    SW_Proxy_User = SU_strdup(User);
  else
    SW_Proxy_User = NULL;
  if(SW_Proxy_Password != NULL)
    free(SW_Proxy_Password);
  if((Password != NULL)&&(strlen(Password)>0))
    SW_Proxy_Password = SU_strdup(Password);
  else
    SW_Proxy_Password = NULL;
}

char *ExtractPath(char *URL,bool proxy)
{
  char *path;
  char l[]=".?/",c;
  int i;

  if(proxy)
  {
    URL = strstr(URL,"://") + 3;
    URL = strchr(URL,'/');
    if(URL == NULL)
    {
      return SU_strdup((char *)"/");
    }
  }
  path = SU_strdup(URL);
  if(strcmp(path,"/") == 0)
    return path;
  if(path[strlen(path)-1] == '/')
  {
    path[strlen(path)-1] = 0;
    return path;
  }
  if(SU_strrchrl(path,l,&c) == NULL)
    return path;
  if(c == '/')
    return path;

  i = (int)(strlen(path)-1);
  while(path[i] != '/')
  {
    if(i == 0)
    {
      path[0] = '/';
      break;
    }
    i--;
  }
  if(i == 0)
    path[1] = 0;
  else
    path[i] = 0;
  return path;
}

void AfficheCookie(SU_PCookie Cok)
{
  printf("Cookie : %s=%s--\n",Cok->Name,Cok->Value);
  if(Cok->Domain != NULL)
    printf("  Domain = %s--\n",Cok->Domain);
  if(Cok->Path != NULL)
    printf("  Path = %s--\n",Cok->Path);
  if(Cok->Expire != NULL)
    printf("  Expires = %s--\n",Cok->Expire);
  if(Cok->Secured)
    printf("  Secured\n");
}

SKYUTILS_API void SU_FreeCookie(SU_PCookie Cok)
{
  free(Cok->Name);
  free(Cok->Value);
  if(Cok->Domain != NULL)
    free(Cok->Domain);
  if(Cok->Path != NULL)
    free(Cok->Path);
  if(Cok->Expire != NULL)
    free(Cok->Expire);
  free(Cok);
}

int GetPortFromHost(char *Host,bool ssl_mode)
{
  char *p;

  p = strchr(Host,':');
  if(p == NULL)
  {
    return ssl_mode?DEFAULT_SSL_PORT:DEFAULT_PORT;
  }
  p[0] = 0;
  p++;
  return atoi(p);
}

int GetHostFromURL(const char *URL,char Host[],int Length,bool proxy,char URL_OUT[],int *PortConnect,const char OtherHost[],bool *ssl_mode)
{
  char *ptr,*ptr2;
  int len;
  char buf[URL_BUF_SIZE];
  char ReplaceHost[URL_BUF_SIZE];

  SU_strcpy(ReplaceHost,OtherHost,sizeof(ReplaceHost));
  SU_strcpy(URL_OUT,URL,URL_BUF_SIZE);
  *ssl_mode = false;
  if(SU_nocasestrstr((char *)URL,"https") == URL)
  {
#ifndef SU_USE_SSL
    if(1)
    {
      printf("SkyUtils_GetHostFromURL Error : HTTPS requested, but skyutils not compiled with SSL support. Exiting !\n");
      return -10;
    }
    *ssl_mode = false;
#else /* SU_USE_SSL */
    *ssl_mode = true;
#endif /* ! SU_USE_SSL */
    ptr = (char *)URL+8;
    ptr2 = strchr(ptr,'/');
  }
  else if(SU_nocasestrstr((char *)URL,"http") == URL)
  {
    ptr = (char *)URL+7;
    ptr2 = strchr(ptr,'/');
  }
  else if(SU_nocasestrstr((char *)URL,"ftp") == URL)
  {
    ptr = (char *)URL+6;
    ptr2 = strchr(ptr,'@');
    if(ptr2 != NULL)
    {
      ptr = ptr2+1;
      ptr2 = strchr(ptr,'/');
    }
  }
  else
  {
    if(ReplaceHost[0] == 0)
      SU_strcpy(Host,URL,Length);
    else
      SU_strcpy(Host,ReplaceHost,Length);
    if(!proxy)
    {
      URL_OUT[0] = '/';
      URL_OUT[1] = 0;
      *PortConnect = GetPortFromHost(Host,*ssl_mode);
    }
    return 0;
  }
  if(ptr2 == NULL)
  {
    if(ReplaceHost[0] == 0)
      SU_strcpy(Host,ptr,Length);
    else
      SU_strcpy(Host,ReplaceHost,Length);
    if(!proxy)
    {
      URL_OUT[0] = '/';
      URL_OUT[1] = 0;
      *PortConnect = GetPortFromHost(Host,*ssl_mode);
    }
    return 0;
  }
  len = (int)(ptr2 - ptr + 1); /* +1 for the \0 */
  if(len > Length)
    len = Length;
  if(ReplaceHost[0] == 0)
  {
    SU_strcpy(Host,ptr,len);
  }
  else
  {
    SU_strcpy(Host,ReplaceHost,Length);
  }
  if(!proxy)
  { /* If not using a proxy, we must remove host from URL_OUT */
    SU_strcpy(buf,ptr2,sizeof(buf));
    SU_strcpy(URL_OUT,buf,URL_BUF_SIZE);
    *PortConnect = GetPortFromHost(Host,*ssl_mode);
  }
  else
  { /* Using proxy ? */
    if(ReplaceHost[0] != 0)
    { /* Ahh, we must replace host in URL_OUT */
      if(URL[0] == 'h')
      {
        if(*ssl_mode)
          strcpy(URL_OUT,"https://");
        else
          strcpy(URL_OUT,"http://");
      }
      else
        strcpy(URL_OUT,"ftp://");
      SU_strcpy(buf,ptr2,sizeof(buf));
      SU_strcat(URL_OUT,ReplaceHost,URL_BUF_SIZE);
      SU_strcat(URL_OUT,buf,URL_BUF_SIZE);
      *PortConnect = GetPortFromHost((char *)ReplaceHost,*ssl_mode);
    }
  }
  return 0;
}

void FreeAnswer(SU_PAnswer Ans)
{
  if(Ans == NULL)
    return;
  if(Ans->Location != NULL)
    free(Ans->Location);
  if(Ans->Data != NULL)
    free(Ans->Data);
}

SU_PAnswer ParseBuffer(SU_PAnswer Ans,char *Buf,int *len,SU_PHTTPActions Act,bool proxy)
{
  char *ptr,*ptr2;
  char *tmp,*tok;
  char *saf; /* Used at the end of the while ! DO NOT USE */
  SU_PCookie Cok;
  float f;
  SU_PList Ptr;

  if(Ans == NULL)
  {
    Ans = (SU_PAnswer) malloc(sizeof(SU_TAnswer));
    memset(Ans,0,sizeof(SU_TAnswer));
    Ans->Data_Length = -1;
    Ans->Data_ToReceive = -1;
  }
  if(Ans->Data_Length != -1)
  {
    Ans->Data = (char *) realloc(Ans->Data,Ans->Data_Length+*len+1); /* +1 for \0 */
    memcpy(Ans->Data+Ans->Data_Length,Buf,*len);
    Ans->Data_Length += *len;
    Ans->Data[Ans->Data_Length] = 0;
    *len = 0;
    return Ans;
  }
  while(*len != 0)
  {
    ptr = strstr(Buf,"\r\n");
    if(ptr == NULL) /* Not enough bytes received */
      return Ans;
    if(ptr == Buf) /* Data following */
    {
#ifdef __unix__
      if(SU_DebugLevel >= 3)
      {
        printf("SkyUtils_ParseBuffer : Found Data in HTTP answer\n");
        if(Ans->Data_ToReceive >= 0)
          printf("SkyUtils_ParseBuffer : Waiting %d bytes\n",Ans->Data_ToReceive);
      }
#endif /* __unix__ */
      Ans->Data_Length = 0;
      if(*len == 2) /* Not enough data */
        return Ans;
      Ans->Data = (char *) malloc(*len-2+1); /* +1 for \0 */
      memcpy(Ans->Data,Buf+2,*len-2);
      Ans->Data_Length = *len - 2;
      Ans->Data[Ans->Data_Length] = 0;
      *len = 0;
      return Ans;
    }
    ptr[0] = 0;
    saf = ptr;
    /* Parse header command */
#ifdef __unix__
    if(SU_DebugLevel >= 3)
      printf("SkyUtils_ParseBuffer : Found header : %s\n",Buf);
#endif /* __unix__ */
    if(SU_nocasestrstr(Buf,"HTTP/") == Buf) /* Found reply code */
    {
      sscanf(Buf,"HTTP/%f %d",&f,&Ans->Code);
    }
    else if(SU_nocasestrstr(Buf,"Content-Length") == Buf)
    {
      Ans->Data_ToReceive = atoi(strchr(Buf,':')+1);
    }
    else if(SU_nocasestrstr(Buf,"Set-Cookie") == Buf) /* Found Set-Cookie */
    {
      Cok = (SU_PCookie) malloc(sizeof(SU_TCookie));
      memset(Cok,0,sizeof(SU_TCookie));
      tmp = SU_TrimLeft(strchr(Buf,':') + 1);
      tmp = SU_strdup(tmp);
      tok = SU_TrimLeft(strtok(tmp,";"));
      /* Get NAME=VALUE */
      ptr2 = strchr(tok,'=');
      ptr2[0] = 0;
      Cok->Name = SU_strdup(tok);
      Cok->Value = SU_strdup(ptr2+1);
      /* Get options */
      tok = SU_TrimLeft(strtok(NULL,";"));
      while(tok != NULL)
      {
        if(strncasecmp(tok,"expires",7) == 0)
        {
          ptr2 = strchr(tok,'=');
          if(ptr2 != NULL)
          {
            Cok->Expire = SU_strdup(ptr2+1);
          }
          else
            printf("SkyUtils_ParseBuffer Warning : Error with Expire value in cookie : %s\n",tok);
        }
        else if(strncasecmp(tok,"path",4) == 0)
        {
          ptr2 = strchr(tok,'=');
          if(ptr2 != NULL)
          {
            Cok->Path = SU_strdup(ptr2+1);
          }
          else
            printf("SkyUtils_ParseBuffer Warning : Error with Path value in cookie : %s\n",tok);
        }
        else if(strncasecmp(tok,"domain",6) == 0)
        {
          ptr2 = strchr(tok,'=');
          if(ptr2 != NULL)
          {
            if(ptr2[1] == '.')
              Cok->Domain = SU_strdup(ptr2+2);
            else
              Cok->Domain = SU_strdup(ptr2+1);
          }
          else
            printf("SkyUtils_ParseBuffer Warning : Error with Domain value in cookie : %s\n",tok);
        }
        else if(strncasecmp(tok,"secure",6) == 0)
        {
          Cok->Secured = true;
        }
#ifdef __unix__
        else if(SU_DebugLevel >= 1)
          printf("SkyUtils_ParseBuffer Warning : Unknown option in Set-Cookie : %s\n",tok);
#endif /* __unix__ */
        tok = SU_TrimLeft(strtok(NULL,";"));
      }
      free(tmp);
      if(Cok->Domain == NULL)
      {
        Cok->Domain = SU_strdup(Act->Host);
      }
      if(Cok->Path == NULL)
      {
        tmp = ExtractPath(Act->URL,proxy);
        Cok->Path = SU_strdup(tmp);
        free(tmp);
      }
#ifdef __unix__
      if(SU_DebugLevel >= 4)
        AfficheCookie(Cok);
#endif /* __unix__ */
      /* Check if a cookie with same Name/Domain/Path exists */
      Ptr = SW_Cookies;
      while(Ptr != NULL)
      {
        if((strcmp(((SU_PCookie)Ptr->Data)->Name,Cok->Name) == 0) && (strcmp(((SU_PCookie)Ptr->Data)->Domain,Cok->Domain) == 0))
        {
          if((Cok->Path != NULL) && (((SU_PCookie)Ptr->Data)->Path != NULL))
          {
            if(strcmp(((SU_PCookie)Ptr->Data)->Path,Cok->Path) == 0)
            {
              SU_FreeCookie((SU_PCookie)Ptr->Data);
              Ptr->Data = Cok;
              break;
            }
          }
        }
        Ptr = Ptr->Next;
      }
      if(Ptr == NULL)
        SW_Cookies = SU_AddElementTail(SW_Cookies,Cok);
    }
    else if(SU_nocasestrstr(Buf,"Location") == Buf) /* Found Location */
    {
      ptr2 = SU_TrimLeft(strchr(Buf,':') + 1);
      Ans->Location = SU_strdup(ptr2);
    }
    /* End of parse header command */
    *len -= (int)((saf - Buf) + 2);
    memmove(Buf,saf+2,*len);
  }
  return Ans;
}

static int CreateConnection(char Host[],int Port,SSL **ssl)
{
  SU_SOCKET Sock;
  struct sockaddr_in sin;
  struct hostent *HE;

  Sock = socket(AF_INET,SOCK_STREAM,getprotobyname("tcp")->p_proto);
  if(Sock == -1)
    return -1;
  sin.sin_family = AF_INET;
  sin.sin_port = htons((unsigned short)Port);
  sin.sin_addr.s_addr = inet_addr(Host);
  if(sin.sin_addr.s_addr == INADDR_NONE)
  {
    HE = gethostbyname(Host);
    if( HE == NULL )
    {
      printf("SkyUtils_CreateConnection : Unknown Host : %s\n",Host);
      return -2;
    }
    sin.sin_addr = *(struct in_addr *)(HE->h_addr_list[0]);
  }
  if(connect(Sock,(struct sockaddr *)(&sin),sizeof(sin)) == -1)
  {
    SU_CLOSE_SOCKET(Sock);
    return -3;
  }
#ifdef SU_USE_SSL
  if(ssl != NULL)
  {
    char errormsg[1024];
    *ssl = SU_SSL_Connect(Sock,errormsg);
    if(*ssl == NULL)
    {
      printf("SkyUtils_CreateConnection : %s\n", errormsg);
      SU_CLOSE_SOCKET(Sock);
      return -4;
    }
  }
#endif
  return (int)Sock;
}

/* Base64 encode a string */
char * http_base64_encode(const char *text)
{

  const char b64_alphabet[] = {
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz"
      "0123456789+/=" };

  /* The tricky thing about this is doing the padding at the end,
   * doing the bit manipulation requires a bit of concentration only */
  char *buffer = NULL;
  char *point = NULL;
  int inlen = 0;
  int outlen = 0;

  /* check our args */
  if (text == NULL)
    return NULL;

  /* Use 'buffer' to store the output. Work out how big it should be...
   * This must be a multiple of 4 bytes */

  inlen = (int)(strlen( text ));
  /* check our arg...avoid a pesky FPE */
  if (inlen == 0)
    {
      buffer = (char *) malloc(sizeof(char));
      buffer[0] = '\0';
      return buffer;
    }
  outlen = (inlen*4)/3;
  if( (inlen % 3) > 0 ) /* got to pad */
    outlen += 4 - (inlen % 3);

  buffer = (char *) malloc( outlen + 1 ); /* +1 for the \0 */
  memset(buffer, 0, outlen + 1); /* initialize to zero */

  /* now do the main stage of conversion, 3 bytes at a time,
   * leave the trailing bytes (if there are any) for later */

  for( point=buffer; inlen>=3; inlen-=3, text+=3 ) {
    *(point++) = b64_alphabet[ *text>>2 ];
    *(point++) = b64_alphabet[ (*text<<4 & 0x30) | *(text+1)>>4 ];
    *(point++) = b64_alphabet[ (*(text+1)<<2 & 0x3c) | *(text+2)>>6 ];
    *(point++) = b64_alphabet[ *(text+2) & 0x3f ];
  }

  /* Now deal with the trailing bytes */
  if( inlen ) {
    /* We always have one trailing byte */
    *(point++) = b64_alphabet[ *text>>2 ];
    *(point++) = b64_alphabet[ (*text<<4 & 0x30) |
			     (inlen==2?*(text+1)>>4:0) ];
    *(point++) = (inlen==1?'=':b64_alphabet[ *(text+1)<<2 & 0x3c ] );
    *(point++) = '=';
  }

  *point = '\0';

  return buffer;
}

static int SendBuffer(SU_SOCKET Sock,char *buf,int len,SSL *ssl,bool verbose)
{
  int res;

#ifdef SU_USE_SSL
  if(ssl)
  {
    if(verbose)
    {
      char errormsg[1024];
      res = SU_SSL_Write(ssl,buf,len,errormsg);
      if(res == -1)
      {
        printf("SkyUtils_SendCommand Error : Error sending command using SSL : %s\n",errormsg);
      }
    }
    else
      res = SU_SSL_Write(ssl,buf,len,NULL);
  }
  else
#endif /* SU_USE_SSL */
#ifdef __unix__
  if(SU_DebugLevel >= 2)
    printf("SkyUtils_SendCommand : Sending %s(%d) : %s\n",ssl?"SSL ":"",len,buf);
#endif /* __unix__ */
  res = send(Sock,buf,len,0);
  return res;
}

static int SendFile(SU_SOCKET Sock,FILE *fp,int FLen,SSL *ssl)
{
  int res = 0;
  char buf[16000];
  int len,pos;

  while(res >= 0)
  {
    len = (FLen > sizeof(buf))?sizeof(buf):FLen;
    if(fread(buf,len,1,fp) != 1)
    {
      res = -1;
      break;
    }
#ifdef SU_USE_SSL
    if(ssl)
      res = SU_SSL_Write(ssl,buf,len,NULL);
    else
#endif /* SU_USE_SSL */
    res = send(Sock,buf,len,SU_MSG_NOSIGNAL);
    FLen -= len;
    if(res <= 0)
    {
      res = -1;
      break;
    }
    else if(res != len)
    { /* Not all bytes sent */
      pos = res;
      len -= res;
      while(len > 0)
      {
#ifdef SU_USE_SSL
        if(ssl)
          res = SU_SSL_Write(ssl,buf+pos,len,NULL);
        else
#endif /* SU_USE_SSL */
        res = send(Sock,buf+pos,len,SU_MSG_NOSIGNAL);
        if(res <= 0)
          break;
        pos += res;
        len -= res;
      }
      if(res <= 0)
        break;
    }
    if(FLen == 0)
    {
      len = 0;
      buf[len++] = 0x0D;
      buf[len++] = 0x0A;
      buf[len] = 0;
#ifdef SU_USE_SSL
      if(ssl)
        res = SU_SSL_Write(ssl,buf,len,NULL);
      else
#endif /* SU_USE_SSL */
      send(Sock,buf,len,SU_MSG_NOSIGNAL);
#ifdef __unix__
      if(SU_DebugLevel >= 2)
        printf("SkyUtils_SendCommand : Successfully sent file\n");
#endif /* __unix__ */
      res = 0;
      break;
    }
  }
  return res;
}

bool SendCommand(SU_SOCKET Sock,SU_PHTTPActions Act,bool proxy,SSL *ssl)
{
  char *buf;
  int len;
  SU_u64 FLen;
  int res;
  char *Com,*tmp,*tmp2,*tmp3;
  SU_PList Ptr;
  int cook,blen,blen2;
  FILE *fp;
  bool do_it;
  int buf_len = 16000+Act->Post_Length;

  if(Act->Command == ACT_GET)
    Com = "GET";
  else if(Act->Command == ACT_POST)
    Com = "POST";
  else if(Act->Command == ACT_PUT)
    Com = "PUT";
  else if(Act->Command == ACT_DELETE)
    Com = "DELETE";
  else
    Com = "ERROR";
  buf = (char *) malloc(buf_len);
  if(Act->URL_Params == NULL)
    SU_snprintf(buf,buf_len,"%s %s HTTP/1.0%c%cHost: %s%c%c",Com,Act->URL,0x0D,0x0A,Act->Host,0x0D,0x0A);
  else
    SU_snprintf(buf,buf_len,"%s %s?%s HTTP/1.0%c%cHost: %s%c%c",Com,Act->URL,Act->URL_Params,0x0D,0x0A,Act->Host,0x0D,0x0A);
  len = (int)(strlen(buf));
  /* Now add header from file, or default one */
  if(SW_UserHeader == NULL)
  {
    if(SW_UserAgent == NULL)
      SU_SetUserAgent(SW_DEFAULT_USER_AGENT);
    SU_snprintf(buf+len,buf_len-len,"User-Agent: %s\x0D\x0A%s",SW_UserAgent,SW_DEFAULT_HEADER);
  }
  else
    SU_snprintf(buf+len,buf_len-len,"%s",SW_UserHeader);
  len = (int)(strlen(buf));

  Ptr = SW_Cookies;
  cook = 0;
  while(Ptr != NULL)
  {
    blen = (int)(strlen(((SU_PCookie)Ptr->Data)->Domain)+2);
    if(strchr(Act->Host,':') == NULL)
    {
      tmp = (char *) malloc(blen);
      SU_snprintf(tmp,blen,"*%s",((SU_PCookie)Ptr->Data)->Domain);
    }
    else
    {
      tmp = (char *) malloc(blen+2);
      SU_snprintf(tmp,blen+2,"*%s:*",((SU_PCookie)Ptr->Data)->Domain);
    }
    if(SU_strwcmp(Act->Host,tmp))
    {
      do_it = false;
      if(((SU_PCookie)Ptr->Data)->Path == NULL)
        do_it = true;
      else
      {
        blen2 = (int)(strlen(((SU_PCookie)Ptr->Data)->Path)+2);
        tmp2 = (char *) malloc(blen2);
        SU_snprintf(tmp2,blen2,"%s*",((SU_PCookie)Ptr->Data)->Path);
        tmp3 = ExtractPath(Act->URL,proxy);
        if(SU_strwcmp(tmp3,tmp2))
          do_it = true;
        free(tmp2);
        free(tmp3);
      }
      if(do_it)
      {
        if(cook == 0)
        {
          SU_snprintf(buf+len,buf_len-len,"Cookie: %s=%s",((SU_PCookie)Ptr->Data)->Name,((SU_PCookie)Ptr->Data)->Value);
          len = (int)(strlen(buf));
          cook = 1;
        }
        else
        {
          SU_snprintf(buf+len,buf_len-len,"; %s=%s",((SU_PCookie)Ptr->Data)->Name,((SU_PCookie)Ptr->Data)->Value);
          len = (int)(strlen(buf));
        }
      }
    }
    free(tmp);
    Ptr = Ptr->Next;
  }
  if(cook != 0)
  {
    buf[len++] = 0x0D;
    buf[len++] = 0x0A;
  }
  if(Act->Referer != NULL)
  {
    SU_snprintf(buf+len,buf_len-len,"Referer: %s%c%c",Act->Referer,0x0D,0x0A);
    len = (int)(strlen(buf));
  }
  /* Manage proxy authorization */
  if(proxy != 0)
  {
    if(SW_Proxy_User != NULL)
    {
       char authtoken[256];
       char *auth64=NULL;

       if(SW_Proxy_Password != NULL)
          SU_snprintf(authtoken,255,"%s:%s",SW_Proxy_User,SW_Proxy_Password);
       else
          SU_snprintf(authtoken,255,"%s:",SW_Proxy_User);
       auth64 = http_base64_encode(authtoken);
       if(auth64 != NULL)
       {
          SU_snprintf(buf+len,buf_len-len,"Proxy-Authorization: Basic %s%c%c",auth64,0x0D,0x0A);
    	  len = (int)(strlen(buf));
          free(auth64);
       }
    }
  }
  if(Act->Command == ACT_POST)
  {
    if(Act->MultiParts == NULL) /* Post_Data */
    {
      SU_snprintf(buf+len,buf_len-len,"Content-type: application/x-www-form-urlencoded%c%cContent-length: %d%c%c%c%c",0x0D,0x0A,Act->Post_Length,0x0D,0x0A,0x0D,0x0A);
      len = (int)(strlen(buf));
      memcpy(buf+len,Act->Post_Data,Act->Post_Length);
      len += Act->Post_Length;
      buf[len++] = 0x0D;
      buf[len++] = 0x0A;
      buf[len] = 0;
      SendBuffer(Sock,buf,len,ssl,true);
    }
    else /* MultiParts */
    {
      int multi_length = 0;
      SU_PList Ptr = Act->MultiParts;
      SU_PHTTPPart Part;
      char boundary[27+13+1];
      int boundary_length;
      int multi_sent = 0;
      SU_u32 tim = (SU_u32) time(NULL);
      SU_u32 pid = SU_PROCESS_SELF;
      SU_u32 tid = SU_THREAD_SELF;
      if(tim >= 0x1000000)
        tim &= 0xFFFFFF;
      if(pid >= 0x10000)
        pid &= 0xFFFF;
      if(tid >= 0x1000)
        tid &= 0xFFF;

      SU_snprintf(boundary,sizeof(boundary),"---------------------------%06x%04x%03x",tim,pid,tid);
      boundary_length = (int)(strlen(boundary));
      while(Ptr != NULL)
      {
        Part = (SU_PHTTPPart) Ptr->Data;
        multi_length += boundary_length + 2 + 2; /* +2 (--) before boundary, + 2 (\n) after boundary */
        if(Part->Header)
        {
          multi_length += (int)(strlen(Part->Header) + 2); /* +2 after header */
        }
        if(Part->FileName)
        {
          fp = fopen(Part->FileName,"rb");
          if(fp == NULL)
          {
            free(buf);
            return false;
          }
          fseek(fp,0,SEEK_END);
          Part->Length = ftell(fp);
          fclose(fp);
        }
        multi_length += Part->Length + 2 + 2; /* +2 (\n) before Data, +2 (\n) after data */
        Ptr = Ptr->Next;
      }
      multi_length += boundary_length + 2 + 2 + 2; /* +2 (--) before boundary, +2 (--) for final boundary, +2 (\n) after final boundary */
      SU_snprintf(buf+len,buf_len,"Content-type: multipart/form-data; boundary=%s%c%cContent-length: %d%c%c%c%c",boundary,0x0D,0x0A,multi_length,0x0D,0x0A,0x0D,0x0A);
      len = (int)(strlen(buf));
      buf[len] = 0;
      SendBuffer(Sock,buf,len,ssl,true);

      Ptr = Act->MultiParts;
      while(Ptr != NULL)
      {
        Part = (SU_PHTTPPart) Ptr->Data;
        if(Part->Header)
        {
          SU_snprintf(buf,buf_len,"--%s%c%c%s%c%c%c%c",boundary,0x0D,0x0A,Part->Header,0x0D,0x0A,0x0D,0x0A);
          len = (int)(strlen(buf));
          buf[len] = 0;
        }
        else
        {
          SU_snprintf(buf,buf_len,"--%s%c%c%c%c",boundary,0x0D,0x0A,0x0D,0x0A);
          len = (int)(strlen(buf));
          buf[len] = 0;
        }
        SendBuffer(Sock,buf,len,ssl,true);
        multi_sent += len;
        if(Part->FileName) /* File */
        {
          fp = fopen(Part->FileName,"rb");
          if(fp == NULL)
          {
            free(buf);
            return false;
          }
          res = SendFile(Sock,fp,Part->Length,ssl);
          multi_sent += Part->Length;
          fclose(fp);
          if(res == -1)
          {
            free(buf);
            return false;
          }
        }
        else /* Data */
        {
          SendBuffer(Sock,Part->Data,Part->Length,ssl,false);
          multi_sent += Part->Length;
        }
        /* End of boundary */
        SU_snprintf(buf,buf_len,"%c%c",0x0D,0x0A);
        len = (int)(strlen(buf));
        buf[len] = 0;
        SendBuffer(Sock,buf,len,ssl,true);
        multi_sent += len;
        Ptr = Ptr->Next;
      }
      /* Final boundary */
      SU_snprintf(buf,buf_len,"--%s--%c%c",boundary,0x0D,0x0A);
      len = (int)(strlen(buf));
      buf[len] = 0;
      SendBuffer(Sock,buf,len,ssl,true);
      multi_sent += len;
      if(multi_sent != multi_length)
      {
        printf("SkyUtils_SendCommand Warning : Multi-part length differs from sent length : %d - %d\n",multi_length,multi_sent);
      }
    }
  }
  else if((Act->Command == ACT_GET) || (Act->Command == ACT_DELETE))
  {
    if(Act->ContentType != NULL)
    {
      SU_snprintf(buf+len,buf_len-len,"Content-Type: %s%c%c",Act->ContentType,0x0D,0x0A);
      len = (int)(strlen(buf));
    }
    buf[len++] = 0x0D;
    buf[len++] = 0x0A;
    buf[len] = 0;
    SendBuffer(Sock,buf,len,ssl,true);
  }
  if(Act->Command == ACT_PUT)
  {
    fp = fopen(Act->FileName,"rb");
    if(fp == NULL)
    {
      free(buf);
      return false;
    }
    fseek(fp,0,SEEK_END);
    FLen = (SU_u64)ftell(fp);
    rewind(fp);
#ifdef __unix__
    if(SU_DebugLevel >= 2)
      printf("SkyUtils_SendCommand : Sending file %s of length %ld\n",Act->FileName,FLen);
#endif /* __unix__ */
    SU_snprintf(buf+len,buf_len-len,"Content-Type: application/octet-stream%c%cContent-length: %ld%c%c%c%c",FLen,0x0D,0x0A,0x0D,0x0A,0x0D,0x0A);
    len = (int)(strlen(buf));
    res = SendBuffer(Sock,buf,len,ssl,true);
    res = SendFile(Sock,fp,(int)FLen,ssl);
    fclose(fp);
    if(res == -1)
    {
      if(Act->CB.OnErrorSendingFile != NULL)
        Act->CB.OnErrorSendingFile(errno,Act->User);
#ifdef __unix__
      if(SU_DebugLevel >= 2)
        printf("SkyUtils_SendCommand Warning : Error sending file, %ld bytes remaining not sent\n",FLen);
#endif /* __unix__ */
    }
#ifdef SU_USE_SSL
    if(ssl)
    {
      SU_SSL_SessionCleanup(ssl);
    }
#endif /* SU_USE_SSL */
    SU_CLOSE_SOCKET(Sock);
    free(buf);
    return res == 0;
  }
  free(buf);
  return true;
}

SU_PAnswer WaitForAnswer(SU_SOCKET Sock,SU_PHTTPActions Act,bool proxy,SSL *ssl)
{
  int len;
  int BufPos = 0;
  char Buf[32768];
  SU_PAnswer Ans = NULL;
  fd_set rfds;
  struct timeval tv;
  int retval;

  FD_ZERO(&rfds);
  FD_SET(Sock,&rfds);
  tv.tv_sec = SW_SocketTimeout;
  tv.tv_usec = 0;
  retval = select((int)(Sock+1),&rfds,NULL,NULL,&tv);
  if(retval != 1)
    return NULL;
#ifdef SU_USE_SSL
  if(ssl)
    len = SU_SSL_Read(ssl,Buf,sizeof(Buf),NULL);
  else
#endif /* SU_USE_SSL */
  len = recv(Sock,Buf,sizeof(Buf),0);
  while(len > 0)
  {
    len += BufPos;
    Ans = ParseBuffer(Ans,Buf,&len,Act,proxy);
    BufPos = len;
    if(Ans->Data_ToReceive >= 0)
    {
      if(Ans->Data_Length >= Ans->Data_ToReceive)
        break;
    }
    FD_ZERO(&rfds);
    FD_SET(Sock,&rfds);
    tv.tv_sec = SW_SocketTimeout;
    tv.tv_usec = 0;
    retval = select((int)(Sock+1),&rfds,NULL,NULL,&tv);
    if(retval == 0) /* Time out */
    {
      if(Ans->Data_Length == -1)
      {
        FreeAnswer(Ans);
        Ans = NULL;
      }
#ifdef __unix__
      else if(SU_DebugLevel >= 1)
        printf("SkyUtils_WaitForAnswer Warning : Connection timed out, but some datas were retrieved\n");
#endif /* __unix__ */
      break;
    }
    else if(retval < 0)
    {
      if(Ans->Data_Length == -1)
      {
        FreeAnswer(Ans);
        Ans = NULL;
      }
#ifdef __unix__
      else if(SU_DebugLevel >= 1)
        printf("SkyUtils_WaitForAnswer Warning : Unexpected network error : %d\n",errno);
#endif /* __unix__ */
      break;
    }
#ifdef SU_USE_SSL
    if(ssl)
      len = SU_SSL_Read(ssl,Buf+BufPos,sizeof(Buf)-BufPos,NULL);
    else
#endif /* SU_USE_SSL */
    len = recv(Sock,Buf+BufPos,sizeof(Buf)-BufPos,0);
  }
#ifdef SU_USE_SSL
  if(ssl)
  {
    SU_SSL_SessionCleanup(ssl);
  }
#endif /* SU_USE_SSL */
  SU_CLOSE_SOCKET(Sock);
  if(Ans != NULL)
  {
#ifdef __unix__
    if(SU_DebugLevel >= 5)
      DumpPage(NULL,Ans->Data,Ans->Data_Length);
#endif /* __unix__ */
    if((Ans->Data != NULL) && (Act->FileName != NULL) && ((Act->Command == ACT_GET) || (Act->Command == ACT_POST)))
      DumpPage(Act->FileName,Ans->Data,Ans->Data_Length);
  }
  return Ans;
}

bool SU_SendProxySSLConnect(SU_SOCKET Sock,char *Host,int Port,int *Code)
{
  char buf[1024];
  int res,len;
  fd_set rfds;
  struct timeval tv;
  int retval,BufPos = 0;
  bool found = false,again = true;
  char *ptr;
  float f;

  if(SW_UserAgent == NULL)
    SU_SetUserAgent(SW_DEFAULT_USER_AGENT);
  SU_snprintf(buf,sizeof(buf),"CONNECT %s:%d HTTP/1.0%c%cUser-Agent: %s%c%cHost: %s%c%cProxy-Connection: close%c%cConnection: close%c%c%c%c",Host,Port,0x0D,0x0A,SW_UserAgent,0x0D,0x0A,Host,0x0D,0x0A,0x0D,0x0A,0x0D,0x0A,0x0D,0x0A);
  len = (int)(strlen(buf));
  res = send(Sock,buf,len,0);
  if(res != len)
    return false;

  FD_ZERO(&rfds);
  FD_SET(Sock,&rfds);
  tv.tv_sec = SW_SocketTimeout;
  tv.tv_usec = 0;
  retval = select((int)(Sock+1),&rfds,NULL,NULL,&tv);
  if(retval != 1)
    return false;
  res = recv(Sock,buf,sizeof(buf)-1,0);
  while((res > 0) && again)
  {
    BufPos += res;
    buf[BufPos] = 0;
    ptr = strstr(buf,"\r\n");
    while(ptr != NULL)
    {
      if(ptr == buf) /* End of answer */
      {
        again = false;
        break;
      }
      if(SU_nocasestrstr(buf,"HTTP/") == buf) /* Found reply code */
      {
        sscanf(buf,"HTTP/%f %d",&f,Code);
        if(*Code == 200) /* Ok */
        {
          BufPos = 0;
          found = true;
          break;
        }
      }
      BufPos -= (int)(ptr+2-buf-1); /* -1 for \0 */
      memmove(buf,ptr+2,BufPos);
      ptr = strstr(buf,"\r\n");
    }
    if(again)
      break;
    FD_ZERO(&rfds);
    FD_SET(Sock,&rfds);
    tv.tv_sec = SW_SocketTimeout;
    tv.tv_usec = 0;
    retval = select((int)(Sock+1),&rfds,NULL,NULL,&tv);
    if(retval != 1)
      return found;
    res = recv(Sock,buf+BufPos,sizeof(buf)-BufPos-1,0);
  }
  return found;
}

SKYUTILS_API char *SU_EncodeURL(const char URL_in[],char URL_out[],int URL_out_len)
{
  char NB[10];
  int i,pos;
  int len = (int)(strlen(URL_in));

  pos = 0;
  for(i=0;i<len;i++)
  {
    if((URL_in[i] >='!') && (URL_in[i] <='~'))
    {
      if((pos+1) >= URL_out_len)
      {
        break;
      }
      URL_out[pos++] = URL_in[i];
    }
    else
    {
      if((pos+3) >= URL_out_len)
      {
        break;
      }
      URL_out[pos++] = '%';
      SU_snprintf(NB,sizeof(NB),"%.2x",URL_in[i]);
      URL_out[pos++] = NB[strlen(NB)-2];
      URL_out[pos++] = NB[strlen(NB)-1];
    }
  }
  URL_out[pos] = 0;
  return URL_out;
}

SKYUTILS_API int SU_ExecuteActions(SU_PList Actions)
{
  SU_PList Ptr = Actions;
  SU_PList ActRec = NULL;
  SU_PAnswer Ans;
  SU_THTTPActions Act;
  char URL_OUT[URL_BUF_SIZE];
  int  Sock;
  char *ptr;
  int  PortConnect;
  SSL *ssl = NULL;
  bool ssl_mode = false;
  int Code;
  char *proxy_string;

  while(Ptr != NULL)
  {
    if(((SU_PHTTPActions)Ptr->Data)->Sleep != 0)
    {
#ifdef __unix__
      if(SU_DebugLevel >= 1)
        printf("SkyUtils_SU_ExecuteActions : Sleeping %d sec before sending command\n",((SU_PHTTPActions)Ptr->Data)->Sleep);
      sleep(((SU_PHTTPActions)Ptr->Data)->Sleep); /* Sleeping */
#else /* !__unix__ */
      Sleep(((SU_PHTTPActions)Ptr->Data)->Sleep*1000); /* Sleeping */
#endif /* __unix__ */
    }
    switch(((SU_PHTTPActions)Ptr->Data)->Command)
    {
      case ACT_GET :
      case ACT_POST :
      case ACT_PUT :
      case ACT_DELETE :
        Code = GetHostFromURL(((SU_PHTTPActions)Ptr->Data)->URL,((SU_PHTTPActions)Ptr->Data)->Host,sizeof(((SU_PHTTPActions)Ptr->Data)->Host),(SW_Proxy_String != NULL),URL_OUT,&PortConnect,((SU_PHTTPActions)Ptr->Data)->Host,&ssl_mode);
        if(Code != 0)
          return Code;
        ((SU_PHTTPActions)Ptr->Data)->SSL = ssl_mode;
        if(((SU_PHTTPActions)Ptr->Data)->CB.OnSendingCommand != NULL)
          ((SU_PHTTPActions)Ptr->Data)->CB.OnSendingCommand((SU_PHTTPActions)Ptr->Data);
        /* Get URL_OUT once again, if 'Act' has been modified in OnSendingCommand */
        Code = GetHostFromURL(((SU_PHTTPActions)Ptr->Data)->URL,((SU_PHTTPActions)Ptr->Data)->Host,sizeof(((SU_PHTTPActions)Ptr->Data)->Host),(SW_Proxy_String != NULL),URL_OUT,&PortConnect,"",&ssl_mode);
        SU_strcpy(((SU_PHTTPActions)Ptr->Data)->URL,URL_OUT,sizeof(URL_OUT));
        if(SW_Proxy_String != NULL)
        {
#ifdef __unix__
          if(SU_DebugLevel >= 1)
          {
            if(SW_Proxy_User == NULL)
              printf("SkyUtils_SU_ExecuteActions : Using proxy: %s, port %d\n",SW_Proxy_String,SW_Proxy_Port);
            else
              printf("SkyUtils_SU_ExecuteActions : Using proxy: %s, with user %s [%s], port %d\n",SW_Proxy_String,SW_Proxy_User,SW_Proxy_Password,SW_Proxy_Port);
          }
#endif /* __unix__ */
          Sock = CreateConnection(SW_Proxy_String,SW_Proxy_Port,NULL); /* Not using SSL struct with a proxy */
        }
        else
          Sock = CreateConnection(((SU_PHTTPActions)Ptr->Data)->Host,PortConnect,ssl_mode?&ssl:NULL);
        if(Sock < 0)
        {
          printf("SkyUtils_SU_ExecuteActions Error : Cannot connect to the host\n");
          return(-1);
        }
        proxy_string = SW_Proxy_String;
#ifdef SU_USE_SSL
        if((SW_Proxy_String != NULL) && ssl_mode) /* If proxy AND ssl, must send a CONNECT message to the proxy */
        {
          char errormsg[1024];
#ifdef __unix__
          if(SU_DebugLevel >= 1)
          {
            printf("SkyUtils_SU_ExecuteActions : Sending SSL CONNECT to the proxy, for %s:%d\n",((SU_PHTTPActions)Ptr->Data)->Host,GetPortFromHost(((SU_PHTTPActions)Ptr->Data)->Host,true));
          }
#endif /* __unix__ */
          if(!SU_SendProxySSLConnect(Sock,((SU_PHTTPActions)Ptr->Data)->Host,GetPortFromHost(((SU_PHTTPActions)Ptr->Data)->Host,true),&Code))
          {
            printf("SkyUtils_SU_ExecuteActions Error : Cannot send CONNECT message to the proxy : Code=%d\n",Code);
            return(-1);
          }
#ifdef __unix__
          if(SU_DebugLevel >= 1)
          {
            printf("SkyUtils_SU_ExecuteActions : SSL CONNECT successfully sent !\n");
          }
#endif /* __unix__ */
          ssl = SU_SSL_Connect(Sock,errormsg);
          if(ssl == NULL)
          {
            printf("SkyUtils_SU_ExecuteActions Error : Cannot create SSL connection : %s\n",errormsg);
            return(-1);
          }
          proxy_string = NULL;
          /* Transforming URL as if we were not using a proxy */
          Code = GetHostFromURL(((SU_PHTTPActions)Ptr->Data)->URL,((SU_PHTTPActions)Ptr->Data)->Host,sizeof(((SU_PHTTPActions)Ptr->Data)->Host),false,URL_OUT,&PortConnect,"",&ssl_mode);
          if(Code != 0)
            return Code;
          SU_strcpy(((SU_PHTTPActions)Ptr->Data)->URL,URL_OUT,sizeof(URL_OUT));
        }
#endif /* SU_USE_SSL */
        if(SendCommand(Sock,(SU_PHTTPActions)Ptr->Data,(proxy_string != NULL),ssl_mode?ssl:NULL)) /* If SSL && proxy, simulate NO PROXY */
        {
          Ans = WaitForAnswer(Sock,((SU_PHTTPActions)Ptr->Data),(proxy_string != NULL),ssl_mode?ssl:NULL);
          if(Ans == NULL)
          {
            printf("SkyUtils_SU_ExecuteActions Error : Connection timed out\n");
            return(-2);
          }
          if(((SU_PHTTPActions)Ptr->Data)->CB.OnAnswer != NULL)
            ((SU_PHTTPActions)Ptr->Data)->CB.OnAnswer(Ans,((SU_PHTTPActions)Ptr->Data)->User);
#ifdef __unix__
          if(SU_DebugLevel >= 2)
            printf("SkyUtils_SU_ExecuteActions : Found Code : %d\n",Ans->Code);
#endif /* __unix__ */
          switch(Ans->Code)
          {
            case 200 : /* Ok reply */
              if(((SU_PHTTPActions)Ptr->Data)->CB.OnOk != NULL)
                ((SU_PHTTPActions)Ptr->Data)->CB.OnOk(Ans,((SU_PHTTPActions)Ptr->Data)->User);
              break;
            case 201 : /* Created */
              if(((SU_PHTTPActions)Ptr->Data)->CB.OnCreated != NULL)
                ((SU_PHTTPActions)Ptr->Data)->CB.OnCreated(Ans,((SU_PHTTPActions)Ptr->Data)->User);
              break;
            case 202 : /* Modified */
              if(((SU_PHTTPActions)Ptr->Data)->CB.OnModified != NULL)
                ((SU_PHTTPActions)Ptr->Data)->CB.OnModified(Ans,((SU_PHTTPActions)Ptr->Data)->User);
              break;
            case 301 : /* Moved */
            case 302 : /* Moved */
            case 303 : /* Moved */
              if(((SU_PHTTPActions)Ptr->Data)->CB.OnMoved != NULL)
                ((SU_PHTTPActions)Ptr->Data)->CB.OnMoved(Ans,((SU_PHTTPActions)Ptr->Data)->User);
              memset(&Act,0,sizeof(Act));
              if(((SU_PHTTPActions)Ptr->Data)->FileName)
                Act.FileName = SU_strdup(((SU_PHTTPActions)Ptr->Data)->FileName);
              Act.User = ((SU_PHTTPActions)Ptr->Data)->User;
              memcpy(&Act.CB,&((SU_PHTTPActions)Ptr->Data)->CB,sizeof(Act.CB));
              Act.Command = ACT_GET;
              if(SU_nocasestrstr(Ans->Location,"http://") != Ans->Location) /* Relative path */
              {
                if(SU_nocasestrstr(Ans->Location,"https://") != Ans->Location) /* Really a relative path */
                {
                  ptr = SU_AddLocationToUrl(((SU_PHTTPActions)Ptr->Data)->URL,((SU_PHTTPActions)Ptr->Data)->Host,Ans->Location,((SU_PHTTPActions)Ptr->Data)->SSL);
                  free(Ans->Location);
                  Ans->Location = ptr;
                }
              }
              /* Let say we use a proxy, so we have less code to execute :o) */
              Code = GetHostFromURL(Ans->Location,((SU_PHTTPActions)Ptr->Data)->Host,sizeof(Act.Host),true,Act.URL,&PortConnect,"",&ssl_mode);
              if(Code != 0)
                return Code;
              SU_EncodeURL(Ans->Location,Act.URL,sizeof(Act.URL));
              Act.URL_Params = NULL;
              if(((SU_PHTTPActions)Ptr->Data)->Referer != NULL)
                Act.Referer = ((SU_PHTTPActions)Ptr->Data)->Referer;
              else
                Act.Referer = ((SU_PHTTPActions)Ptr->Data)->URL;
              ActRec = SU_AddElementHead(NULL,&Act);
              SU_ExecuteActions(ActRec);
              if(Act.FileName)
                free(Act.FileName);
              ActRec = SU_DelElementHead(ActRec);
              break;
            case 403 : /* Forbidden */
              if(((SU_PHTTPActions)Ptr->Data)->CB.OnForbidden != NULL)
                ((SU_PHTTPActions)Ptr->Data)->CB.OnForbidden(Ans,((SU_PHTTPActions)Ptr->Data)->User);
              break;
            case 404 : /* Page Not Found */
              if(((SU_PHTTPActions)Ptr->Data)->CB.OnNotFound != NULL)
                ((SU_PHTTPActions)Ptr->Data)->CB.OnNotFound(Ans,((SU_PHTTPActions)Ptr->Data)->User);
              break;
            case 413 : /* Request entity too large */
              if(((SU_PHTTPActions)Ptr->Data)->CB.OnTooBig != NULL)
                ((SU_PHTTPActions)Ptr->Data)->CB.OnTooBig(Ans,((SU_PHTTPActions)Ptr->Data)->User);
              break;
            case 503 : /* Unknown Host */
              if(((SU_PHTTPActions)Ptr->Data)->CB.OnUnknownHost != NULL)
                ((SU_PHTTPActions)Ptr->Data)->CB.OnUnknownHost(Ans,((SU_PHTTPActions)Ptr->Data)->User);
              break;
            default : /* Other */
              if(((SU_PHTTPActions)Ptr->Data)->CB.OnOtherReply != NULL)
                ((SU_PHTTPActions)Ptr->Data)->CB.OnOtherReply(Ans,Ans->Code,((SU_PHTTPActions)Ptr->Data)->User);
              break;
          }
          FreeAnswer(Ans);
        }
        break;
      default :
        printf("SkyUtils_SU_ExecuteActions Warning : Unknown Action !!\n");
    }
    Ptr = Ptr->Next;
  }
  return 0;
}

void SU_FreePart(SU_PHTTPPart Part)
{
  if(Part->Header)
    free(Part->Header);
  if(Part->FileName)
    free(Part->FileName);
  if(Part->Data)
    free(Part->Data);
  free(Part);
}

SKYUTILS_API void SU_FreeAction(SU_PHTTPActions Act)
{
  if(Act->URL_Params != NULL)
    free(Act->URL_Params);
  if(Act->Post_Data != NULL)
    free(Act->Post_Data);
  if(Act->FileName != NULL)
    free(Act->FileName);
  if(Act->Referer != NULL)
    free(Act->Referer);
  if(Act->ContentType)
    free(Act->ContentType);
  if(Act->MultiParts)
  {
    SU_PList Ptr = Act->MultiParts;
    while(Ptr != NULL)
    {
      SU_FreePart((SU_PHTTPPart)Ptr->Data);
      Ptr = Ptr->Next;
    }
    SU_FreeList(Act->MultiParts);
  }
  free(Act);
}

SKYUTILS_API SU_PInput SU_GetInput(char *html)
{
  SW_GetInput_String = html;
  return SU_GetNextInput();
}

SKYUTILS_API SU_PInput SU_GetNextInput(void)
{
  char *p,*ps,*pt,*q,*r,*s,*tmp,buf[500];
  char c,toto[3],res;
  int len;
  SU_PInput In;
  bool textarea = false;

  p = SU_nocasestrstr(SW_GetInput_String,"<input");
  ps = SU_nocasestrstr(SW_GetInput_String,"<select");
  pt = SU_nocasestrstr(SW_GetInput_String,"<textarea");
  if((pt != NULL) && ((pt<p) || (p == NULL)) && ((pt<ps) || (ps == NULL))) /* Textarea found first */
  {
    p = pt+3; /* +3 to adjust from sizeof("textarea") to sizeof("input") */
    textarea = true;
  }
  if(((p>ps) || (p == NULL)) && (ps != NULL))
    p = ps+1; /* +1 to adjust from sizeof("select") to sizeof("input") */
  if(p == NULL)
    return NULL;
  s = p;
  In = (SU_PInput) malloc(sizeof(SU_TInput));
  memset(In,0,sizeof(SU_TInput));
  p+=7;
  r = strchr(p,'>');
  /* Now parse input tags */
  toto[0] = '=';
  toto[1] = ' ';
  toto[2] = 0;
  while(p[0] != '>')
  {
    while(*p == ' ')
      p++;
    q = SU_strchrl(p,toto,&res);
    if(q == NULL)
      break;
    if(q > r) /* Attention ici, si on veux plus tard recup les non Name=Value */
      break;
    len = (int)(q-p);
    if(len >= sizeof(buf))
      len = sizeof(buf) - 1;
    memcpy(buf,p,len);
    buf[len] = 0;
    /* buf contient la partie Name de Name=Value */
    p = SU_TrimLeft(q + 1);
    if(res == ' ')
    {
      if(p[0] != '=')
        continue;
      else
        p = SU_TrimLeft(p+1);
    }
    while((len > 0) && (buf[len-1] == ' '))
    {
      len--;
      buf[len] = 0; /* Remove trailing spaces */
    }
    if((strchr(buf,' ') == NULL) && (res != '>')) /* Si on a bien a faire a un Name=Value */
    {
      if(p[0] == '"') /* Si la partie Value est une chaine */
      {
        c = '"';
        p++;
      }
      else if(p[0] == '\'') /* Si la partie Value est une chaine */
      {
        c = '\'';
        p++;
      }
      else
        c = ' ';
      q = strchr(p,c);
      if(q == NULL)
        break;
      if(q > r)
      {
        if((c == '"') || (c == '\'')) /* '>' must be inside the string */
          r = strchr(r+1,'>');
        else
          q = r;
      }
      len = (int)(q-p);
      if(len <= 0)
        continue;
      tmp = (char *) malloc(len+1);
      memcpy(tmp,p,len);
      tmp[len] = 0;
      p = q;
      if((c == '"') || (c == '\'')) /* Si la partie Value est une chaine */
        p++;
      if(SU_nocasestrstr(buf,"type") == buf)
        In->Type = tmp;
      else if(SU_nocasestrstr(buf,"name") == buf)
        In->Name = tmp;
      else if(SU_nocasestrstr(buf,"value") == buf)
        In->Value = tmp;
      else
        free(tmp);
    }
  }
  if(textarea)
  {
    if(In->Type == NULL)
      In->Type = SU_strdup((char *)"textarea");
    p = SU_nocasestrstr(r+1,"</textarea>");
    if(p == NULL)
    {
      if(In->Name != NULL)
      {
        free(In->Name);
        In->Name = NULL;
      }
    }
    else
    {
      if(In->Value != NULL)
        free(In->Value);
      In->Value = (char *) malloc(p-r);
      SU_strcpy(In->Value,r+1,p-r);
      r = p+2;
    }
  }
  if(r != NULL)
    SW_GetInput_String = r;
  else
    SW_GetInput_String = s+6;
  if(In->Name == NULL)
  {
    SU_FreeInput(In);
    return SU_GetNextInput();
  }
  return In;
}

SKYUTILS_API void SU_FreeInput(SU_PInput In)
{
  if(In->Type != NULL)
    free(In->Type);
  if(In->Name != NULL)
    free(In->Name);
  if(In->Value != NULL)
    free(In->Value);
  free(In);
}

SKYUTILS_API SU_PImage SU_GetImage(char *html)
{
  SW_GetImage_String = html;
  return SU_GetNextImage();
}

SKYUTILS_API SU_PImage SU_GetNextImage(void)
{
  char *p,*q,*tmp;
  int len;
  char c;
  SU_PImage Im;

  p = SU_nocasestrstr(SW_GetImage_String,"img src");
  if(p == NULL)
    return NULL;
  Im = (SU_PImage) malloc(sizeof(SU_TImage));
  memset(Im,0,sizeof(SU_TImage));
  p+=7;
  while(*p == ' ')
    p++;
  p++; /* zap le '=' */
  while(*p == ' ')
    p++; /* zap les espaces apres le '=' */
  if(*p == '"')
  {
    c = '"';
    p++; /* zap le '"' si c'est une chaine */
  }
  else if(*p == '\'')
  {
    c = '\'';
    p++; /* zap le '\'' si c'est une chaine */
  }
  else
    c = ' ';
  q = strchr(p,c);
  len = (int)(q-p);
  tmp = (char *) malloc(len+1);
  memcpy(tmp,p,len);
  tmp[len] = 0;
  p = q;
  if((c == '"') || (c == '\'')) /* Si la partie Value est une chaine */
    p++;
  Im->Src = tmp;
  while(p[0] != '>')
  {
    /* Faudrait boucler ici pour recup le Name eventuellement */
    p++;
  }

  SW_GetImage_String = p;
  return Im;
}

SKYUTILS_API void SU_FreeImage(SU_PImage Im)
{
  if(Im->Src != NULL)
    free(Im->Src);
  if(Im->Name != NULL)
    free(Im->Name);
  free(Im);
}

SKYUTILS_API SU_PHTTPActions SU_RetrieveLink(const char URL[],const char Ans[],const char link[],const int index)
{
  char *p,*q,c,*tmp,*tmp2,*rp,*rs;
  SU_PHTTPActions Act;
  int i;
  bool found;

  p = (char *)Ans-1;
  for(i=1;i<=index;i++)
    p = strstr(p+1,link);
  if(p == NULL)
    return NULL;
  while(strncasecmp(p,"href",4) != 0)
    p--;
  p+=4;
  p = SU_TrimLeft(p); /* Remove spaces */
  p++; /* Zap '=' */
  p = SU_TrimLeft(p); /* Remove spaces */
  if(p[0] == '"')
  {
    c = '"';
    p++; /* Zap '"' */
  }
  else if(p[0] == '\'')
  {
    c = '\'';
    p++; /* Zap '\'' */
  }
  else
    c = ' ';
  q = strchr(p,c);
  tmp = (char *) malloc(q-p+1);
  SU_strcpy(tmp,p,q-p+1);

  Act = (SU_PHTTPActions) malloc(sizeof(SU_THTTPActions));
  memset(Act,0,sizeof(SU_THTTPActions));
  Act->Command = ACT_GET;
  /* URL in tmp, but may be relative */
  if(strncasecmp(tmp,"http",4) == 0) /* Absolute */
    strncpy(Act->URL,tmp,sizeof(Act->URL));
  else
  {
    if(tmp[0] == '/') /* Root of the host */
    {
#ifdef __unix__
      tmp2 = strchr(URL+7,'/');
#else /* !__unix__ */
      tmp2 = strchr((char *)URL+7,'/');
#endif /* __unix__ */
      if(tmp2 == NULL) /* Already at the root of the site */
      {
        SU_strcpy(Act->URL,URL,sizeof(Act->URL));
        SU_strcat(Act->URL,tmp,sizeof(Act->URL));
      }
      else
      {
        if((tmp2-URL+1) >= sizeof(Act->URL))
          printf("SkyUtils_SU_RetrieveLink Warning : URL replacement in SU_RetrieveLink is bigger than sizeof(URL). Result should be unpredictable\n");
        else
          SU_strcpy(Act->URL,URL,tmp2-URL+1); /* Copy the root part of URL */
        SU_strcat(Act->URL,tmp,sizeof(Act->URL));
      }
    }
    else
    {
      tmp2 = tmp;
      strncpy(Act->URL,URL,sizeof(Act->URL));
      /* If / at the end of URL, remove it */
      if(Act->URL[strlen(Act->URL)-1] == '/')
        Act->URL[strlen(Act->URL)-1] = 0;
      /* If end of URL if a file, remove it */
      rp = strrchr(Act->URL,'.');
      rs = strrchr(Act->URL,'/');
      if(rp > rs)
        rs[0] = 0;
      /* For each ../ remove it from URL */
      while(strncasecmp(tmp2,"../",3) == 0)
      {
        tmp2+=3;
        i = (int)(strlen(Act->URL) - 1);
        found = false;
        while(i >= 0)
        {
          if(Act->URL[i] == '/')
          {
            found = true;
            Act->URL[i] = 0;
            break;
          }
          i--;
        }
        if(!found)
        {
          free(tmp);
          free(Act);
          return NULL;
        }
      }
      /* If no / at the end of URL, add it */
      if(Act->URL[strlen(Act->URL)-1] != '/')
        SU_strcat(Act->URL,"/",sizeof(Act->URL));
      /* Cat URL and dest */
      SU_strcat(Act->URL,tmp2,sizeof(Act->URL));
    }
  }
  free(tmp);
  return Act;
}

/* Code added by Pierre Bacquet (pbacquet@delta.fr) */
SKYUTILS_API SU_PHTTPActions SU_RetrieveFrame(const char URL[],const char Ans[],const char framename[])
{
  char *p,*q,c,*tmp,*tmp2,*rp,*rs;
  SU_PHTTPActions Act;
  int i;
  bool found;
  char pattern[1024];

  SU_snprintf(pattern,sizeof(pattern),"FRAME NAME=%s", framename);

  p = SU_nocasestrstr((char *)Ans,pattern);
  if(p == NULL)
    return NULL;
  while(strncasecmp(p,"src",3) != 0)
    p++;
  p+=3;
  p = SU_TrimLeft(p); /* Remove spaces */
  p++; /* Zap '=' */
  p = SU_TrimLeft(p); /* Remove spaces */
  if(p[0] == '"')
  {
    c = '"';
    p++; /* Zap '"' */
  }
  else if(p[0] == '\'')
  {
    c = '\'';
    p++; /* Zap '\'' */
  }
  else
    c = ' ';
  q = strchr(p,c);
  tmp = (char *) malloc(q-p+1);
  SU_strcpy(tmp,p,q-p+1);

  Act = (SU_PHTTPActions) malloc(sizeof(SU_THTTPActions));
  memset(Act,0,sizeof(SU_THTTPActions));
  Act->Command = ACT_GET;
  /* URL in tmp, but may be relative */
  if(strncasecmp(tmp,"http",4) == 0) /* Absolute */
    strncpy(Act->URL,tmp,sizeof(Act->URL));
  else
  {
    if(tmp[0] == '/') /* Root of the host */
    {
#ifdef __unix__
      tmp2 = strchr(URL+7,'/');
#else /* !__unix__ */
      tmp2 = strchr((char *)URL+7,'/');
#endif /* __unix__ */
      if(tmp2 == NULL) /* Already at the root of the site */
      {
        SU_strcpy(Act->URL,URL,sizeof(Act->URL));
        SU_strcat(Act->URL,tmp,sizeof(Act->URL));
      }
      else
      {
        if((tmp2-URL+1) >= sizeof(Act->URL))
          printf("SkyUtils_SU_RetrieveFrame Warning : URL replacement in SU_RetrieveFrame is bigger than sizeof(URL). Result should be unpredictable\n");
        else
          SU_strcpy(Act->URL,URL,tmp2-URL+1); /* Copy the root part of URL */
        SU_strcat(Act->URL,tmp,sizeof(Act->URL));
      }
    }
    else
    {
      tmp2 = tmp;
      strncpy(Act->URL,URL,sizeof(Act->URL));
      /* If / at the end of URL, remove it */
      if(Act->URL[strlen(Act->URL)-1] == '/')
        Act->URL[strlen(Act->URL)-1] = 0;
      /* If end of URL if a file, remove it */
      rp = strrchr(Act->URL,'.');
      rs = strrchr(Act->URL,'/');
      if(rp > rs)
        rs[0] = 0;
      /* For each ../ remove it from URL */
      while(strncasecmp(tmp2,"../",3) == 0)
      {
        tmp2+=3;
        i = (int)(strlen(Act->URL) - 1);
        found = false;
        while(i >= 0)
        {
          if(Act->URL[i] == '/')
          {
            found = true;
            Act->URL[i] = 0;
            break;
          }
          i--;
        }
        if(!found)
        {
          free(tmp);
          free(Act);
          return NULL;
        }
      }
      /* If no / at the end of URL, add it */
      if(Act->URL[strlen(Act->URL)-1] != '/')
        SU_strcat(Act->URL,"/",sizeof(Act->URL));
      /* Cat URL and dest */
      SU_strcat(Act->URL,tmp2,sizeof(Act->URL));
    }
  }
  free(tmp);
  return Act;
}

/* Retrieve document.forms[num] */
SKYUTILS_API SU_PForm SU_RetrieveForm(const char Ans[],const int num)
{
  char *p,*ps,*pt,*q,*r,*saf,*parse,*tmp,c,buf[500];
  int i,len;
  SU_PInput In;
  SU_PForm Form;
  SU_PList Ptr;
  char toto[3],res;
  bool textarea = false;

  p = SU_nocasestrstr((char *)Ans,"<form");
  if(p == NULL)
    return NULL;
  for(i=0;i<num;i++)
  {
    p = SU_nocasestrstr(p,"/form");
    if(p == NULL)
      return NULL;
    p = SU_nocasestrstr(p,"<form");
    if(p == NULL)
      return NULL;
  }
  q = SU_nocasestrstr(p,"/form");
  if(q == NULL)
    return NULL;
  saf = (char *) malloc(q-p+1);
  toto[1] = '>';
  toto[2] = 0;
  SU_strcpy(saf,p,q-p+1);
  /* Got the full form in saf */
  parse = saf;
  Form = (SU_PForm) malloc(sizeof(SU_TForm));
  memset(Form,0,sizeof(SU_TForm));
  Ptr = NULL;
  p = SU_TrimLeft(parse+5);
  /* Now parse form tag */
  while(p[0] != '>')
  {
    if(strncasecmp(p,"method",6) == 0)
    {
      p = SU_TrimLeft(p+6);
      p++; /* Zap '=' */
      p = SU_TrimLeft(p);
      if(p[0] == '"')
      {
        c = '"';
        p++; /* Zap '"' */
      }
      else if(p[0] == '\'')
      {
        c = '\'';
        p++; /* Zap '\'' */
      }
      else
        c = ' ';
      toto[0] = c;
      q = SU_strchrl(p,toto,&res);
      if(q == NULL)
        break;
      tmp = (char *) malloc(q-p+1);
      SU_strcpy(tmp,p,q-p+1);
      Form->Method = tmp;
      p = q;
      if((c == '"') || (c == '\''))
        p++; /* Zap '"' */
    }
    else if(strncasecmp(p,"name",4) == 0)
    {
      p = SU_TrimLeft(p+4);
      p++; /* Zap '=' */
      p = SU_TrimLeft(p);
      if(p[0] == '"')
      {
        c = '"';
        p++; /* Zap '"' */
      }
      else if(p[0] == '\'')
      {
        c = '\'';
        p++; /* Zap '\'' */
      }
      else
        c = ' ';
      toto[0] = c;
      q = SU_strchrl(p,toto,&res);
      if(q == NULL)
        break;
      tmp = (char *) malloc(q-p+1);
      SU_strcpy(tmp,p,q-p+1);
      Form->Name = tmp;
      p = q;
      if((c == '"') || (c == '\''))
        p++; /* Zap '"' */
    }
    else if(strncasecmp(p,"action",6) == 0)
    {
      p = SU_TrimLeft(p+6);
      p++; /* Zap '=' */
      p = SU_TrimLeft(p);
      if(p[0] == '"')
      {
        c = '"';
        p++; /* Zap '"' */
      }
      else if(p[0] == '\'')
      {
        c = '\'';
        p++; /* Zap '\'' */
      }
      else
        c = ' ';
      toto[0] = c;
      q = SU_strchrl(p,toto,&res);
      if(q == NULL)
        break;
      tmp = (char *) malloc(q-p+1);
      SU_strcpy(tmp,p,q-p+1);
      Form->Action = tmp;
      p = q;
      if((c == '"') || (c == '\''))
        p++; /* Zap '"' */
    }
    else
    {
      q = strchr(p,' ');
      r = strchr(p,'>');
      if((q == NULL) || (r == NULL))
        break;
      if(r < q)
        break;
      else
        p = q;
    }
    p = SU_TrimLeft(p);
  }
#ifdef __unix__
  if(SU_DebugLevel >= 3)
    printf("SkyUtils_SU_RetrieveForm : Infos for forms[%d] : Method=%s - Name=%s - Action=%s\n",num,(Form->Method == NULL)?"(null)":Form->Method,(Form->Name == NULL)?"(null)":Form->Name,(Form->Action == NULL)?"(null)":Form->Action);
#endif /* __unix__ */

  p = SU_nocasestrstr(parse,"<input");
  ps = SU_nocasestrstr(parse,"<select");
  pt = SU_nocasestrstr(parse,"<textarea");
  if((pt != NULL) && ((pt<p) || (p == NULL)) && ((pt<ps) || (ps == NULL))) /* Textarea found first */
  {
    p = pt+3; /* +3 to adjust from sizeof("textarea") to sizeof("input") */
    textarea = true;
  }
  if(((p>ps) || (p == NULL)) && (ps != NULL))
    p = ps+1; /* +1 to adjust from sizeof("select") to sizeof("input") */
  while(p != NULL)
  {
    In = (SU_PInput) malloc(sizeof(SU_TInput));
    memset(In,0,sizeof(SU_TInput));
    p = SU_TrimLeft(p+6);
    /* Now parse input tags */
    r = strchr(p,'>');
    toto[0] = '=';
    toto[1] = ' ';
    while(p[0] != '>')
    {
      q = SU_strchrl(p,toto,&res);
      if(q == NULL)
        break;
      if(q > r) /* Attention ici, si on veux plus tard recup les non Name=Value */
        break;
      len = (int)(q-p);
      if(len >= sizeof(buf))
        len = sizeof(buf) - 1;
      memcpy(buf,p,len);
      buf[len] = 0;
      /* buf contient la partie Name de Name=Value */
      p = SU_TrimLeft(q + 1);
      if(res == ' ')
      {
        if(p[0] != '=')
          continue;
        else
          p = SU_TrimLeft(p+1);
      }
      while((len > 0) && (buf[len-1] == ' '))
      {
        len--;
        buf[len] = 0; /* Remove trailing spaces */
      }
      if((strchr(buf,' ') == NULL) && (res != '>')) /* Si on a bien a faire a un Name=Value */
      {
        if(p[0] == '"') /* Si la partie Value est une chaine */
        {
          c = '"';
          p++;
        }
        else if(p[0] == '\'') /* Si la partie Value est une chaine */
        {
          c = '\'';
          p++;
        }
        else
          c = ' ';
        q = strchr(p,c);
        if(q == NULL)
          q = r;
        if(q > r)
        {
          if((c == '"') || (c == '\'')) /* '>' must be inside the string */
            r = strchr(r+1,'>');
          else
            q = r;
        }
        len = (int)(q-p);
        if(len <= 0)
          continue;
        tmp = (char *) malloc(len+1);
        memcpy(tmp,p,len);
        tmp[len] = 0;
        p = q;
        if((c == '"') || (c == '\'')) /* Si la partie Value est une chaine */
          p++;
        if(SU_nocasestrstr(buf,"type") == buf)
          In->Type = tmp;
        else if(SU_nocasestrstr(buf,"name") == buf)
          In->Name = tmp;
        else if(SU_nocasestrstr(buf,"value") == buf)
          In->Value = tmp;
        else
          free(tmp);
      }
      p = SU_TrimLeft(p);
    }
    if(textarea)
    {
      if(In->Type == NULL)
        In->Type = SU_strdup((char *)"textarea");
      q = SU_nocasestrstr(p+1,"</textarea>");
      if(q == NULL)
      {
        if(In->Name != NULL)
        {
          free(In->Name);
          In->Name = NULL;
        }
      }
      else
      {
        if(In->Value != NULL)
          free(In->Value);
        In->Value = (char *) malloc(q-p);
        SU_strcpy(In->Value,r+1,q-p);
        p = q+2;
      }
    }
    if(In->Type == NULL)
      In->Type = SU_strdup((char *)"text");
    if(In->Name != NULL)
    {
#ifdef __unix__
      if(SU_DebugLevel >= 3)
        printf("SkyUtils_SU_RetrieveForm : Adding INPUT to form[%d] : Type=%s - Name=%s - Value=%s\n",num,(In->Type == NULL)?"(null)":In->Type,(In->Name == NULL)?"(null)":In->Name,(In->Value == NULL)?"(null)":In->Value);
#endif /* __unix__ */
      Ptr = SU_AddElementHead(Ptr,In);
    }
    else
      SU_FreeInput(In);

    textarea = false;
    parse = p+1; /* Set parse to the end of INPUT (after the '>') */
    p = SU_nocasestrstr(parse,"<input");
    ps = SU_nocasestrstr(parse,"<select");
    pt = SU_nocasestrstr(parse,"<textarea");
    if((pt != NULL) && ((pt<p) || (p == NULL)) && ((pt<ps) || (ps == NULL))) /* Textarea found first */
    {
      p = pt+3; /* +3 to adjust from sizeof("textarea") to sizeof("input") */
      textarea = true;
    }
    if(((p>ps) || (p == NULL)) && (ps != NULL))
      p = ps+1; /* +1 to adjust from sizeof("select") to sizeof("input") */
  }
  free(saf);
  Form->Inputs = Ptr;
  return Form;
}

SKYUTILS_API void SU_FreeForm(SU_PForm Form)
{
  SU_PList Ptr;

  Ptr = Form->Inputs;
  while(Ptr != NULL)
  {
    SU_FreeInput((SU_PInput)Ptr->Data);
    Ptr = Ptr->Next;
  }
  SU_FreeList(Form->Inputs);
  if(Form->Method != NULL)
    free(Form->Method);
  if(Form->Name != NULL)
    free(Form->Name);
  if(Form->Action != NULL)
    free(Form->Action);
}

SKYUTILS_API char *SU_AddLocationToUrl(const char *URL,const char *Host,const char *Location,bool ssl_mode)
{
  char *ptr = NULL;
  int len,i,pos = 0;

  if(strncasecmp(Location,"http://",7) != 0) /* Relative path */
  {
    len = (int)(strlen(Host)+strlen(URL)+strlen(Location)+strlen("https://")+1);
    ptr = (char *) malloc(len);
    if(Location[0] == '/')
    { /* Relative path, but absolute on the site */
      SU_snprintf(ptr,len,"http%s://%s",ssl_mode?"s":"",Host);
      /* Remove trailing / if exists */
      if(ptr[strlen(ptr)-1] == '/' )
        ptr[strlen(ptr)-1] = 0;
    }
    else
    { /* Relative path from current directory */
      if(strncasecmp(URL,"http://",7) == 0) /* If using proxy, or if URL is already absolute */
        SU_strcpy(ptr,URL,len);
      else if(strncasecmp(URL,"https://",8) == 0) /* If using proxy, or if URL is already absolute - SSL */
        SU_strcpy(ptr,URL,len);
      else
        SU_snprintf(ptr,len,"http%s://%s%s",ssl_mode?"s":"",Host,URL);

      if(strcmp(ptr+strlen("http://")+(ssl_mode?1:0),Host) == 0) /* If requested the root of the site */
        SU_strcat(ptr,"/",len);
      else
      {
        i = (int)(strlen(ptr) - 1);
        while(i>=0)
        {
          if(ptr[i] == '/')
          {
            ptr[i+1] = 0;
            break;
          }
          i--;
        }
      }
      /* Here, ptr have a trailing '/' */
      /* Check for '../' in Location */
      while(strncmp(Location+pos,"../",3) == 0)
      {
        i = (int)(strlen(ptr) - 1 - 1); /* Start from before the trailing '/' */
        while(i>=0)
        {
          if(ptr[i] == '/')
          {
            ptr[i+1] = 0; /* Go back a directory level */
            break;
          }
          i--;
        }
        pos += 3;
      }
    }
    SU_strcat(ptr,Location+pos,len);
  }
  else
    ptr = SU_strdup(Location);
  return ptr;
}

/* Skips white spaces before the string, then extracts it */
SKYUTILS_API char *SU_GetStringFromHtml(const char Ans[],const char TextBefore[])
{
  char *p,*q,*tmp;
  char c;
  int len;

  p = strstr(Ans,TextBefore);
  if(p == NULL)
    return NULL;
  p += strlen(TextBefore);
  while(p[0] == ' ') /* Remove spaces */
    p++;

  if(p[0] == '"') /* If we have a string */
  {
    c = '"';
    p++;
  }
  else if(p[0] == '\'') /* If we have a string */
  {
    c = '\'';
    p++;
  }
  else
    c = ' ';
  q = strchr(p,c);
  if(q == NULL)
    return NULL;
  len = (int)(q-p);
  tmp = (char *) malloc(len+1);
  memcpy(tmp,p,len);
  tmp[len] = 0;
  return tmp;
}

SKYUTILS_API void SU_SetUserAgent(const char UA[])
{
  if(SW_UserAgent != NULL)
    free(SW_UserAgent);
  if(UA == NULL)
    SW_UserAgent = NULL;
  else
    SW_UserAgent = SU_strdup(UA);
}
