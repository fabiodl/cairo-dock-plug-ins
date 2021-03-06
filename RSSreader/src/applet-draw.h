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


#ifndef __APPLET_DRAW__
#define  __APPLET_DRAW__

#include <cairo-dock.h>
#include "applet-struct.h"


char* ltrim( char* str, const char* t );

gchar *cd_rssreader_cut_feed_lines_with_return (GldiModuleInstance *myApplet, int iMaxWidth, gchar *cLongLine);

void cd_rssreader_cut_feed_lines (GldiModuleInstance *myApplet, int iMaxWidth, gchar *cLongLine);


void cd_rssreader_upload_title_TASK (GldiModuleInstance *myApplet);


void cd_rssreader_launch_task (GldiModuleInstance *myApplet);


void cd_applet_draw_my_desklet (GldiModuleInstance *myApplet, int iWidth, int iHeight);

void cd_applet_update_my_icon (GldiModuleInstance *myApplet/**, Icon *pIcon, GldiContainer *pContainer*/);


#endif
