/****************************************************************/
/* Debug header unit                                            */
/* (c) Christophe CALMEJANE (Ze KiLleR) - 1999-2014             */
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

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <skyutils/skyutils.h>

#define SU_DBG_MAX_SOCKETS 4 /* Don't forget to change initialisation of SU_DBG_OUT_SOCKET_Socks array */
typedef struct
{
  SU_u16 Output;
  char *Name;
} SU_DBG_TOutputName;


/* *** Global Debug Variables *** */
extern SU_u64 SU_DBG_Flags;
extern SU_u16 SU_DBG_Output;
extern bool SU_DBG_OPT_Time;
extern bool SU_DBG_OPT_ThreadId;
extern bool SU_DBG_OPT_ProcessId;
/* *** Output Specific variables *** */
  /* Printf output */
  extern bool SU_DBG_OUT_PRINTF_Color;
  /* File output */
  extern FILE *SU_DBG_OUT_FILE_File;
  extern char *SU_DBG_OUT_FILE_FileName;
  extern bool SU_DBG_OUT_FILE_DeletePreviousLog;
  /* Console output */
#ifdef _WIN32
  extern HWND SU_DBG_OUT_CONSOLE_Hwnd;
  extern UINT SU_DBG_OUT_CONSOLE_Msg;
  extern char *SU_DBG_OUT_CONSOLE_Name;
#endif /* _WIN32 */
  /* Socket output */
  extern SU_SOCKET SU_DBG_OUT_SOCKET_Socks[SU_DBG_MAX_SOCKETS];


void SU_DBG_Init(void);


#endif /* !__DEBUG_H__ */
