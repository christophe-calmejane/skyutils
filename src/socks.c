/****************************************************************/
/* Socket unit - TCP/UDP                                        */
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

#ifndef SU_TRACE_INTERNAL
#undef malloc
#undef calloc
#undef realloc
#undef strdup
#undef free
#endif /* !SU_TRACE_INTERNAL */

SKYUTILS_API int SU_GetPortByName(char *port,char *proto)
{
  struct servent *PSE;

  PSE = getservbyname(port,proto);
  if(PSE == NULL)
    return atoi(port);
  return ntohs(PSE->s_port);
}

SKYUTILS_API char *SU_GetMachineName(char *RemoteHost)
{
  char *tmp,*tmp2;
  tmp = strchr(RemoteHost,'.');
  if(tmp == NULL)
  {
    return SU_strdup(RemoteHost);
  }
  tmp2 = (char *)malloc(tmp-RemoteHost+1);
  SU_strcpy(tmp2,RemoteHost,tmp-RemoteHost+1);
  return tmp2;
}

SKYUTILS_API char *SU_NameOfPort(char *Host)
{
  struct hostent *hp;
  struct in_addr inp;

  inp.s_addr = inet_addr(Host);
  if(inp.s_addr == INADDR_NONE)
    return NULL;
  hp = gethostbyaddr((char *)&inp,4,AF_INET);
  if(hp == NULL)
    return NULL;
  return (char *)hp->h_name;
}

SKYUTILS_API char *SU_AdrsOfPort(char *Host)
{
  struct hostent *hp;
  struct in_addr inp;

  hp = gethostbyname(Host);
  if(hp == NULL)
    return NULL;
  memcpy((void *)&inp, hp->h_addr, hp->h_length);
  return inet_ntoa(inp);
}

SKYUTILS_API unsigned int SU_GetSocketPort(SU_SOCKET sock)
{
  SU_SOCKLEN_T len;
  struct sockaddr_in SAddr;

  len = sizeof(SAddr);
  if(getsockname(sock,(struct sockaddr *)&SAddr,&len) != -1)
  {
    return ntohs(SAddr.sin_port);
  }
  return 0;
}

/* 
 Sends a buffer to socket.
 If 'packet_size' is not 0, sub buffers of this size will be issued to the socket layer
 Returns : 
  0  : Socket closed
  -1 : Read error on socket (see SU_errno)
  >0 : Total of bytes sent
*/
SKYUTILS_API int SU_SendTCPBuffer(SU_SOCKET sock,char *buf,int size,int packet_size)
{
  int written;
  int write_len = size;
  int total = 0;
  int ret = 0;
  int to_write;
  
  if(packet_size == 0) /* Don't care about packet_size, force it to buffer size */
  {
    packet_size = write_len;
  }

  while(write_len > 0)
  {
    to_write = write_len;
    if(to_write > packet_size)
      to_write = packet_size;
    written = send(sock,buf+size-write_len,to_write,SU_MSG_NOSIGNAL);
    if(written > 0)
    {
      total += written;
      write_len -= written;
      ret = total;
    }
    else
    {
      if(SU_errno == SU_EAGAIN) /* Ignore retry error */
        continue;
      else
      {
        ret = written;
        break; /* But break on fatal */
      }
    }
  }
  return ret;
}

