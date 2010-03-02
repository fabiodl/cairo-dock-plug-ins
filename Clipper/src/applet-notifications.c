/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-clipboard.h"
#include "applet-notifications.h"


//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	if (myData.pItems == NULL)
	{
		cairo_dock_remove_dialog_if_any (myIcon);
		cairo_dock_show_temporary_dialog_with_icon (D_("No items yet."), myIcon, myContainer, 2000, "same icon");
	}
	else
	{
		GtkWidget *pMenu = cd_clipper_build_items_menu ();
		cairo_dock_popup_menu_on_container (pMenu, myContainer);
	}
CD_APPLET_ON_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
static void _cd_clipper_clear_history (GtkMenuItem *menu_item, gpointer data)
{
	CD_APPLET_ENTER;
	GtkClipboard *pClipBoard;
	if (myConfig.iItemType & CD_CLIPPER_CLIPBOARD)
	{
		pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
		gtk_clipboard_clear (pClipBoard);
	}
	
	if (myConfig.iItemType & CD_CLIPPER_PRIMARY)
	{
		pClipBoard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
		gtk_clipboard_clear (pClipBoard);
	}
	
	g_list_foreach (myData.pItems, (GFunc) g_free, NULL);
	g_list_free (myData.pItems);
	myData.pItems = NULL;
	int i;
	for (i=0; i<4; i++)
		myData.iNbItems[i] = 0;
	CD_APPLET_LEAVE();
}

static void _cd_clipper_paste_all (GtkMenuItem *menu_item, gpointer data)
{
	CD_APPLET_ENTER;
	gchar *cText = cd_clipper_concat_items_of_type ((myConfig.iItemType & CD_CLIPPER_CLIPBOARD) ? CD_CLIPPER_CLIPBOARD : CD_CLIPPER_PRIMARY, "\n");  // on prend que les CTRL+c si possible.
	GtkClipboard *pClipBoard;
	if (myConfig.bPasteInPrimary)
	{
		pClipBoard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
		gtk_clipboard_set_text (pClipBoard, cText, -1);
	}
	if (myConfig.bPasteInClipboard)
	{
		pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
		gtk_clipboard_set_text (pClipBoard, cText, -1);
	}
	g_free (cText);
	CD_APPLET_LEAVE();
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		CD_APPLET_ADD_IN_MENU_WITH_STOCK ("Clear clipboard History", "gtk-clear", _cd_clipper_clear_history, pSubMenu);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK ("Paste all copied items", "gtk-paste", _cd_clipper_paste_all, pSubMenu);
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myConfig.pPersistentItems == NULL)
	{
		cairo_dock_remove_dialog_if_any (myIcon);
		cairo_dock_show_temporary_dialog_with_icon (D_("No persistent items.\nYou can add some by drag and dropping some text on the icon."), myIcon, myContainer, 6000, "same icon");
	}
	else
	{
		GtkWidget *pMenu = cd_clipper_build_persistent_items_menu ();
		cairo_dock_popup_menu_on_container (pMenu, myContainer);
	}
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_DROP_DATA_BEGIN
	cd_message ("'%s' --> permanent !", CD_APPLET_RECEIVED_DATA);
	
	if (myConfig.pPersistentItems == NULL)
	{
		myConfig.pPersistentItems = g_new0 (gchar *, 2);
		myConfig.pPersistentItems[0] = g_strdup (CD_APPLET_RECEIVED_DATA);
	}
	else
	{
		GString *pString = g_string_new ("");
		int i;
		for (i = 0; myConfig.pPersistentItems[i] != NULL; i ++)
		{
			g_string_append_printf (pString, "%s;", myConfig.pPersistentItems[i]);
		}
		g_string_append (pString, CD_APPLET_RECEIVED_DATA);
		cairo_dock_update_conf_file (myApplet->cConfFilePath,
			G_TYPE_STRING, "Configuration", "persistent", pString->str,
			G_TYPE_INVALID);
		
		myConfig.pPersistentItems = g_realloc (myConfig.pPersistentItems, (i + 2) * sizeof (gchar*));
		myConfig.pPersistentItems[i] = g_strdup (CD_APPLET_RECEIVED_DATA);
		myConfig.pPersistentItems[i+1] = NULL;  // precaution.
		g_string_free (pString, TRUE);
	}
CD_APPLET_ON_DROP_DATA_END
