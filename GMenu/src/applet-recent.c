/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Adapted from the Gnome-panel for Cairo-Dock by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include "applet-struct.h"
#include "applet-recent.h"
#include "applet-menu-callbacks.h"

static void
recent_documents_activate_cb (GtkRecentChooser *chooser,
			      gpointer          data)
{
	GtkRecentInfo *recent_info = gtk_recent_chooser_get_current_item (chooser);
	const char *uri = gtk_recent_info_get_uri (recent_info);
	g_print ("uri : %s / %s\n", uri, gtk_recent_info_get_display_name(recent_info));
	cairo_dock_fm_launch_uri (uri);
	gtk_recent_info_unref (recent_info);
}

static void
panel_recent_manager_changed_cb (GtkRecentManager *manager,
				 GtkWidget        *menu_item)
{
	int size;

	g_object_get (manager, "size", &size, NULL);

	gtk_widget_set_sensitive (menu_item, size > 0);
}

void cd_menu_append_recent_to_menu (GtkWidget *top_menu, GtkRecentManager *manager)
{
	g_return_if_fail (myData.pRecentMenuItem == NULL && manager != NULL);
	GtkWidget      *recent_menu;
	GtkWidget      *pMenuItem;
	int             size;

	pMenuItem = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (top_menu), pMenuItem);
	
	pMenuItem = gtk_image_menu_item_new_with_label (D_("Recent Documents"));
	const gchar *cIconPath = MY_APPLET_SHARE_DATA_DIR"/icon-recent.svg";
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size (cIconPath, 16, 16, NULL);
	GtkWidget *image = gtk_image_new_from_pixbuf (pixbuf);
	g_object_unref (pixbuf);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (pMenuItem), image);
	
	recent_menu = gtk_recent_chooser_menu_new_for_manager (manager);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (pMenuItem), recent_menu);

	g_signal_connect (G_OBJECT (recent_menu), "button_press_event",
		  G_CALLBACK (menu_dummy_button_press_event), NULL);  // utile ?

	gtk_menu_shell_append (GTK_MENU_SHELL (top_menu), pMenuItem);
	gtk_widget_show_all (pMenuItem);
	
	gtk_recent_chooser_set_local_only (GTK_RECENT_CHOOSER (recent_menu),
		 FALSE);
	gtk_recent_chooser_set_show_tips (GTK_RECENT_CHOOSER (recent_menu),
		TRUE);
	gtk_recent_chooser_set_sort_type (GTK_RECENT_CHOOSER (recent_menu),
		GTK_RECENT_SORT_MRU);

	g_signal_connect (GTK_RECENT_CHOOSER (recent_menu),
		"item-activated",
		G_CALLBACK (recent_documents_activate_cb),
		NULL);

	g_signal_connect_object (manager, "changed",
		G_CALLBACK (panel_recent_manager_changed_cb),
		 pMenuItem, 0);

	size = 0;
	g_object_get (manager, "size", &size, NULL);
	gtk_widget_set_sensitive (pMenuItem, size > 0);
	g_print ("size : %d\n", size);
	
	myData.pRecentMenuItem = pMenuItem;
}


void cd_menu_clear_recent (GtkMenuItem *menu_item, gpointer data)
{
	int iAnswer = cairo_dock_ask_question_and_wait (D_("Clear the list of the recently used documents ?"), myIcon, myContainer);
	if (iAnswer == GTK_RESPONSE_YES)
	{
		gtk_recent_manager_purge_items (myData.pRecentManager, NULL);
	}
}
