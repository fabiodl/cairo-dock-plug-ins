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

#ifndef __APPLET_NOTIFICATIONS__
#define  __APPLET_NOTIFICATIONS__

#include <cairo-dock.h>


gboolean cd_do_key_pressed (gpointer pUserData, GldiContainer *pContainer, guint iKeyVal, guint iModifierType, const gchar *string, int iKeyCode);


void cd_do_on_shortkey_nav (const char *keystring, gpointer data);


gboolean cd_do_update_container (gpointer pUserData, GldiContainer *pContainer, gboolean *bContinueAnimation);


gboolean cd_do_check_icon_destroyed (gpointer pUserData, Icon *pIcon);


gboolean cd_do_check_active_dock (gpointer pUserData, GldiWindowActor *actor);


gboolean cd_do_render (gpointer pUserData, GldiContainer *pContainer, cairo_t *pCairoContext);

gboolean cd_do_on_click (gpointer pUserData, Icon *icon, GldiContainer *pContainer);

#endif