/* 
 Reads a buffer from socket. If TimeOut is not NULL, select-timeout is used. If WaitForFullBuffer is true, don't return until error, or 'size' bytes has been read
 Returns : 
  0  : Socket closed
  -1 : Read error on socket (see SU_errno)
  -2 : Read timed out (value defined in TimeOut parameter)
  >0 : Total of bytes read
*/
SKYUTILS_API int SU_ReadTCPBuffer(SU_SOCKET sock,char *buf,int size,struct timeval *TimeOut,bool WaitForFullBuffer)
{
  int lus;
  int lus_len = size;
  int total = 0;
  fd_set rfds;
  int retval;
  int ret = 0;
  
  while(lus_len > 0)
  {
    FD_ZERO(&rfds);
    FD_SET(sock,&rfds);
    retval = select(sock+1,&rfds,NULL,NULL,TimeOut);
    if(retval == 0) /* Time out */
    {
      ret = -2;
      break;
    }
    else if(retval == -1) /* Select error */
    {
#ifndef _WIN32
      if(errno == EINTR) /* 'Select' might be interrupted by a system call, we need to ignore the returned error */
        continue;
#endif /* !_WIN32 */
      ret = -1;
      break;
    }

    lus = recv(sock,buf+size-lus_len,lus_len,SU_MSG_NOSIGNAL);
    if(lus > 0)
    {
      total += lus;
      lus_len -= lus;
      ret = total;
      if(!WaitForFullBuffer) /* Don't need full size, return now */
        break;
    }
    else
    {
      if(lus == -1 && SU_errno == SU_EAGAIN) /* Ignore retry error */
      {
        continue;
      }
      else
      {
        ret = lus;
        break; /* But break on fatal */
      }
    }
  }
  return ret;
}

/* 
 Select on multiple socket at the same time, for a read operation. If TimeOut is not NULL, select-timeout is used. CB is called for each socket that is ready for a read.
 Returns : 
  0  : Function completed OK
  -1 : Read error on socket (see SU_errno)
  -2 : Read timed out (value defined in TimeOut parameter)
*/
SKYUTILS_API int SU_SelectMultiSocketForRead(SU_PMultiSocket socks,struct timeval *TimeOut,SU_MULTISOCK_CB_FUNC *CB)
{
  fd_set rfds;
  unsigned int n = 0;
  int ret,i;

  while(true)
  {
    /* Prepare select */
    FD_ZERO(&rfds);
    for(i=0;i<socks->nb_sons;i++)
    {
      if((socks->sons[i] != NULL) && (socks->sons[i]->sock != SU_NOT_A_SOCKET))
      {
        FD_SET(socks->sons[i]->sock,&rfds);
        n = MAX(n,socks->sons[i]->sock);
      }
    }
    ret = select(n+1,&rfds,NULL,NULL,TimeOut);
    if(ret == 0) /* Time out */
    {
      return -2;
    }
    else if(ret == -1) /* Select error */
    {
#ifndef _WIN32
      if(errno == EINTR) /* 'Select' might be interrupted by a system call, we need to ignore the returned error */
        continue;
#endif /* !_WIN32 */
      return -1;
    }
    break; /* Only here to handle the EINTR error */
  }

  /* Check all fds */
  for(i=0;i<socks->nb_sons;i++)
  {
    if((socks->sons[i] != NULL) && (socks->sons[i]->sock != SU_NOT_A_SOCKET) && FD_ISSET(socks->sons[i]->sock,&rfds))
    {
      CB(socks,socks->sons[i]);
    }
  }
  return 0;
}
/* 
 Sends a buffer to a multiple socket struct.
 If 'packet_size' is not 0, sub buffers of this size will be issued to the socket layer
 Returns (in rets) : 
  0  : Socket closed
  -1 : Read error on socket (see SU_errno)
  >0 : Total of bytes sent
*/
SKYUTILS_API void SU_SendTCPBufferToMultiSocket(SU_PMultiSocket socks,char *buf,int size,int packet_size,int *rets)
{
  int i;
  int ret;

  if(rets != NULL)
    memset(rets,0,sizeof(int)*socks->nb_sons);

  for(i=0;i<socks->nb_sons;i++)
  {
    if((socks->sons[i] != NULL) && (socks->sons[i]->sock != SU_NOT_A_SOCKET))
    {
      ret = SU_SendTCPBuffer(socks->sons[i]->sock,buf,size,packet_size);
      if(rets != NULL)
        rets[i] = ret;
    }
  }
}


