#define SU_DBG_WIN_AUTHOR "Christophe Calméjane"
#define SU_DBG_WIN_VERSION "1.7"

#include <skyutils/skyutils.h>

/* Win32 variables */
#ifdef _WIN32
#include "resource.h" 
#include <shlwapi.h>
volatile HWND L_hwnd = NULL;
UINT L_Msg;
HINSTANCE L_hInstance = 0;
bool Scrolling = true;
bool Capturing = true;
UINT uFindReplaceMsg;
/* Config Gbl Vars */
volatile HWND hConfigWnd = NULL;
volatile HWND hPropertiesWnd = NULL;
/* About Gbl Vars */
volatile HWND hAboutWnd = NULL;
#define ABOUT_TEXT "SkyUtils Debug Console v"SU_DBG_WIN_VERSION"\n(c) "SU_DBG_WIN_AUTHOR" - 2002-2011\n\nThis is part of the SkyUtils library version "SKYUTILS_VERSION"\n(c) "SKYUTILS_AUTHOR" - 1999-2011"
volatile HWND hHelpWnd = NULL;
#define HELP_TEXT "Usage : \nsu_dbg_win.exe [Name] [-pListenPort]\n\nName : Name of this instance of the console\nListenPort : Set console in socket mode, on this port"

/* Find Gbl Vars */
volatile HWND hFindWnd = NULL;
char szFindText[1024];
bool bFinding = false;
bool bFindDown = true;
bool bFindCase = false;
/* Save Glb Vars */
char szSaveFile[1024];

#define ITEMHEIGHT 16
#define MAXCOLORS 24
#define MIN_WIDTH_ALLOWED 375
#define MIN_HEIGHT_ALLOWED 145

COLORREF ColorTable[MAXCOLORS] = {RGB(0,0,0),     /* Noir */
                                  RGB(128,0,0),   /* Rouge foncé */
                                  RGB(0,100,0),   /* Vert foncé */
                                  RGB(0,0,128),   /* Bleu foncé */
                                  RGB(128,128,0), /* Jaune foncé */
                                  RGB(0,100,128), /* Cyan foncé */
                                  RGB(128,0,100), /* Violet foncé */
                                  RGB(255,0,0),   /* Rouge clair */
                                  RGB(0,180,80),  /* Vert clair */
                                  RGB(0,0,255),   /* Bleu clair */
                                  RGB(255,128,0), /* Orange clair */
                                  RGB(0,180,230), /* Cyan clair */
                                  RGB(255,0,255), /* Violet foncé */
                                  RGB(0,0,0),     /* Noir */
                                  RGB(0,0,0),     /* Noir */
                                  RGB(0,0,0),     /* Noir */
                                  RGB(0,0,0),     /* Noir */
                                  RGB(0,0,0),     /* Noir */
                                  RGB(0,0,0),     /* Noir */
                                  RGB(0,0,0),     /* Noir */
                                  RGB(0,0,0),     /* Noir */
                                  RGB(0,0,0),     /* Noir */
                                  RGB(0,0,0),     /* Noir */
                                  RGB(0,0,0)      /* Noir */
                                 };
bool DisplayTable[MAXCOLORS] = {true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true};
char *DisplayTableStrings[MAXCOLORS] = {"Message 0x000001","Message 0x000002","Message 0x000004","Message 0x000008","Message 0x000010","Message 0x000020","Message 0x000040","Message 0x000080","Message 0x000100","Message 0x000200","Message 0x000400","Message 0x000800","Message 0x001000","Message 0x002000","Message 0x004000","Message 0x008000","Message 0x010000","Message 0x020000","Message 0x040000","Message 0x080000","Message 0x100000","Message 0x200000","Message 0x400000","Message 0x800000"};

#else /* !_WIN32 */
/* Linux variables */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <gtk/gtk.h>
#include "interface.h"
#include "support.h"
								 
#define MAXCOLORS 12

volatile GtkWidget *L_hwnd = NULL;
volatile bool MainLoop = false;
GtkWidget *L_hwndErr = NULL;
GtkWidget *L_hwndCfg = NULL;
GtkTextView *L_htxt = NULL;
volatile bool ErrorWindow = false;
SU_SEM_HANDLE L_sem;
SU_PList L_Messages = NULL;
GtkTextIter L_iter;
GtkTextBuffer *L_buf;
typedef struct
{
  char *Name;
  char *Value;
} TColorRef;
typedef struct
{
  char *Msg;
  SU_u64 Type;
} TMessage, *PMessage;
TColorRef ColorTable[MAXCOLORS] = {{"col0","#000000"},
                                                    {"col1","#800000"},
                                                    {"col2","#008000"},
                                                    {"col3","#000080"},
                                                    {"col4","#808000"},
                                                    {"col5","#008080"},
                                                    {"col6","#800080"},
                                                    {"col7","#FF0000"},
                                                    {"col8","#00FF00"},
                                                    {"col9","#0000FF"},
                                                    {"col10","#FFFF00"},
                                                    {"col11","#00FFFF"}
                                                   };
bool DisplayTable[MAXCOLORS] = {true,true,true,true,true,true,true,true,true,true,true,true};
char *DisplayTableStrings[MAXCOLORS] = {"Message 0x00001","Message 0x00002","Message 0x00004","Message 0x00008","Message 0x00010","Message 0x00020","Message 0x00040","Message 0x00080","Message 0x00100","Message 0x00200","Message 0x00400","Message 0x00800"};

#endif /* _WIN32 */

/* Common variables */
int SU_SockPort = 0;
char *SU_ConsoleName = NULL;
int L_x = 0,L_y = 0,L_w = 500,L_h = 300;
#define MAX_CONNS 10
char *ConnStrs[MAX_CONNS];
SU_SOCKET ConnSocks[MAX_CONNS];
int NbConns = 0;

bool LoadCfg(FILE *fp);
void SaveCfg();


/* Win32 code */
#ifdef _WIN32
COLORREF GetColorFromFlag(UINT val)
{
  int i = 0;

  val>>=1;
  while(val)
  {
    i++;
    val>>=1;
  }
  if(i >= MAXCOLORS)
    return RGB(0,0,0);
  return ColorTable[i];
}

bool IsStringDisplayed(SU_u64 val)
{
  int i = 0;

  val>>=1;
  while(val)
  {
    i++;
    val>>=1;
  }
  if(i >= MAXCOLORS)
    return true;
  return DisplayTable[i];
}

void SetDisplayTable(HWND wnd)
{
  int i;
  char buf[200];

  for(i=0;i<MAXCOLORS;i++)
  {
    _snprintf(buf,sizeof(buf),DisplayTableStrings[i]);
    SetDlgItemText(wnd,(int)MAKEINTRESOURCE(IDC_STATIC1+i),buf);
    if(DisplayTable[i])
      CheckDlgButton(wnd,(int)MAKEINTRESOURCE(IDC_CHECK1+i),BST_CHECKED);
    else
      CheckDlgButton(wnd,(int)MAKEINTRESOURCE(IDC_CHECK1+i),BST_UNCHECKED);
  }
}

