/****************************************************************/
/* Debug Config unit                                            */
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

#ifdef _WIN32

#include "debug.h"
#include "Windows\\Skyutils\\su_resource.h"

#ifndef SU_TRACE_INTERNAL
#undef malloc
#undef calloc
#undef realloc
#undef strdup
#undef free
#endif /* !SU_TRACE_INTERNAL */

static bool _su_dbg_options_changed = false;

/* INTERNAL FUNCTIONS */

LRESULT CALLBACK __SU_DBG_ChooseDebugOptions_Windows_Routine(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDOK:
        {
          char flags[50],console[150],filepath[512];
          char tmp[200];
          SU_u64 flags_64;

          SU_u16 output = SU_DBG_OUTPUT_NONE;
          /* Check if options are valid */
          GetDlgItemText(hwnd,(int)MAKEINTRESOURCE(IDC_EDIT_FLAGS),flags,sizeof(flags));
          if(sscanf(flags,"%I64i",&flags_64) != 1)
          {
            MessageBox(hwnd,"Flags value is not valid","SU_DBG_ChooseDebugOptions Error",MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TOPMOST);
            return TRUE;
          }
          GetDlgItemText(hwnd,(int)MAKEINTRESOURCE(IDC_EDIT_CONSOLE_NAME),console,sizeof(console));
          if(IsDlgButtonChecked(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_CONSOLE)) == BST_CHECKED)
          {
            int count = 0;
            if(console[0] == 0)
            {
              MessageBox(hwnd,"You must enter a name for the console","SU_DBG_ChooseDebugOptions Error",MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TOPMOST);
              return TRUE;
            }
            /* Don't know why, but the message box returns immediatly, on the second time we run the ChooseDebugOption dialog */
            while(FindWindow(NULL,console) == NULL)
            {
              SU_snprintf(tmp,sizeof(tmp),"Cannot find SkyUtils Debug Console of name '%s'\nStart it then clic on RETRY, or clic on CANCEL to change the name",console);
              if(MessageBox(hwnd,tmp,"SU_DBG_ChooseDebugOptions Error",MB_RETRYCANCEL | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TOPMOST) != IDRETRY)
                return TRUE;
              count++;
              if(count >= 5)
                return TRUE;
            }
          }
          GetDlgItemText(hwnd,(int)MAKEINTRESOURCE(IDC_EDIT_LOG_PATH),filepath,sizeof(filepath));

          /* *** Set new options *** */
          /* ** Global ** */
          SU_DBG_SetOptions(IsDlgButtonChecked(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_TIME)) == BST_CHECKED,IsDlgButtonChecked(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_PROCESS)) == BST_CHECKED,IsDlgButtonChecked(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_THREAD)) == BST_CHECKED);
          SU_DBG_SetFlags(flags_64);
          if(IsDlgButtonChecked(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_PRINTF)) == BST_CHECKED)
            SetFlag(output,SU_DBG_OUTPUT_PRINTF);
          if(IsDlgButtonChecked(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_CONSOLE)) == BST_CHECKED)
            SetFlag(output,SU_DBG_OUTPUT_CONSOLE);
          if(IsDlgButtonChecked(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_FILE)) == BST_CHECKED)
            SetFlag(output,SU_DBG_OUTPUT_FILE);
          if(IsDlgButtonChecked(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_SOCKET)) == BST_CHECKED)
            SetFlag(output,SU_DBG_OUTPUT_SOCKET);
          if(IsDlgButtonChecked(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_POPUP)) == BST_CHECKED)
            SetFlag(output,SU_DBG_OUTPUT_POPUP);
          SU_DBG_SetOutput(output);
          /* ** Specific ** */
          /* Printf Output Options */
          SU_DBG_OUT_PRINTF_SetOptions(IsDlgButtonChecked(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_ANSI)) == BST_CHECKED);
          /* Console Output Options */
          SU_DBG_OUT_CONSOLE_SetOptions(console);
          /* File Output Options */
          SU_DBG_OUT_FILE_SetOptions(filepath,IsDlgButtonChecked(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_DELETE_PREVIOUS)) == BST_CHECKED);
          /* Socket Output Options */
          /* Not done yet */
          
          _su_dbg_options_changed = true;
          /* Close box */
          DestroyWindow(hwnd);
          return TRUE;
        }
        case IDCANCEL:
          /* Close box */
          DestroyWindow(hwnd);
          return TRUE;
      }
      break;

    case WM_CLOSE:
      /* Close box */
      DestroyWindow(hwnd);
      return TRUE;
    case WM_DESTROY:
      //PostQuitMessage(0); /* DON'T POST QUIT, SINCE WE ARE NOT THE MAIN APP !! */
      return TRUE;
  }
  return FALSE;
}