/*------------------------------------------------------------*/
SKYUTILS_API SU_PServerInfo SU_CreateServer(int port,int type,bool ReUseAdrs)
{
  SU_PServerInfo SI;
  SU_SOCKLEN_T len;

  SI = (SU_PServerInfo) malloc(sizeof(SU_TServerInfo));
  memset(SI,0,sizeof(SU_TServerInfo));
  if( type == SOCK_STREAM )
    SI->sock = socket(AF_INET,type,getprotobyname("tcp")->p_proto);
  else if( type == SOCK_DGRAM )
    SI->sock = socket(AF_INET,type,getprotobyname("udp")->p_proto);
  else
    return NULL;
  if(SI->sock == SU_INVALID_SOCKET)
  {
    free(SI);
    return NULL;
  }
  memset(&(SI->SAddr),0,sizeof(struct sockaddr_in));
#ifdef __unix__
  if(ReUseAdrs)
  {
    len = sizeof(struct sockaddr_in);
    if(getsockname(SI->sock,(struct sockaddr *)&(SI->SAddr),&len) == -1)
    {
      int err = SU_errno; /* We need to backup last error, because it will be overriden by the following SU_CLOSE_SOCKET call */
      SU_CLOSE_SOCKET(SI->sock);
      free(SI);
      SU_seterrno(err); /* Restore last error */
      return NULL;
    }
    len = 1;
    setsockopt(SI->sock,SOL_SOCKET,SO_REUSEADDR,(char *)&len,sizeof(len));
  }
#endif /* __unix__ */
  SI->SAddr.sin_family = AF_INET;
  SI->SAddr.sin_port = htons(port);
  SI->SAddr.sin_addr.s_addr = 0;
  if(bind(SI->sock,(struct sockaddr *)&(SI->SAddr), sizeof(SI->SAddr)) == -1)
  {
    int err = SU_errno; /* We need to backup last error, because it will be overriden by the following SU_CLOSE_SOCKET call */
    SU_CLOSE_SOCKET(SI->sock);
    free(SI);
    SU_seterrno(err); /* Restore last error */
    return NULL;
  }

#ifdef _WIN32
  if(ReUseAdrs)
  {
    len = sizeof(struct sockaddr_in);
    if(getsockname(SI->sock,(struct sockaddr *)&(SI->SAddr),&len) == -1)
    {
      int err = SU_errno; /* We need to backup last error, because it will be overriden by the following SU_CLOSE_SOCKET call */
      SU_CLOSE_SOCKET(SI->sock);
      free(SI);
      SU_seterrno(err); /* Restore last error */
      return NULL;
    }
    len = 1;
    setsockopt(SI->sock,SOL_SOCKET,SO_REUSEADDR,(char *)&len,sizeof(len));
  }
#endif /* _WIN32 */

  return SI;
}

SKYUTILS_API int SU_ServerListen(SU_PServerInfo SI)
{
  if(SI == NULL)
    return SOCKET_ERROR;
  if(listen(SI->sock,SOMAXCONN) == -1)
    return SOCKET_ERROR;
  return 0;
}

SKYUTILS_API SU_PClientSocket SU_ServerAcceptConnection(SU_PServerInfo SI)
{
  struct sockaddr sad;
  SU_SOCKLEN_T len;
  SU_SOCKET tmpsock;
  SU_PClientSocket CS;

  if(SI == NULL)
    return NULL;
  len = sizeof(sad);
  while(true)
  {
    tmpsock = accept(SI->sock,&sad,&len);
    if(tmpsock == SU_INVALID_SOCKET)
    {
#ifndef _WIN32
      if(errno == EINTR) /* 'Accept' might be interrupted by a system call, we need to ignore the returned error */
        continue;
#endif /* !_WIN32 */
      return NULL;
    }
    break; /* Only here to handle the EINTR error */
  }
  CS = (SU_PClientSocket) malloc(sizeof(SU_TClientSocket));
  memset(CS,0,sizeof(SU_TClientSocket));
  CS->sock = tmpsock;
  memcpy(&CS->SAddr,&sad,sizeof(CS->SAddr));
  return CS;
}