void UpdateDisplayTable(HWND wnd)
{
  int i;

  for(i=0;i<MAXCOLORS;i++)
  {
    DisplayTable[i] = IsDlgButtonChecked(wnd,(int)MAKEINTRESOURCE(IDC_CHECK1+i)) == BST_CHECKED;
  }
}

void DisplayMessage(char *Msg,SU_u64 Type)
{
  static HWND dlg = 0;
  LRESULT val;

  if(dlg == 0)
  {
    dlg = GetDlgItem(L_hwnd,(int)MAKEINTRESOURCE(IDC_LIST1));
  }

  val = SendMessage(dlg,LB_ADDSTRING,0,(LPARAM)Msg);
  SendMessage(dlg,LB_SETITEMDATA,val,(long)Type);
  if(Scrolling)
    SendMessage(dlg,LB_SETCURSEL,val,0);
}

bool NewThread(SU_THREAD_ROUTINE_TYPE(Entry),void *User)
{
  SU_THREAD_HANDLE Handle;
  SU_THREAD_ID ThreadId;

  Handle = _beginthreadex(NULL,0,Entry,User,0,&ThreadId);
  return (Handle != 0); /* _beginthreadex returns 0 on error, while _beginthread returns -1 */
}

COLORREF ChooseColorMainLoop(HWND parent,COLORREF col)
{
  CHOOSECOLOR cc;
  static COLORREF acrCustClr[16];

  // Initialize CHOOSECOLOR 
  ZeroMemory(&cc, sizeof(cc));
  cc.lStructSize = sizeof(cc);
  cc.hwndOwner = parent;
  cc.lpCustColors = (LPDWORD) acrCustClr;
  cc.rgbResult = col;
  cc.Flags = CC_FULLOPEN | CC_RGBINIT;
 
  if(ChooseColor(&cc) == TRUE)
    return cc.rgbResult;
  else
    return col;
}

LRESULT CALLBACK wndProcProperties(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  LPDRAWITEMSTRUCT lpdis;
  char buf[8192];
  COLORREF oldcolor;

  switch (message) {
    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDOK:
        /* Close box */
        {
          HWND dlg = GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_MSG_COLOR));
          ColorTable[GetWindowLong(hwnd,GWL_USERDATA)] = GetWindowLong(dlg,GWL_USERDATA);
          dlg = GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_MSG_NAME));
          GetWindowText(dlg,buf,sizeof(buf));
          DisplayTableStrings[GetWindowLong(hwnd,GWL_USERDATA)] = strdup(buf);
          dlg = GetDlgItem(hConfigWnd,(int)MAKEINTRESOURCE(IDC_STATIC1+GetWindowLong(hwnd,GWL_USERDATA)));
          SetWindowText(dlg,buf);
          InvalidateRect(dlg,NULL,false);
          DestroyWindow(hwnd);
          PostQuitMessage(0);
          return TRUE;
        }
        case IDCANCEL:
          DestroyWindow(hwnd);
          PostQuitMessage(0);
          return TRUE;
        case IDC_CHANGE_COLOR:
        {
          HWND dlg = GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_MSG_COLOR));
          SetWindowLong(dlg,GWL_USERDATA,ChooseColorMainLoop(hwnd,GetWindowLong(dlg,GWL_USERDATA)));
          InvalidateRect(dlg,NULL,false);
          return TRUE;
        }
      }
      break;
    case WM_CLOSE:
      /* Close box */
      DestroyWindow(hwnd);
      PostQuitMessage(0);
      return TRUE;
    case WM_DESTROY:
      PostQuitMessage(0);
      InvalidateRect(hConfigWnd,NULL,false);
      UpdateWindow(hConfigWnd);
      return TRUE;
    case WM_DRAWITEM:
      lpdis = (LPDRAWITEMSTRUCT) lParam;
      if(lpdis->CtlType != ODT_STATIC)
        return FALSE;
      switch (lpdis->itemAction)
      {
        case ODA_SELECT:
        case ODA_DRAWENTIRE:
        {
          GetWindowText(lpdis->hwndItem,buf,sizeof(buf));
          oldcolor = SetTextColor(lpdis->hDC,GetWindowLong(lpdis->hwndItem,GWL_USERDATA));
          TextOut(lpdis->hDC,2,0,buf,strlen(buf));
          SetTextColor(lpdis->hDC,oldcolor);
          break; 
        }
        case ODA_FOCUS: 
          break; 
      } 
      return TRUE;
    case WM_INITDIALOG:
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_MSG_COLOR)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_MSG_COLOR)),GWL_STYLE) | SS_OWNERDRAW);
      return TRUE;
  }
  return FALSE;
}