/* EXPORTED FUNCTIONS */
SKYUTILS_API bool SU_DBG_ChooseDebugOptions_Windows(HINSTANCE hInstance,HWND Parent)
{
  char buf[200];
  MSG msg;
  HWND hwnd = CreateDialog(hInstance,MAKEINTRESOURCE(IDD_SU_CHOOSE_OPT),Parent,__SU_DBG_ChooseDebugOptions_Windows_Routine);

  _su_dbg_options_changed = false;
  if(hwnd == NULL)
  {
    SU_snprintf(buf,sizeof(buf),"SU_DBG_ChooseDebugOptions_Windows : Cannot load dialog resource : %s\n",SU_strerror(GetLastError()));
    MessageBox(hwnd,buf,"SU_DBG_ChooseDebugOptions Error",MB_OK | MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
    return false;
  }

  /* Load env vars default values (if not done yet) */
  SU_DBG_Init();

  /* *** Init default values *** */
  /* ** Global ** */
  if(SU_DBG_Output & SU_DBG_OUTPUT_PRINTF)
    CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_PRINTF),BST_CHECKED);
  else
    CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_PRINTF),BST_UNCHECKED);
  if(SU_DBG_Output & SU_DBG_OUTPUT_CONSOLE)
    CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_CONSOLE),BST_CHECKED);
  else
    CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_CONSOLE),BST_UNCHECKED);
  if(SU_DBG_Output & SU_DBG_OUTPUT_FILE)
    CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_FILE),BST_CHECKED);
  else
    CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_FILE),BST_UNCHECKED);
  /*if(SU_DBG_Output & SU_DBG_OUTPUT_SOCKET)
    CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_SOCKET),BST_CHECKED);
  else*/ // Not done yet
    CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_SOCKET),BST_UNCHECKED);
  if(SU_DBG_Output & SU_DBG_OUTPUT_POPUP)
    CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_POPUP),BST_CHECKED);
  else
    CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_POPUP),BST_UNCHECKED);

  if(SU_DBG_OPT_Time)
    CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_TIME),BST_CHECKED);
  else
    CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_TIME),BST_UNCHECKED);
  if(SU_DBG_OPT_ThreadId)
    CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_THREAD),BST_CHECKED);
  else
    CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_THREAD),BST_UNCHECKED);
  if(SU_DBG_OPT_ProcessId)
    CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_PROCESS),BST_CHECKED);
  else
    CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_PROCESS),BST_UNCHECKED);
  SU_snprintf(buf,sizeof(buf),"0x%016I64X",SU_DBG_Flags);
  SetDlgItemText(hwnd,(int)MAKEINTRESOURCE(IDC_EDIT_FLAGS),buf);

  /* ** Specific ** */
  /* Printf output */
  if(SU_DBG_OUT_PRINTF_Color)
    CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_ANSI),BST_CHECKED);
  else
    CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_ANSI),BST_UNCHECKED);

  /* File output */
  if(SU_DBG_OUT_FILE_DeletePreviousLog)
    CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_DELETE_PREVIOUS),BST_CHECKED);
  else
    CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHK_DELETE_PREVIOUS),BST_UNCHECKED);
  if(SU_DBG_OUT_FILE_FileName)
    SetDlgItemText(hwnd,(int)MAKEINTRESOURCE(IDC_EDIT_LOG_PATH),SU_DBG_OUT_FILE_FileName);
  else
    SetDlgItemText(hwnd,(int)MAKEINTRESOURCE(IDC_EDIT_LOG_PATH),"");

  /* Console output */
  if(SU_DBG_OUT_CONSOLE_Name)
    SetDlgItemText(hwnd,(int)MAKEINTRESOURCE(IDC_EDIT_CONSOLE_NAME),SU_DBG_OUT_CONSOLE_Name);
  else
    SetDlgItemText(hwnd,(int)MAKEINTRESOURCE(IDC_EDIT_CONSOLE_NAME),SU_DBG_CONSOLE_DEFAULT_WINDOW_NAME);

  /* Socket output */
  /* Not done yet */

  SendMessage(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_EDIT_FLAGS)),EM_SETLIMITTEXT,16+2,0);
  /* Lets show the dialog */
  ShowWindow(hwnd ,SW_SHOW);
  while(GetMessage(&msg,hwnd ,0,0) && msg.message)
  {
    if(!IsWindow(hwnd ) || !IsDialogMessage(hwnd ,&msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return _su_dbg_options_changed;
}

#endif /* _WIN32 */