SKYUTILS_API int SU_ServerAcceptConnectionWithTimeout(SU_PServerInfo SI,struct timeval *TimeOut,SU_PClientSocket *NewSon) /* Returns -1 on error, 0 on timeout, 1 if son accepted */
{
  *NewSon = NULL;

  if(SI == NULL)
    return -1;

  if(TimeOut != NULL)
  {
    while(true)
    {
      /* Prepare select */
      int ret;
      fd_set rfds;
      FD_ZERO(&rfds);
      FD_SET(SI->sock,&rfds);
      ret = select((int)SI->sock+1,&rfds,NULL,NULL,TimeOut);
      if(ret == 0) /* Time out */
      {
        return 0;
      }
      else if(ret == SU_INVALID_SOCKET) /* Select error */
      {
#ifndef _WIN32
        if(errno == EINTR) /* 'Select' might be interrupted by a system call, we need to ignore the returned error */
          continue;
#endif /* !_WIN32 */
        return -1;
      }
      break; /* Only here to handle the EINTR error */
    }
  }
  *NewSon = SU_ServerAcceptConnection(SI);
  if(*NewSon == NULL) /* Error during accept, must return an error */
    return -1;
  return 1; /* Success */
}

SKYUTILS_API void SU_ServerDisconnect(SU_PServerInfo SI)
{
  if(SI == NULL || SI->sock == SU_NOT_A_SOCKET)
    return;
  SU_CLOSE_SOCKET(SI->sock);
  SI->sock = SU_NOT_A_SOCKET;
}

SKYUTILS_API void SU_FreeSI(SU_PServerInfo SI)
{
  SU_ServerDisconnect(SI);
  free(SI);
}

/*------------------------------------------------------------*/
SKYUTILS_API SU_PClientSocket SU_ClientConnect(char *adrs,char *port,int type)
{
  struct servent *SE;
  struct sockaddr_in sin;
  struct hostent *HE;
  SU_PClientSocket CS;

  CS = (SU_PClientSocket) malloc(sizeof(SU_TClientSocket));
  memset(CS,0,sizeof(SU_TClientSocket));
  if(type == SOCK_STREAM)
    CS->sock = socket(AF_INET,SOCK_STREAM,getprotobyname("tcp")->p_proto);
  else if(type == SOCK_DGRAM)
    CS->sock = socket(AF_INET,SOCK_DGRAM,getprotobyname("udp")->p_proto);
  else
    return NULL;
  if(CS->sock == SU_INVALID_SOCKET)
  {
    free(CS);
    return NULL;
  }
  sin.sin_family = AF_INET;
  if(type == SOCK_STREAM)
    SE = getservbyname(port,"tcp");
  else if(type == SOCK_DGRAM)
    SE = getservbyname(port,"udp");
  else
    return NULL;
  if(SE == NULL)
    sin.sin_port = htons(atoi(port));
  else
    sin.sin_port = SE->s_port;
  sin.sin_addr.s_addr = inet_addr(adrs);
  if(sin.sin_addr.s_addr == INADDR_NONE)
  {
    HE = gethostbyname(adrs);
    if(HE == NULL)
    {
      /*printf("SkyUtils_ClientConnect Warning: Unknown Host: %s\n",adrs);*/
      SU_seterrno(ENXIO);
      return NULL;
    }
    sin.sin_addr = *(struct in_addr *)(HE->h_addr_list[0]);
  }
  if(connect(CS->sock,(struct sockaddr *)(&sin),sizeof(sin)) == -1)
  {
    int err = SU_errno; /* We need to backup last error, because it will be overriden by the following SU_CLOSE_SOCKET call */
    SU_CLOSE_SOCKET(CS->sock);
    free(CS);
    SU_seterrno(err); /* Restore last error */
    return NULL;
  }
  memcpy(&CS->SAddr,&sin,sizeof(CS->SAddr));
  return CS;
}