SU_THREAD_ROUTINE(PropertiesLoop,info)
{
  HWND parent = hConfigWnd,dlg;
  int idx = (int)info;
  MSG msg;
  char buf[1024];
  BOOL bRet;

  hPropertiesWnd = CreateDialog(L_hInstance,MAKEINTRESOURCE(IDD_DIALOG6),parent,wndProcProperties);
  SetWindowLong(hPropertiesWnd,GWL_USERDATA,idx);
  dlg = GetDlgItem(hPropertiesWnd,(int)MAKEINTRESOURCE(IDC_MSG_COLOR));
  SetWindowLong(dlg,GWL_USERDATA,ColorTable[idx]);
  _snprintf(buf,sizeof(buf),DisplayTableStrings[idx]);
  SetDlgItemText(hPropertiesWnd,(int)MAKEINTRESOURCE(IDC_MSG_NAME),buf);
  ShowWindow(hPropertiesWnd,SW_SHOW);
  while((bRet = GetMessage(&msg,hPropertiesWnd,0,0)) != 0)
  {
    if(bRet == -1) /* Window doesn't exist anymore */
      break;
    if(!IsWindow(hPropertiesWnd) || !IsDialogMessage(hPropertiesWnd,&msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
  hPropertiesWnd = NULL;
  return 0;
}

LRESULT CALLBACK wndProcConfig(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  LPDRAWITEMSTRUCT lpdis;
  char buf[8192];
  COLORREF oldcolor;
  bool do_clic = false;

  switch (message) {
    int i;
    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDOK:
          /* Close box */
          UpdateDisplayTable(hwnd);
          SaveCfg();
          DestroyWindow(hwnd);
          PostQuitMessage(0);
          return TRUE;
        case IDC_BUTTON1:
          for(i=0;i<MAXCOLORS;i++)
            CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHECK1+i),BST_UNCHECKED);
          return TRUE;
        case IDC_BUTTON2:
          for(i=0;i<MAXCOLORS;i++)
            CheckDlgButton(hwnd,(int)MAKEINTRESOURCE(IDC_CHECK1+i),BST_CHECKED);
          return TRUE;
      }
      if(HIWORD(wParam) == STN_CLICKED)
        do_clic = true;
      if(HIWORD(wParam) == STN_DBLCLK)
      {
        if(hPropertiesWnd == NULL)
          NewThread(PropertiesLoop,(void *)(LOWORD(wParam) - IDC_STATIC1));
        else
          SetFocus(hPropertiesWnd);
        do_clic = true;
      }
      if(do_clic)
      {
        int chk = (int)MAKEINTRESOURCE(IDC_CHECK1 + (LOWORD(wParam) - IDC_STATIC1));
        if(IsDlgButtonChecked(hwnd,chk) == BST_CHECKED)
          CheckDlgButton(hwnd,chk,BST_UNCHECKED);
        else
          CheckDlgButton(hwnd,chk,BST_CHECKED);
        return TRUE;
      }
      break;
    case WM_CLOSE:
      /* Close box */
      DestroyWindow(hwnd);
      PostQuitMessage(0);
      return TRUE;
    case WM_DESTROY:
      PostQuitMessage(0);
      UpdateWindow(L_hwnd);
      return TRUE;
    case WM_DRAWITEM:
      lpdis = (LPDRAWITEMSTRUCT) lParam;
      if(lpdis->CtlType != ODT_STATIC)
        return FALSE;
      switch (lpdis->itemAction)
      {
        case ODA_SELECT:
        case ODA_DRAWENTIRE:
          GetWindowText(lpdis->hwndItem,buf,sizeof(buf));
          oldcolor = SetTextColor(lpdis->hDC,ColorTable[lpdis->CtlID-IDC_STATIC1]);
          TextOut(lpdis->hDC,2,0,buf,strlen(buf));
          SetTextColor(lpdis->hDC,oldcolor);
          break; 
        case ODA_FOCUS: 
          break; 
      } 
      return TRUE;
    case WM_INITDIALOG:
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC1)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC1)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC2)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC2)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC3)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC3)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC4)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC4)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC5)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC5)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC6)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC6)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC7)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC7)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC8)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC8)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC9)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC9)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC10)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC10)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC11)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC11)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC12)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC12)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC13)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC13)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC14)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC14)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC15)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC15)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC16)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC16)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC17)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC17)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC18)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC18)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC19)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC19)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC20)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC20)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC21)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC21)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC22)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC22)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC23)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC23)),GWL_STYLE) | SS_OWNERDRAW);
      SetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC24)),GWL_STYLE,GetWindowLong(GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_STATIC24)),GWL_STYLE) | SS_OWNERDRAW);
      return TRUE;
  }
  return FALSE;
}

void Exiting(void)
{
  /* Close box */
  RECT rect;

  if(IsIconic(L_hwnd) ||IsZoomed(L_hwnd))
  {
    ShowWindow(L_hwnd,SW_RESTORE);
  }
  GetWindowRect(L_hwnd,&rect);
  L_x = rect.left;
  L_y = rect.top;
  L_w = rect.right-rect.left;
  L_h = rect.bottom-rect.top;
  DestroyWindow(L_hwnd);
  L_hwnd = NULL;
  PostQuitMessage(0);
}

void FindString(bool findDown)
{
  HWND dlg;
  char buf[2048],*res;
  int startidx,endidx,step,count,i;

  dlg = GetDlgItem(L_hwnd,(int)MAKEINTRESOURCE(IDC_LIST1));
  startidx = SendMessage(dlg,LB_GETCURSEL,0,0);
  if(startidx == LB_ERR)
    startidx = 0;
  count = SendMessage(dlg,LB_GETCOUNT,0,0);
  if(count == LB_ERR)
  {
    MessageBox(L_hwnd,"Error getting list count",SU_DBG_CONSOLE_DEFAULT_WINDOW_NAME,MB_OK);
    return;
  }
  if(findDown)
  {
    step = 1;
    endidx = count;
    startidx++;
  }
  else
  {
    step = -1;
    endidx = -1;
    startidx--;
  }
  for(i=startidx;i!=endidx;i+=step)
  {
    if(SendMessage(dlg,LB_GETTEXT,i,(LPARAM)buf) == LB_ERR)
      break;
    if(bFindCase)
      res = StrStr(buf,szFindText);
    else
      res = StrStrI(buf,szFindText);
    if(res != NULL)
    {
      SendMessage(dlg,LB_SETCURSEL,i,0);
      return;
    }
  }
  _snprintf(buf,sizeof(buf),"Cannot find the string '%s'.",szFindText);
  MessageBox(L_hwnd,buf,SU_DBG_CONSOLE_DEFAULT_WINDOW_NAME,MB_OK | MB_ICONEXCLAMATION);
  return;
}

SU_THREAD_ROUTINE(FindNextMainLoop,info)
{
  if(info == NULL)
    FindString(bFindDown);
  else
    FindString(!bFindDown);
  return 0;
}

