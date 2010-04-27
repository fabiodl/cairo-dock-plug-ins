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


#ifndef __CD_CLOCK_DRAW__
#define  __CD_CLOCK_DRAW__


#include <cairo-dock.h>
#include "applet-struct.h"


void cd_clock_free_alarm (CDClockAlarm *pAlarm);


void cd_clock_init_time (CairoDockModuleInstance *myApplet);

gboolean cd_clock_update_with_time (CairoDockModuleInstance *myApplet);


void cd_clock_draw_text (CairoDockModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime);


void cd_clock_draw_analogic (CairoDockModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime);


void cd_clock_draw_analogic_opengl (CairoDockModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime, double fFraction);

void cd_clock_render_analogic_to_texture (CairoDockModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime, double fFraction);


#endif

