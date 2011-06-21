/*********************************************************************************************************
** (C) Copyright 2002-2003 Christophe Calmejane, Thierry Carrard, Claude Limousin and Alexis Vartanian   
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of the authors ;
** the contents of this file may not be used, disclosed to third parties, 
** copied or duplicated in any form, in whole or in part, without the 
** prior written permission of the authors 
**********************************************************************************************************/
#include <gtk/gtk.h>


gboolean
on_SkyUtils_Console_delete_event       (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_button3_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_ErrorWnd_delete_event               (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_button1_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_button2_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_ConfigWnd_delete_event              (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_button4_clicked                     (GtkButton       *button,
                                        gpointer         user_data);