SU_THREAD_ROUTINE(ConfigMainLoop,info)
{
  HWND parent = (HWND)info;
  MSG msg;
  BOOL bRet;

  hConfigWnd = CreateDialog(L_hInstance,MAKEINTRESOURCE(IDD_DIALOG2),parent,wndProcConfig);
  SetDisplayTable(hConfigWnd);
  ShowWindow(hConfigWnd,SW_SHOW);
  while((bRet = GetMessage(&msg,hConfigWnd,0,0)) != 0)
  {
    if(bRet == -1) /* Window doesn't exist anymore */
      break;
    if(!IsWindow(hConfigWnd) || !IsDialogMessage(hConfigWnd,&msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
  hConfigWnd = NULL;
  return 0;
}

LRESULT CALLBACK wndProcAbout(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDOK:
          DestroyWindow(hAboutWnd);
          hAboutWnd = NULL;
          PostQuitMessage(0);
          return TRUE;
      }
  }
  return FALSE;
}

SU_THREAD_ROUTINE(AboutMainLoop,info)
{
  HWND parent = (HWND)info,dlg;
  MSG msg;
  BOOL bRet;

  hAboutWnd = CreateDialog(L_hInstance,MAKEINTRESOURCE(IDD_DIALOG4),parent,wndProcAbout);
  dlg = GetDlgItem(hAboutWnd,(int)MAKEINTRESOURCE(IDC_COPYRIGHT));
  SetWindowText(dlg,ABOUT_TEXT);
  ShowWindow(hAboutWnd,SW_SHOW);
  while((bRet = GetMessage(&msg,hAboutWnd,0,0)) != 0)
  {
    if(bRet == -1) /* Window doesn't exist anymore */
      break;
    if(!IsWindow(hAboutWnd) || !IsDialogMessage(hAboutWnd,&msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
  hAboutWnd = NULL;
  return 0;
}

LRESULT CALLBACK wndProcHelp(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDOK:
          DestroyWindow(hHelpWnd);
          hHelpWnd = NULL;
          PostQuitMessage(0);
          return TRUE;
      }
  }
  return FALSE;
}

SU_THREAD_ROUTINE(HelpMainLoop,info)
{
  HWND parent = (HWND)info,dlg;
  MSG msg;
  BOOL bRet;

  hHelpWnd = CreateDialog(L_hInstance,MAKEINTRESOURCE(IDD_DIALOG5),parent,wndProcHelp);
  dlg = GetDlgItem(hHelpWnd,(int)MAKEINTRESOURCE(IDC_HELP1));
  SetWindowText(dlg,HELP_TEXT);
  ShowWindow(hHelpWnd,SW_SHOW);
  while((bRet = GetMessage(&msg,hHelpWnd,0,0)) != 0)
  {
    if(bRet == -1) /* Window doesn't exist anymore */
      break;
    if(!IsWindow(hHelpWnd) || !IsDialogMessage(hHelpWnd,&msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
  hHelpWnd = NULL;
  return 0;
}

SU_THREAD_ROUTINE(FindMainLoop,info)
{
  HWND parent = (HWND)info;
  MSG msg;
  FINDREPLACE fr;
  BOOL bRet;

  // Initialize FINDREPLACE
  ZeroMemory(&fr, sizeof(fr));
  fr.lStructSize = sizeof(fr);
  fr.hwndOwner = parent;
  fr.lpstrFindWhat = szFindText;
  fr.wFindWhatLen = sizeof(szFindText);
  fr.Flags = FR_HIDEWHOLEWORD | FR_DOWN;
  szFindText[0] = 0;
  bFinding = false;

  hFindWnd = FindText(&fr);

  while((bRet = GetMessage(&msg,hFindWnd,0,0)) != 0)
  {
    if(bRet == -1) /* Window doesn't exist anymore */
      break;
    if(!IsWindow(hFindWnd) || !IsDialogMessage(hFindWnd,&msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
  if(bFinding)
    FindString(bFindDown);

  hFindWnd = NULL;
  return 0;
}

void SaveLog()
{
  FILE *fp;
  HWND dlg;
  char buf[2048];
  int count,i,len;
  bool failed = false;

  fp = fopen(szSaveFile,"wb");
  if(fp != NULL)
  {
    dlg = GetDlgItem(L_hwnd,(int)MAKEINTRESOURCE(IDC_LIST1));
    count = SendMessage(dlg,LB_GETCOUNT,0,0);
    if(count == LB_ERR)
    {
      MessageBox(L_hwnd,"Error getting list count (buffer not saved, but file erased)",SU_DBG_CONSOLE_DEFAULT_WINDOW_NAME,MB_OK | MB_ICONERROR);
    }
    else
    {
      for(i=0;i<count;i++)
      {
        if(SendMessage(dlg,LB_GETTEXT,i,(LPARAM)buf) == LB_ERR)
        {
          failed = true;
          break;
        }
        len = strlen(buf);
        buf[len++] = 0x0D;
        buf[len++] = 0x0A;
        buf[len] = 0;
        fwrite(buf,len,1,fp);
      }
    }
    fclose(fp);
    if(failed)
    {
      MessageBox(L_hwnd,"Error getting list value (buffer not fully saved)",SU_DBG_CONSOLE_DEFAULT_WINDOW_NAME,MB_OK | MB_ICONERROR);
    }
  }
  else
  {
    MessageBox(L_hwnd,"Error open file for writing (buffer not saved)",SU_DBG_CONSOLE_DEFAULT_WINDOW_NAME,MB_OK | MB_ICONERROR);
  }
}

bool SaveMainLoop(HWND parent)
{
  OPENFILENAME of;
  char filter[]="Text files (*.txt)\0*.txt\0\0";
  char buf[1024];

  // Initialize OPENFILENAME
  ZeroMemory(&of, sizeof(of));
  of.lStructSize = sizeof(of);
  of.hwndOwner = parent;
  of.lpstrFilter = (const char *)filter;
  of.lpstrFile = szSaveFile;
  of.nMaxFile = sizeof(szSaveFile);
  of.lpstrDefExt = "*.txt";
  //of.lpstrTitle = "";
  of.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
  GetCurrentDirectory(sizeof(buf),buf);
  szSaveFile[0] = 0;

  if(GetSaveFileName(&of) != 0)
  {
    SaveLog();
  }

  SetCurrentDirectory(buf);
  return 0;
}

LRESULT CALLBACK wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  char buf[8192];
  LPMEASUREITEMSTRUCT lpmis;
  LPDRAWITEMSTRUCT lpdis;
  TEXTMETRIC tm;
  int y;
  LRESULT val;
  COLORREF oldcolor;
  HWND dlg;
  HWND hwndOwner;
  RECT rc, rcDlg, rcOwner;

  if(message == L_Msg) /* String message */
  {
    if(GlobalGetAtomName(wParam,buf,sizeof(buf)) != 0)
    {
      /* Add string - lParam = DebugFlags */
      if(IsStringDisplayed(lParam) && Capturing)
        DisplayMessage(buf,lParam);
    }
    GlobalDeleteAtom(wParam);
    return TRUE;
  }
  if(message == uFindReplaceMsg)
  {
    LPFINDREPLACE lpfr;
    lpfr = (LPFINDREPLACE)lParam;

    if(lpfr->Flags & FR_DIALOGTERM)
    {
      return 0;
    }
    if(lpfr->Flags & FR_FINDNEXT)
    {
      bFindCase = (lpfr->Flags & FR_MATCHCASE) != 0;
      bFindDown = (lpfr->Flags & FR_DOWN) != 0;
      bFinding = true;
      SendMessage(hFindWnd,WM_CLOSE,0,0);
      return 0;
    }
    return 0;
  }
  switch (message)
  {
    case WM_INITDIALOG:
      if((hwndOwner = GetParent(hwnd)) == NULL)
        hwndOwner = GetDesktopWindow();
      GetWindowRect(hwndOwner, &rcOwner);
      GetWindowRect(hwnd, &rcDlg);
      CopyRect(&rc, &rcOwner);
      // Offset the owner and dialog box rectangles so that
      // right and bottom values represent the width and
      // height, and then offset the owner again to discard
      // space taken up by the dialog box.
      OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
      OffsetRect(&rc, -rc.left, -rc.top);
      OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);
      // The new position is the sum of half the remaining
      // space and the owner's original position.
      SetWindowPos(hwnd,HWND_TOP,rcOwner.left + (rc.right / 2),rcOwner.top + (rc.bottom / 2),0, 0,SWP_NOSIZE);
      return TRUE;
    case WM_SIZE:
      //if(wParam == SIZE_RESTORED)
      {
        /* Set size of ListWindow */
        dlg = GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_LIST1));
        SetWindowPos(dlg,HWND_TOP,0,0,LOWORD(lParam) - 21,HIWORD(lParam) - 15 - 45,SWP_NOMOVE);
        /* Set size of Clear button */
        dlg = GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_BUTTON1));
        SetWindowPos(dlg,HWND_TOP,LOWORD(lParam) / 2 - 103,HIWORD(lParam) - 34,0,0,SWP_NOSIZE);
        /* Set size of Close button */
        dlg = GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_BUTTON2));
        SetWindowPos(dlg,HWND_TOP,LOWORD(lParam) / 2 + 26,HIWORD(lParam) - 34,0,0,SWP_NOSIZE);
        /* Set size of Autoscroll Checkbox */
        dlg = GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_SCROLL));
        SetWindowPos(dlg,HWND_TOP,10,HIWORD(lParam) - 28,0,0,SWP_NOSIZE);
        /* Set size of Capture Checkbox */
        dlg = GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_CAPTURE));
        SetWindowPos(dlg,HWND_TOP,LOWORD(lParam)-66,HIWORD(lParam) - 28,0,0,SWP_NOSIZE);
        return FALSE;
      }
      return FALSE;
    case WM_SIZING:
      {
        LPRECT rect = (LPRECT)lParam;
        if((rect->right-rect->left) < MIN_WIDTH_ALLOWED)
        {
          if((wParam == WMSZ_LEFT) || (wParam == WMSZ_BOTTOMLEFT) || (wParam == WMSZ_TOPLEFT))
            rect->left = rect->right - MIN_WIDTH_ALLOWED;
          else
            rect->right = rect->left + MIN_WIDTH_ALLOWED;
        }
        if((rect->bottom-rect->top) < MIN_HEIGHT_ALLOWED)
        {
          if((wParam == WMSZ_TOP) || (wParam == WMSZ_TOPLEFT) || (wParam == WMSZ_TOPRIGHT))
            rect->top = rect->bottom - MIN_HEIGHT_ALLOWED;
          else
            rect->bottom = rect->top + MIN_HEIGHT_ALLOWED;
        }
        return FALSE;
      }
    case WM_MEASUREITEM:
      lpmis = (LPMEASUREITEMSTRUCT) lParam;
      // Set the height of the list box items.
      lpmis->itemHeight = ITEMHEIGHT;
      return TRUE;
    case WM_DRAWITEM :
      lpdis = (LPDRAWITEMSTRUCT) lParam;
      // If there are no list box items, skip this message.
      if (lpdis->itemID == -1)
          break;
      switch (lpdis->itemAction)
      {
        case ODA_SELECT:
          if((lpdis->itemState & ODS_SELECTED) == 0)
            DrawFocusRect(lpdis->hDC,&lpdis->rcItem);
        case ODA_DRAWENTIRE:
          // Display the text associated with the item.
          SendMessage(lpdis->hwndItem,LB_GETTEXT,lpdis->itemID,(LPARAM)buf);
          val = SendMessage(lpdis->hwndItem,LB_GETITEMDATA,lpdis->itemID,0);

          GetTextMetrics(lpdis->hDC,&tm);
          y = (lpdis->rcItem.bottom + lpdis->rcItem.top - tm.tmHeight) / 2;

          oldcolor = SetTextColor(lpdis->hDC,GetColorFromFlag(val));
          TextOut(lpdis->hDC,2,y,buf,strlen(buf));
          SetTextColor(lpdis->hDC,oldcolor);

          if(lpdis->itemState & ODS_SELECTED)
            DrawFocusRect(lpdis->hDC,&lpdis->rcItem);
          break;
        case ODA_FOCUS: 
          break; 
      } 
      return TRUE;
    case WM_COMMAND:
    {
      switch(LOWORD(wParam))
      {
        case IDC_BUTTON2:
          {
            /* Config */
            if(hConfigWnd == NULL)
              NewThread(ConfigMainLoop,(void *)hwnd);
            else
              SetFocus(hConfigWnd);
            return TRUE;
          }
        case IDC_BUTTON1:
          dlg = GetDlgItem(hwnd,(int)MAKEINTRESOURCE(IDC_LIST1));
          SendMessage(dlg,LB_RESETCONTENT,0,0);
          return TRUE;
        case ID_FILE_EXIT:
          Exiting();
          return TRUE;
        case ID_EDIT_SEARCH:
          /* Search */
          if(hFindWnd == NULL)
            NewThread(FindMainLoop,(void *)hwnd);
          else
            SetFocus(hFindWnd);
          return TRUE;
        case ID_EDIT_FINDNEXT:
        case ID_EDIT_FINDNEXT_R:
          /* Search Next */
          if(hFindWnd != NULL)
            SetFocus(hFindWnd);
          else
          {
            if(szFindText[0] == 0)
              NewThread(FindMainLoop,(void *)hwnd);
            else
              NewThread(FindNextMainLoop,(void *)(LOWORD(wParam) == ID_EDIT_FINDNEXT_R));
          }
          return TRUE;
        case ID_FILE_SAVEBUFFER:
          if(szSaveFile[0] == 0)
            SaveMainLoop(hwnd);
          else
            SaveLog();
          return TRUE;
        case ID_FILE_SAVEAS:
          SaveMainLoop(hwnd);
          return TRUE;
        case ID_HELP_ABOUT:
          /* About */
          if(hAboutWnd == NULL)
            NewThread(AboutMainLoop,(void *)hwnd);
          else
            SetFocus(hAboutWnd);
          return TRUE;
        case ID_HELP_HELP:
          /* Help */
          if(hHelpWnd == NULL)
            NewThread(HelpMainLoop,(void *)hwnd);
          else
            SetFocus(hHelpWnd);
          return TRUE;
        case IDC_SCROLL:
          Scrolling = IsDlgButtonChecked(hwnd,(int)MAKEINTRESOURCE(IDC_SCROLL)) == BST_CHECKED;
          return TRUE;
        case IDC_CAPTURE:
          Capturing = IsDlgButtonChecked(hwnd,(int)MAKEINTRESOURCE(IDC_CAPTURE)) == BST_CHECKED;
          return TRUE;
      }
      break;
    }
    case WM_CLOSE:
    {
      Exiting();
      return TRUE;
    }
    case WM_DESTROY:
      PostQuitMessage(0);
      return TRUE;
    case WM_INITMENUPOPUP:
      return 0;
  }
  return FALSE;
}

