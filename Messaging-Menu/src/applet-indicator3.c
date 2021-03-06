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

#include "applet-struct.h"
#include "applet-indicator3.h"
#include "indicator-applet3-utils.h"

void _check_demanding_attention (const gchar *cName, const gchar *cAnimationName)
{
	if (g_str_has_suffix (cName, "-new"))
		CD_APPLET_DEMANDS_ATTENTION (cAnimationName, 60);
	else
		CD_APPLET_STOP_DEMANDING_ATTENTION;
}

static void _icon_updated (GObject *pObject, G_GNUC_UNUSED GParamSpec *pParam, GldiModuleInstance *myApplet)
{
	g_return_if_fail (GTK_IS_IMAGE (pObject));
	GtkImage *pImage = GTK_IMAGE (pObject);

	gchar *cName = NULL;
	if (! cd_indicator3_update_image (pImage, &cName, myApplet, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE))
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;

	if (cName)
	{
		_check_demanding_attention (cName, myConfig.cAnimationName);
		g_free (cName);
	}
}

void cd_messaging_accessible_desc_update (G_GNUC_UNUSED IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, GldiModuleInstance *myApplet)
{
	cd_indicator3_accessible_desc_update (pEntry, myConfig.defaultTitle, myApplet);
}

void cd_messaging_entry_added (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, GldiModuleInstance *myApplet)
{
	cd_debug ("Entry Added: %p", pEntry);
	g_return_if_fail (myData.pEntry == NULL); // should not happen... only one entry

	myData.pEntry = pEntry;
	GtkImage *pImage = cd_indicator3_get_image (pEntry);

	// get the icon and the label

	// signals
	cd_indicator3_notify_image (pImage, G_CALLBACK (_icon_updated), myApplet);
	cd_indicator3_notify_visibility (pImage, G_CALLBACK (_icon_updated), myApplet);


	cd_indicator3_check_visibility (pImage, myApplet);
	cd_messaging_accessible_desc_update (pIndicator, pEntry, myApplet);
}

void cd_messaging_entry_removed (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, GldiModuleInstance *myApplet)
{
	// should not happen... except at the end.
	cd_debug ("Entry Removed");

	gboolean bHide = FALSE;
	if (myData.pEntry != NULL && myData.pEntry == pEntry) // only if an entry was already added and it was the same that we want to remove
	{
		myData.pEntry = NULL;
		bHide = TRUE;
	}
	
	if (pEntry && pEntry->image)
	{
		g_signal_handlers_disconnect_by_func (G_OBJECT (pEntry->image), G_CALLBACK (_icon_updated), myApplet);
		cd_indicator3_disconnect_visibility (pEntry->image, myApplet, bHide);
	}
}

void cd_messaging_destroy (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, gpointer data)
{
	cd_debug ("Destroy");
	cd_messaging_entry_removed (pIndicator, pEntry, data);
}

void cd_messaging_reload (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, gpointer data)
{
	cd_debug ("Reload");
	g_return_if_fail (pEntry != NULL);
	cd_messaging_accessible_desc_update (pIndicator, pEntry, data);
	_icon_updated (G_OBJECT (pEntry->image), NULL, data);
}