SKYUTILS_API int SU_ClientSend(SU_PClientSocket CS,char *msg)
{
  if(CS == NULL)
    return SOCKET_ERROR;
  return send(CS->sock,msg,strlen(msg),SU_MSG_NOSIGNAL);
}

SKYUTILS_API int SU_ClientSendBuf(SU_PClientSocket CS,char *buf,int len)
{
  if(CS == NULL)
    return SOCKET_ERROR;
  return send(CS->sock,buf,len,SU_MSG_NOSIGNAL);
}

SKYUTILS_API void SU_ClientDisconnect(SU_PClientSocket CS)
{
  if(CS == NULL || CS->sock == SU_NOT_A_SOCKET)
    return;
  SU_CLOSE_SOCKET(CS->sock);
  CS->sock = SU_NOT_A_SOCKET;
}

SKYUTILS_API void SU_FreeCS(SU_PClientSocket CS)
{
  SU_ClientDisconnect(CS);
  free(CS);
}

/*------------------------------------------------------------*/
SKYUTILS_API int SU_UDPSendBroadcast(SU_PServerInfo SI,char *Text,int len,char *port)
{
  struct sockaddr_in sin;
  int i;
  int total = 0,packet;

  if(SI == NULL)
    return SOCKET_ERROR;
#ifdef DEBUG
  {
    SU_SOCKLEN_T si = sizeof(int);
    if(getsockopt(SI->sock,SOL_SOCKET,SO_BROADCAST,(char *)&i,&si) == SOCKET_ERROR)
      return SOCKET_ERROR;
    if(i == 0)
    {
      printf("SkyUtils_SendBroadcast Warning: SO_BROADCAST is OFF... must have been set ON before\n");
      return SOCKET_ERROR;
    }
  }
#endif /* DEBUG */
  sin.sin_family = AF_INET;
  sin.sin_port = htons(atoi(port));
  sin.sin_addr.s_addr = INADDR_BROADCAST;

  i = 0;
  while(len > 0)
  {
    packet = len;
    if(packet > SU_UDP_MAX_LENGTH)
      packet = SU_UDP_MAX_LENGTH;
    i += sendto(SI->sock,Text+total,packet,0,(struct sockaddr *)&sin,sizeof(sin));
    total += packet;
    len -= packet;
    if(len != 0)
      SU_USLEEP(500); /* Sleep for 500 msec */
  }
  return i;
}

SKYUTILS_API int SU_UDPSendToAddr(SU_PServerInfo SI,char *Text,int len,char *Addr,char *port)
{
  int i;
  struct sockaddr_in sin;
  struct hostent *PHE;
  int total = 0,packet;

  if(SI == NULL)
    return SOCKET_ERROR;
  sin.sin_addr.s_addr = inet_addr(Addr);
  if(sin.sin_addr.s_addr == INADDR_NONE)
  {
    PHE = gethostbyname(Addr);
    if(PHE == NULL)
      return SOCKET_ERROR;
    sin.sin_addr = *(struct in_addr *)(PHE->h_addr_list[0]);
  }
  sin.sin_family = AF_INET;
  sin.sin_port = htons(atoi(port));

  i = 0;
  while(len > 0)
  {
    packet = len;
    if(packet > SU_UDP_MAX_LENGTH)
      packet = SU_UDP_MAX_LENGTH;
    i += sendto(SI->sock,Text+total,packet,0,(struct sockaddr *)&sin,sizeof(sin));
    total += packet;
    len -= packet;
    if(len != 0)
      SU_USLEEP(500); /* Sleep for 500 msec */
  }
  return i;
}

SKYUTILS_API int SU_UDPSendToSin(SU_PServerInfo SI,char *Text,int len,struct sockaddr_in sin)
{
  int i = 0;
  int total = 0,packet;

  if(SI == NULL)
    return SOCKET_ERROR;
  while(len > 0)
  {
    packet = len;
    if(packet > SU_UDP_MAX_LENGTH)
      packet = SU_UDP_MAX_LENGTH;
    i += sendto(SI->sock,Text+total,packet,0,(struct sockaddr *)&sin,sizeof(struct sockaddr_in));
    total += packet;
    len -= packet;
    if(len != 0)
      SU_USLEEP(500); /* Sleep for 500 msec */
  }
  return i;
}