void SetWindowTitle(char *Txt)
{
  SetWindowText(L_hwnd,Txt);
}

void WaitForWindowCreate(void)
{
  while(L_hwnd == NULL)
  {
    SU_SLEEP(1);
  }
}

bool InitGUI(char *ConsoleName,int x,int y,int w,int h)
{
  L_Msg = RegisterWindowMessage(((ConsoleName == NULL)||(ConsoleName[0] == 0))?SU_DBG_CONSOLE_DEFAULT_WINDOW_NAME:ConsoleName);
  L_hwnd = CreateDialog(L_hInstance,MAKEINTRESOURCE(IDD_DIALOG1),GetDesktopWindow(),wndProc);
  if(L_hwnd == NULL)
    return false;
  if(w < MIN_WIDTH_ALLOWED)
    w = MIN_WIDTH_ALLOWED;
  if(h < MIN_HEIGHT_ALLOWED)
    h = MIN_HEIGHT_ALLOWED;
  MoveWindow(L_hwnd,x,y,w,h,true);
  CheckDlgButton(L_hwnd,(int)MAKEINTRESOURCE(IDC_SCROLL),BST_CHECKED);
  CheckDlgButton(L_hwnd,(int)MAKEINTRESOURCE(IDC_CAPTURE),BST_CHECKED);
  uFindReplaceMsg = RegisterWindowMessage(FINDMSGSTRING);
  szFindText[0] = 0;
  return true;
}

