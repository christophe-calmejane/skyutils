/*********************************************************************************************************
** (C) Copyright 2002-2003 Christophe Calmejane, Thierry Carrard, Claude Limousin and Alexis Vartanian   
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of the authors ;
** the contents of this file may not be used, disclosed to third parties, 
** copied or duplicated in any form, in whole or in part, without the 
** prior written permission of the authors 
**********************************************************************************************************/
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include <skyutils.h>
extern GtkTextView *L_htxt;
extern volatile GtkWidget *L_hwnd;
extern GtkWidget *L_hwndErr;
extern GtkWidget *L_hwndCfg;
extern GtkTextIter L_iter;
extern GtkTextBuffer *L_buf;
extern int L_x,L_y,L_w,L_h;

gboolean
on_SkyUtils_Console_delete_event       (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  gtk_window_get_position(GTK_WINDOW(L_hwnd),&L_x,&L_y);
  gtk_window_get_size(GTK_WINDOW(L_hwnd),&L_w,&L_h);
  gtk_main_quit();
  return FALSE;
}

/* Error window close button */
void
on_button3_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  on_ErrorWnd_delete_event(GTK_WIDGET(button),NULL,NULL); /* ??? Why does it not work this user_data, but with button ??? */
}

extern volatile bool ErrorWindow;
gboolean
on_ErrorWnd_delete_event               (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  gtk_widget_hide(L_hwndErr);
  ErrorWindow = false;
  return TRUE;
}

/* Clear button */
void
on_button1_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  gtk_text_buffer_set_text(L_buf,"",0);
  gtk_text_buffer_get_end_iter(L_buf,&L_iter);
}

void SetDisplayTable(GtkWindow *wnd);
/* Config button */
void
on_button2_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  SetDisplayTable(GTK_WINDOW(L_hwndCfg));
  gtk_widget_show(L_hwndCfg);
}

void UpdateDisplayTable(GtkWindow *wnd);
void SaveCfg(void);
gboolean
on_ConfigWnd_delete_event              (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  gtk_widget_hide(L_hwndCfg);
  UpdateDisplayTable(GTK_WINDOW(L_hwndCfg));
  SaveCfg();
  return TRUE;
}

/* Config window close button */
void
on_button4_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  on_ConfigWnd_delete_event(GTK_WIDGET(user_data),NULL,NULL); /* ??? Why does it not work this user_data, but with button ??? */
}