SKYUTILS_API int SU_UDPReceiveFrom(SU_PServerInfo SI,char *Text,int len,char **ip,int Blocking)
{
   struct sockaddr_in sin;
   SU_SOCKLEN_T ssin;
   int i;
   struct hostent *hp;

  if(SI == NULL)
    return SOCKET_ERROR;
  if(!Blocking)
  {
#ifdef _WIN32
    i = 1;
    ioctlsocket(SI->sock,FIONBIO,(unsigned long *)&i);
#else /* !_WIN32 */
    fcntl(SI->sock,F_SETFL,O_NONBLOCK);
#endif /* _WIN32 */
  }
  ssin = sizeof(sin);
  i = recvfrom(SI->sock,Text,len,SU_MSG_NOSIGNAL,(struct sockaddr *)&sin,&ssin);
  if(i == SOCKET_ERROR)
    return SOCKET_ERROR;

  hp = gethostbyaddr((char *)&sin.sin_addr,4,AF_INET);
  if(hp == NULL)
    return i;
  *ip = (char *)hp->h_name;

  return i;
}

SKYUTILS_API int SU_UDPReceiveFromSin(SU_PServerInfo SI,char *Text,int len,struct sockaddr_in *ret_sin,int Blocking)
{
   struct sockaddr_in sin;
   int i;
   SU_SOCKLEN_T ssin;

  if(SI == NULL)
    return SOCKET_ERROR;
  if(!Blocking)
  {
#ifdef _WIN32
    i = 1;
    ioctlsocket(SI->sock,FIONBIO,(unsigned long *)&i);
#else /* !_WIN32 */
    fcntl(SI->sock,F_SETFL,O_NONBLOCK);
#endif /* _WIN32 */
  }
  ssin = sizeof(sin);
  i = recvfrom(SI->sock,Text,len,SU_MSG_NOSIGNAL,(struct sockaddr *)&sin,&ssin);
  if(i == SOCKET_ERROR)
    return SOCKET_ERROR;

  memcpy(ret_sin,&sin,sizeof(sin));
  return i;
}

SKYUTILS_API int SU_SetSocketOpt(SU_SOCKET sock,int Opt,int value)
{
  return setsockopt(sock,SOL_SOCKET,Opt,(char *)&value,sizeof(value));
}

SKYUTILS_API int SU_SetTcpOpt(SU_SOCKET sock,int Opt,int value)
{
  return setsockopt(sock,IPPROTO_TCP,Opt,(char *)&value,sizeof(value));
}

SKYUTILS_API bool SU_SetSocketBlocking(SU_SOCKET sock,bool Block)
{
#ifdef _WIN32
  int v = Block?0:1;
	return ioctlsocket(sock,FIONBIO,(unsigned long *)&v) != SOCKET_ERROR;
#else /* !_WIN32 */
  int  ret = 0;
  ret = fcntl (sock, F_GETFL, 0);
  if(ret == -1)
    return -1;
  if(Block)
    ClearFlag(ret,O_NONBLOCK);
  else
    SetFlag(ret,O_NONBLOCK);
  return fcntl(sock,F_SETFL,ret) != SOCKET_ERROR;
#endif /* _WIN32 */
}

SKYUTILS_API bool SU_SplitIPv4(const char *IPin,unsigned char IPout[4])
{
	unsigned int f1,f2,f3,f4;
#define CHECK_INVALID_VALUE(x) if(x >= 256) return false
	
	if(sscanf(IPin,"%d.%d.%d.%d",&f1,&f2,&f3,&f4) != 4)
		return false;
	
	CHECK_INVALID_VALUE(f1);
	CHECK_INVALID_VALUE(f2);
	CHECK_INVALID_VALUE(f3);
	CHECK_INVALID_VALUE(f4);
	
	IPout[0] = f1;
	IPout[1] = f2;
	IPout[2] = f3;
	IPout[3] = f4;
	
#undef CHECK_INVALID_VALUE
  return true;
}