void GUIMainLoop(void)
{
  MSG msg;
  HACCEL accel;

  accel = LoadAccelerators(L_hInstance,MAKEINTRESOURCE(IDR_ACCELERATOR1));
  ShowWindow(L_hwnd,SW_SHOW);
  while(GetMessage(&msg,NULL,0,0))
  {
    if(!IsWindow(L_hwnd) || !TranslateAccelerator(L_hwnd,accel,&msg) || !IsDialogMessage(L_hwnd,&msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
}

bool WSInit(int Major,int Minor)
{
  WORD wVersionRequested;
  WSADATA wsaData;
  int err;
  static bool SU_Sock_Init = false;

  if(!SU_Sock_Init)
  {
    SU_Sock_Init = true;
    wVersionRequested = MAKEWORD( Major, Minor );
    err = WSAStartup(wVersionRequested,&wsaData);
    if(err != 0)
      return false;
    if(LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2)
    {
      WSACleanup();
      return false;
    }
  }
  return true;
}

void ErrorMessage(char *Msg)
{
  MessageBox(NULL,Msg,SU_DBG_CONSOLE_DEFAULT_WINDOW_NAME,MB_OK);
}

#else /* !_WIN32 */

SU_PList SU_DBG_WIN_AddElementTail(SU_PList List,void *Elem)
{
  SU_PList Ptr,Ptr2,El;

  Ptr = List;
  Ptr2 = NULL;
  while(Ptr != NULL)
  {
    Ptr2 = Ptr;
    Ptr = Ptr->Next;
  }
  El = (SU_PList) malloc(sizeof(SU_TList));
  El->Next = NULL;
  El->Data = Elem;
  if(List == NULL)
    return El;
  Ptr2->Next = El;
  return List;
}

bool SU_DBG_WIN_CreateSem(SU_SEM_HANDLE *Handle,int InitialCount,int MaximumCount,const char SemName[])
{
  return sem_init(Handle,0,InitialCount) == 0;
}

char *GetColorFromFlag(SU_u64 val)
{
  int i = 0;

  val>>=1;
  while(val)
  {
    i++;
    val>>=1;
  }
  if(i >= MAXCOLORS)
    return "#000000";
  return ColorTable[i].Name;
}

bool IsStringDisplayed(SU_u64 val)
{
  int i = 0;

  val>>=1;
  while(val)
  {
    i++;
    val>>=1;
  }
  if(i >= MAXCOLORS)
    return true;
  return DisplayTable[i];
}

void SetDisplayTable(GtkWindow *wnd)
{
  int i;
  GtkToggleButton *but;
  char buf[200];

  for(i=0;i<MAXCOLORS;i++)
  {
    snprintf(buf,sizeof(buf),"checkbutton%d",i+1);
    but = (GtkToggleButton *) lookup_widget(GTK_WIDGET(wnd),buf);
    gtk_toggle_button_set_active(but,DisplayTable[i]);
  }
}

void UpdateDisplayTable(GtkWindow *wnd)
{
  int i;
  GtkToggleButton *but;
  char buf[100];

  for(i=0;i<MAXCOLORS;i++)
  {
    snprintf(buf,sizeof(buf),"checkbutton%d",i+1);
    but = (GtkToggleButton *) lookup_widget(GTK_WIDGET(wnd),buf);
    DisplayTable[i] = gtk_toggle_button_get_active(but);
  }
}

gint timeout_callback( gpointer data )
{
  PMessage Msg;
  SU_PList Ptr,Ptr2;
  
  SU_SEM_WAIT(L_sem);
  if(L_Messages != NULL)
  {
    Ptr = L_Messages;
    while(Ptr != NULL)
    {
      Msg = (PMessage) Ptr->Data;
      gtk_text_buffer_insert_with_tags_by_name(L_buf,&L_iter,Msg->Msg,-1,GetColorFromFlag(Msg->Type),NULL);
      free(Msg->Msg);
      free(Msg);
      Ptr2 = Ptr;
      Ptr = Ptr->Next;
      free(Ptr2);
    }
    L_Messages = NULL;
    gtk_text_view_scroll_to_iter(L_htxt,&L_iter,0,false,0,0);
  }
  SU_SEM_POST(L_sem);
  return TRUE;
}

void CreateTags(void)
{
  int i;
  GtkTextBuffer *buf;

  buf = gtk_text_view_get_buffer(L_htxt);
  for(i=0;i<MAXCOLORS;i++)
  {
    gtk_text_buffer_create_tag(buf, ColorTable[i].Name,"foreground", ColorTable[i].Value, NULL);
  }
}

void DisplayMessage(char *Msg,SU_u64 Type)
{
  int len = strlen(Msg);
  PMessage Message;

  Msg[len++] = '\n';
  Msg[len] = 0;
  SU_SEM_WAIT(L_sem);
  Message = (PMessage) malloc(sizeof(TMessage));
  Message->Msg = strdup(Msg);
  Message->Type = Type;
  L_Messages = SU_DBG_WIN_AddElementTail(L_Messages,Message);
  SU_SEM_POST(L_sem);
}

void SetWindowTitle(char *Txt)
{
  gtk_window_set_title(GTK_WINDOW(L_hwnd),Txt);
}

void WaitForWindowCreate(void)
{
  while(L_hwnd == NULL)
  {
    SU_SLEEP(1);
  }
}

bool NewThread(SU_THREAD_ROUTINE_TYPE(Entry),void *User)
{
  SU_THREAD_HANDLE Handle;
  if(pthread_create(&Handle,NULL,Entry,User) != 0)
    return false;
  pthread_detach(Handle);
  return true;
}

bool InitGUI(char *ConsoleName,int x,int y,int w,int h)
{
  /* init gtk */
  gtk_set_locale ();
  gtk_init (0, NULL);

  L_hwnd = create_SkyUtils_Console ();
  if(L_hwnd == NULL)
    return false;
  L_hwndErr = create_ErrorWnd();
  if(L_hwndErr == NULL)
    return false;
  L_hwndCfg = create_ConfigWnd();
  if(L_hwndCfg == NULL)
    return false;

  gtk_window_move(GTK_WINDOW(L_hwnd),x,y);
  gtk_window_resize(GTK_WINDOW(L_hwnd),w,h);
  gtk_widget_show(GTK_WIDGET(L_hwnd));
  L_htxt = (GtkTextView *) lookup_widget(GTK_WIDGET(L_hwnd),"textview1");
  if(L_htxt == NULL)
    return false;

  L_buf = gtk_text_view_get_buffer(L_htxt);
  gtk_text_buffer_get_end_iter(L_buf,&L_iter);
  
  CreateTags();
  
  if(!SU_DBG_WIN_CreateSem(&L_sem,1,1,"Sem"))
    return false;

  return true;
}

void GUIMainLoop(void)
{
  gint tag;
  /* enter the GTK main loop */
  MainLoop = true;
  tag = gtk_timeout_add(200,timeout_callback,NULL);
  gtk_main();
  gtk_timeout_remove(tag);
}

void ErrorMessage(char *Msg)
{
  GtkLabel *txt;

  printf("SkyUtils Console Error : %s\n",Msg);  
  if((L_hwnd == NULL) || (!MainLoop))
    return;
  txt = (GtkLabel *) lookup_widget(L_hwndErr,"label4");
  gtk_label_set_text(txt,Msg);
  gtk_widget_show(L_hwndErr);
  ErrorWindow = true;
  
  while(ErrorWindow)
    SU_SLEEP(1);
}

#endif /* _WIN32 */

/* Common code */
int ReadLine(FILE *fp,char S[],int len)
{
  int i;
  char c;

  i = 0;
  S[0] = 0;
  if(fread(&c,1,1,fp) != 1)
    return 0;
  while((c == 0x0A) || (c == 0x0D))
  {
    if(fread(&c,1,1,fp) != 1)
      return 0;
  }
  while((c != 0x0A) && (c != 0x0D))
  {
    if(i >= (len-1))
      break;
    S[i++] = c;
    if(fread(&c,1,1,fp) != 1)
      break;
  }
  S[i] = 0;
  return 1;
}

bool LoadCfg(FILE *fp)
{
  char Buf[1024],val[8];
  int i = 0;

  if(ReadLine(fp,Buf,sizeof(Buf))) /* Load window settings */
  {
    if(sscanf(Buf,"%d %d %d %d",&L_x,&L_y,&L_w,&L_h) != 4)
    {
      ErrorMessage("Error : Old su_dbg_win.cfg file format. Add '0 0 200 200' as first line");
      return false;
    }
  }
  while(ReadLine(fp,Buf,sizeof(Buf))) /* Load color settings */
  {
    DisplayTable[i] = Buf[0] == '1';
    strncpy(val+1,Buf+1,6); /* Get color */
    val[7] = 0;
#ifdef _WIN32
    sscanf(val+1,"%x",&ColorTable[i]);
#else /* !_WIN32 */
    val[0] = '#';
    ColorTable[i].Value = strdup(val);
#endif /* _WIN32 */
    DisplayTableStrings[i] = strdup(Buf+7);
    i++;
  }
  return true;
}

void SaveCfg()
{
  FILE *fp;
  int i;
  char buf[200];

  snprintf(buf,sizeof(buf),"su_dbg_win_%s.cfg",((SU_ConsoleName == NULL)||(SU_ConsoleName[0] == 0))?"default":SU_ConsoleName);
  fp = fopen(buf,"wb");
  if(fp != NULL)
  {
    fprintf(fp,"%d %d %d %d\n",L_x,L_y,L_w,L_h); /* Write window settings */

    for(i=0;i<MAXCOLORS;i++) /* Write color settings */
    {
#ifdef _WIN32
      fprintf(fp,"%s%.6x%s\n",DisplayTable[i]?"1":"0",ColorTable[i],DisplayTableStrings[i]);
#else /* !_WIN32 */
      fprintf(fp,"%s%s%s\n",DisplayTable[i]?"1":"0",ColorTable[i].Value+1,DisplayTableStrings[i]);
#endif /* _WIN32 */
    }
    fclose(fp);
  }
}

void GetSocketMessages(SU_SOCKET sock)
{
  int res;
  SU_u32 len,total;
  SU_u64 Type;
  char buf[4096];

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
    if(IsStringDisplayed(Type))
      DisplayMessage(buf,Type);
  }
}

int SetNewConnect(const char *Addr,SU_SOCKET sock)
{
  int i,idx=-1;
  char buf[1024];

  snprintf(buf,sizeof(buf),"Application connected from %s",Addr);
  DisplayMessage(buf,0);
  snprintf(buf,sizeof(buf),"%s [Connected from",((SU_ConsoleName == NULL)||(SU_ConsoleName[0] == 0))?SU_DBG_CONSOLE_DEFAULT_WINDOW_NAME:SU_ConsoleName);
  for(i=0;i<MAX_CONNS;i++)
  {
    if(ConnStrs[i] == NULL)
    {
      if(idx == -1)
      {
        ConnStrs[i] = strdup(Addr);
        ConnSocks[i] = sock;
        idx = i;
        NbConns++;
        strcat(buf," ");
        strcat(buf,ConnStrs[i]);
      }
    }
    else
    {
      strcat(buf," ");
      strcat(buf,ConnStrs[i]);
    }
  }
  strcat(buf,"]");
  if(idx == -1)
  {
    ErrorMessage("Mismatch between NbConns and ConnStrs tab !!");
    return 0;
  }
  SetWindowTitle(buf);
  return idx;
}

void RemoveConnect(int idx)
{
  int i;
  char buf[1024];

  snprintf(buf,sizeof(buf),"Application disconnected (%s). Closing link",ConnStrs[idx]);
  DisplayMessage(buf,0);
  free(ConnStrs[idx]);
  ConnStrs[idx] = NULL;
  NbConns--;

  if(NbConns == 0)
  {
    snprintf(buf,sizeof(buf),"%s [Listening on port %d]",((SU_ConsoleName == NULL)||(SU_ConsoleName[0] == 0))?SU_DBG_CONSOLE_DEFAULT_WINDOW_NAME:SU_ConsoleName,SU_SockPort);
  }
  else
  {
    snprintf(buf,sizeof(buf),"%s [Connected from",((SU_ConsoleName == NULL)||(SU_ConsoleName[0] == 0))?SU_DBG_CONSOLE_DEFAULT_WINDOW_NAME:SU_ConsoleName);
    for(i=0;i<MAX_CONNS;i++)
    {
      if(ConnStrs[i] != NULL)
      {
        strcat(buf," ");
        strcat(buf,ConnStrs[i]);
      }
    }
    strcat(buf,"]");
  }
  SetWindowTitle(buf);
}

SU_THREAD_ROUTINE(ThreadFuncChild,info)
{
  int idx = (int) info;

  /* Retreive messages */
  GetSocketMessages(ConnSocks[idx]);
  SU_CLOSE_SOCKET(ConnSocks[idx]);
  /* Client disconnected */
  RemoveConnect(idx);
  SU_THREAD_RETURN(0);
}

SU_THREAD_ROUTINE(ThreadFunc,info)
{
  SU_SOCKET sock = (SU_SOCKET) info;
  struct sockaddr_in sad;
  int len,i;
  int clsock;
  char buf[500];

  WaitForWindowCreate();
  for(i=0;i<MAX_CONNS;i++)
    ConnStrs[i] = NULL;
  snprintf(buf,sizeof(buf),"%s [Listening on port %d]",((SU_ConsoleName == NULL)||(SU_ConsoleName[0] == 0))?SU_DBG_CONSOLE_DEFAULT_WINDOW_NAME:SU_ConsoleName,SU_SockPort);
  SetWindowTitle(buf);
  while(1)
  {
    len = sizeof(sad);
    clsock = accept(sock,(struct sockaddr *)&sad,&len);
    if(clsock == -1)
    {
      SU_SLEEP(2);
      continue;
    }
    if(NbConns >= MAX_CONNS)
    {
      ErrorMessage("Too many active connections. Increase MAX_CONNS value");
      SU_THREAD_RETURN(0);
    }
    i = SetNewConnect(inet_ntoa(sad.sin_addr),clsock);
    /* Ok client connected */
    if(!NewThread(ThreadFuncChild,(void *)i))
    {
      ErrorMessage("Error creating child thread");
      SU_THREAD_RETURN(0);
    }
  }
  SU_THREAD_RETURN(0);
}

SU_SOCKET ListenServer(int port)
{
  SU_SOCKET sock;
  struct sockaddr_in SAddr;

#ifdef _WIN32
  WSInit(2,2);
#endif /* _WIN32 */
  sock = socket(AF_INET,SOCK_STREAM,getprotobyname("tcp")->p_proto);
  if(sock == -1)
  {
    return -1;
  }
  memset(&(SAddr),0,sizeof(struct sockaddr_in));
  SAddr.sin_family = AF_INET;
  SAddr.sin_port = htons(port);
  SAddr.sin_addr.s_addr = 0;
  if(bind(sock,(struct sockaddr *)&(SAddr), sizeof(SAddr)) == -1)
  {
    SU_CLOSE_SOCKET(sock);
    return -1;
  }

  if(listen(sock,10) == -1)
  {
    SU_CLOSE_SOCKET(sock);
    return -1;
  }

  return sock;
}

#ifdef _WIN32
char *SU_DBG_WIN_strchrl(const char *s,const char *l,char *found)
{
  long int len,i;

  len = strlen(l);
  while(s[0] != 0)
  {
    for(i=0;i<len;i++)
    {
      if(s[0] == l[i])
      {
        if(found != NULL)
          *found = s[0];
        return (char *)s;
      }
    }
    s++;
  }
  return NULL;
}

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
#else /* !_WIN32 */
int main(int argc,char *argv[])
#endif /* _WIN32 */
{
  FILE *fp;
  int value = 0,i;
  char buf[200];

#ifdef _WIN32
#define MAX_PARAMS 12
  char *argv[MAX_PARAMS],*p,*q,*r,found;
  int argc = 1;
  char search[]=" \"";

  argv[0] = strdup("");
  p = lpCmdLine;
  q = SU_DBG_WIN_strchrl(p,search,&found);
  while(q != NULL)
  {
    if(argc >= MAX_PARAMS)
    {
      ErrorMessage("Command line parse fatal error : too many arguments");
      return -1;
    }
    switch(found)
    {
      case ' ' :
        q[0] = 0;
        argv[argc++] = strdup(p);
        p = q + 1;
        break;
      case '"' :
        r = strchr(q+1,'"');
        if(r == NULL)
        {
          ErrorMessage("Command line parse fatal error");
          return -1;
        }
        r[0] = 0;
        argv[argc++] = strdup(q+1);
        p = r + 1;
        while(p[0] == ' ') p++;
        break;
    }
    q = SU_DBG_WIN_strchrl(p,search,&found);
  }
  if(p[0] != 0)
  {
    if(argc >= MAX_PARAMS)
    {
      ErrorMessage("Command line parse fatal error : too many arguments");
      return -1;
    }
    argv[argc++] = strdup(p);
  }

#endif /* _WIN32 */

  /* Parse arguments */
  for(i=1;i<argc;i++)
  {
    if(strncmp(argv[i],"-p",2) == 0) /* Port number for socket mode */
    {
      value = atoi(argv[i]+2);
    }
    else if((strcmp(argv[i],"-h") == 0) || (strcmp(argv[i],"--help") == 0))
    {
      ErrorMessage("Usage : su_dbg_win [Name] [-pListenPort]");
      return -1;
    }
    else
      SU_ConsoleName = argv[i];
  }

  /* Load config file */
  snprintf(buf,sizeof(buf),"su_dbg_win_%s.cfg",((SU_ConsoleName == NULL)||(SU_ConsoleName[0] == 0))?"default":SU_ConsoleName);
  fp = fopen(buf,"rb");
  if(fp != NULL)
  {
    if(!LoadCfg(fp))
    {
      fclose(fp);
      return -1;
    }
    fclose(fp);
  }

  if(!InitGUI(SU_ConsoleName,L_x,L_y,L_w,L_h))
    return -1;

  SetWindowTitle(((SU_ConsoleName == NULL)||(SU_ConsoleName[0] == 0))?SU_DBG_CONSOLE_DEFAULT_WINDOW_NAME:SU_ConsoleName);

  /* Check if we want Socket or WinMsg */
  if(value != 0)
  {
    SU_SOCKET sock;
    sock = ListenServer(value);
    if(sock == -1)
    {
      ErrorMessage("Error creating socket server");
      return 1;
    }
    /* Socket mode */
    SU_SockPort = value;
    if(!NewThread(ThreadFunc,(void *)sock))
    {
      ErrorMessage("Error creating thread");
      return 2;
    }
  }

  GUIMainLoop();

  SaveCfg();

  return 0;
}
