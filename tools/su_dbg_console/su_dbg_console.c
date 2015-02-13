/* SkyUtils Remote debug console */
/* (c) Christophe Calmejane - 2002-07 */

#include <skyutils/skyutils.h>

#ifdef _WIN32
#define ERROR_MSG(x) MessageBox(NULL,x,"SU_DBG_CONSOLE",MB_OK)
#else /* !_WIN32 */
#define ERROR_MSG(x) printf("su_dbg_console error : %s\n",x)
#endif /* _WIN32 */

#define MAXCOLORS 13

int ColorTable[MAXCOLORS] = {0,
                                  1,
                                  2,
                                  4,
                                  3,
                                  6,
                                  5,
                                  1,
                                  2,
                                  4,
                                  3,
                                  6,
                                  5,
                                 };

bool UseAnsi = false;

int GetColorFromFlag(int val)
{
  int i = 0;

  val>>=1;
  while(val)
  {
    i++;
    val>>=1;
  }
  if(i >= MAXCOLORS)
    return 0;
  return ColorTable[i];
}

void GetSocketMessages(SU_SOCKET sock)
{
  int res;
  SU_u32 len,total;
  SU_u64 Type;
  char buf[4096];
  int color;

  while(1)
  {
    /* Receiving size of message */
    res = recv(sock,(char *)&len,sizeof(len),SU_MSG_NOSIGNAL);
    if(res != sizeof(len))
    {
      return;
    }
    /* Receiving type of message */
    res = recv(sock,(char *)&Type,sizeof(Type),SU_MSG_NOSIGNAL);
    if(res != sizeof(Type))
    {
      return;
    }
    /* Receiving message */
    total = 0;
    while(total < len)
    {
      res = recv(sock,buf+total,len-total,SU_MSG_NOSIGNAL);
      if(res <= 0)
        return;
      total += res;
    }

    buf[total] = 0;
    if(UseAnsi)
    {
      color = GetColorFromFlag((int)Type);
      if(color == 0)
        printf("\033[3%d;4" SU_ANSI_WHITE "m" "%s" SU_ANSI_RESET "\n",color,buf);
      else
      {
        if(Type > 32)
          printf("\033[3%d;4" SU_ANSI_BLACK "m" SU_ANSI_HIGHLIGHT "%s" SU_ANSI_RESET "\n",color,buf);
        else
          printf("\033[3%d;4" SU_ANSI_BLACK "m" "%s" SU_ANSI_RESET "\n",color,buf);
      }
    }
    else
      printf("%s\n",buf);
  }
}

void PrintHelp(void)
{
  printf("su_dbg_console usage :\n");
  printf("  su_dbg_console <Port> [-ansi]\n\n");
  exit(-1);
}

int main(int argc,char *argv[])
{
  int value;
  SU_PServerInfo SI;
  SU_PClientSocket CS;

  if((argc < 2) || (argc > 3))
    PrintHelp();
  if(argc == 3)
  {
    if(strcmp(argv[2],"-ansi") != 0)
      PrintHelp();
    UseAnsi = true;
  }
  
#ifdef _WIN32
  if(!SU_SockInit(2,2))
  {
    ERROR_MSG("Cannot load winsock2");
    return 1;
  }
#endif
  value = atoi(argv[1]);
  if(value == 0)
  {
    ERROR_MSG("Bad port value");
    return 2;
  }
  
  SI = SU_CreateServer(value,SOCK_STREAM,false);
  if(SI == NULL)
  {
    ERROR_MSG("Error creating socket server");
    return 3;
  }
  if(SU_ServerListen(SI) == SOCKET_ERROR)
  {
    ERROR_MSG("Error listening on socket");
    return 4;
  }
  
  while(1)
  {
    printf("su_dbg_console : Listening on port %d\n",value);
    CS = SU_ServerAcceptConnection(SI);
    if(CS == NULL)
    {
      ERROR_MSG("Error accepting connection");
      return 5;
    }
    /* Ok client connected */
    printf("su_dbg_console : Connected to %s\n",inet_ntoa(CS->SAddr.sin_addr));
    /* Retreive messages */
    GetSocketMessages(CS->sock);
    /* Client disconnected */
    printf("Application disconnected (%s). Closing link\n",inet_ntoa(CS->SAddr.sin_addr));
    SU_FreeCS(CS);
  }
  return 0;
}