SKYUTILS_API bool SU_SplitMAC(const char *MACin,unsigned char MACout[6])
{
  unsigned int f1,f2,f3,f4,f5,f6;
#define CHECK_INVALID_VALUE(x) if(x >= 256) return false

  if(sscanf(MACin,"%x:%x:%x:%x:%x:%x",&f1,&f2,&f3,&f4,&f5,&f6) != 6)
    return false;

  CHECK_INVALID_VALUE(f1);
  CHECK_INVALID_VALUE(f2);
  CHECK_INVALID_VALUE(f3);
  CHECK_INVALID_VALUE(f4);
  CHECK_INVALID_VALUE(f5);
  CHECK_INVALID_VALUE(f6);

  MACout[0] = f1;
  MACout[1] = f2;
  MACout[2] = f3;
  MACout[3] = f4;
  MACout[4] = f5;
  MACout[5] = f6;

#undef CHECK_INVALID_VALUE
  return true;
}

SKYUTILS_API bool SU_WakeUpComputer(const char *IP,const char *port,const char *MAC)
{
  SU_PServerInfo SI = SU_CreateServer(0,SOCK_DGRAM,false);
  unsigned char MagicPacket[6+6*16];
  unsigned int i,j;
  unsigned char MACasInt[6];
  bool result = true;

  if(MAC == NULL || MAC[0] == 0)
    return false;
  if(!SU_SplitMAC(MAC,MACasInt))
    return false;

  /* Build magic packet header */
  for(i=0;i<6;i++)
    MagicPacket[i] = 0xFF;

  /* Build magic packet data */
  for(j=0;j<16;j++)
  {
    for(i=0;i<6;i++)
      MagicPacket[6+j*6+i] = MACasInt[i];
  }

  if(port == NULL)
    port = "9";
  if(IP == NULL) /* Broadcast */
  {
    /* Enable broadcast on socket */
    SU_SetSocketOpt(SI->sock,SO_BROADCAST,1);
    /* Send magic packet */
    if(SU_UDPSendBroadcast(SI,(char *)MagicPacket,sizeof(MagicPacket),(char *)port) == -1)
      result = false;
  }
  else /* Targeted IP */
  {
    /* Send magic packet */
    if(SU_UDPSendToAddr(SI,(char *)MagicPacket,sizeof(MagicPacket),(char *)IP,(char *)port) == -1)
      result = false;
  }
  /* Free resources */
  SU_FreeSI(SI);

  return result;
}

#ifdef _WIN32

SKYUTILS_API bool SU_SockInit(int Major,int Minor)
{
  WORD wVersionRequested;
  WSADATA wsaData;
  int err;
  static bool SU_Sock_Init = false;

  if(!SU_Sock_Init)
  {
    wVersionRequested = MAKEWORD( Major, Minor );
    err = WSAStartup(wVersionRequested,&wsaData);
    if(err != 0)
      return false;
    if(LOBYTE( wsaData.wVersion ) != Major || HIBYTE( wsaData.wVersion ) != Minor)
    {
      WSACleanup();
      return false;
    }
    SU_Sock_Init = true;
  }
  return true;
}

SKYUTILS_API void SU_SockUninit(void)
{
  static bool SU_Sock_UnInit = false;

  if(!SU_Sock_UnInit)
  {
    SU_Sock_UnInit = true;
    WSACleanup();
  }
}
#else /* !_WIN32 */

SKYUTILS_API bool SU_SockInit(int Major,int Minor)
{
  return true;
}

SKYUTILS_API void SU_SockUninit(void)
{
}

#endif /* _WIN32 */
