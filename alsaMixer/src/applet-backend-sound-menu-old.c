/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* based on indicator-messages.c written by :
*  Ted Gould <ted@canonical.com>
*  Cody Russell <cody.russell@canonical.com>
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

#if (SOUND_SERVICE_VERSION < 1)
#include "dbus-shared-names-old.h"
#else
#include "dbus-shared-names.h"
#endif

#include "applet-struct.h"
#include "common-defs.h"
#include "volume-widget.h"
#include "mute-widget.h"
#include "applet-menu.h"
#include "applet-backend-alsamixer.h"  // cd_mixer_init_alsa (fallback alsa backend)
#include "applet-generic.h"
#include "applet-draw.h"
#include "applet-backend-sound-menu-old.h"

/*
// we can use icons designed for Unity, or more common icons that we are likely to find in icons themes.
static const gchar *_get_icon_from_state_unity (gint iState)
{
	switch (iState)
	{
		case MUTED: return "audio-volume-muted-panel";
		case ZERO_LEVEL: return "audio-volume-low-zero-panel";
		case LOW_LEVEL: return "audio-volume-low-panel";
		case MEDIUM_LEVEL: return "audio-volume-medium-panel";
		case HIGH_LEVEL: return "audio-volume-high-panel";
		case BLOCKED: return "audio-volume-muted-blocking-panel";
		default: return "audio-output-none-panel";
	}
}
static const gchar *_get_icon_from_state (gint iState)
{
	switch (iState)
	{
		case ZERO_LEVEL: return "audio-volume-off";
		case LOW_LEVEL: return "audio-volume-low";
		case MEDIUM_LEVEL: return "audio-volume-medium";
		case HIGH_LEVEL: return "audio-volume-high";
		default: return "audio-volume-muted";
	}
}*/

  ///////////
 // PROXY //
///////////

static void
on_sound_state_updated (DBusGProxy * proxy, gint iNewState, GldiModuleInstance *myApplet)
{
	cd_debug ("%s (iNewState : %d)", __func__, iNewState);
	CD_APPLET_ENTER;
	if (iNewState != myData.iCurrentState)
	{
		myData.iCurrentState = iNewState;
		gboolean bIsMute = (iNewState == MUTED
		|| iNewState == UNAVAILABLE
		|| iNewState == BLOCKED);
		if (myData.bIsMute != bIsMute)
		{
			myData.bIsMute = bIsMute;
			cd_update_icon ();
		}
	}
	CD_APPLET_LEAVE();
}

static int _get_volume (void)
{
	if (myData.volume_widget)
		return (int)volume_widget_get_current_volume (myData.volume_widget);
	else
		return 0;
}

static void _set_volume (int iVolume)
{
	if (myData.volume_widget)
		volume_widget_update (VOLUME_WIDGET(myData.volume_widget), (gdouble)iVolume, "scroll updates");
}

static void _toggle_mute (void)
{
	if (myData.mute_widget)
		mute_widget_toggle (MUTE_WIDGET (myData.mute_widget));
}

static void _show_menu (void)
{
	cd_indicator_show_menu (myData.pIndicator);
}

static void _stop (void)
{
	cd_indicator_destroy (myData.pIndicator);
	
	g_list_free (myData.transport_widgets_list);
}

static void cd_sound_on_connect (GldiModuleInstance *myApplet)
{
	cd_debug ("%s ()", __func__);
	// the sound service is up and running, stop the alsa mixer if ever we initialized it before.
	cd_stop ();
	
	// and set the interface
	myData.ctl.get_volume = _get_volume;
	myData.ctl.set_volume = _set_volume;
	myData.ctl.toggle_mute = _toggle_mute;
	myData.ctl.show_hide = _show_menu;
	myData.ctl.stop = _stop;
	myData.ctl.reload = cd_update_icon;
	
	// connect to the service signals.
	dbus_g_proxy_add_signal(myData.pIndicator->pServiceProxy, INDICATOR_SOUND_SIGNAL_STATE_UPDATE, G_TYPE_INT, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.pIndicator->pServiceProxy,
		INDICATOR_SOUND_SIGNAL_STATE_UPDATE,
		G_CALLBACK(on_sound_state_updated),
		myApplet,
		NULL);
}

static void cd_sound_on_disconnect (GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_debug ("%s", __func__);
	
	if (myData.ctl.get_volume == _get_volume)  // the backend was set, unset it
	{
		memset (&myData.ctl, 0, sizeof (CDSoundCtl));
		
		cd_debug ("clean");
		myData.volume_widget = NULL;
		myData.transport_widgets_list = NULL;
		myData.voip_widget = NULL;
		myData.mute_widget = NULL;
	}
	
	// no (more) sound service, now rely on alsa.
	cd_mixer_init_alsa ();
	
	CD_APPLET_LEAVE();
}

static void _on_got_sound_state (DBusGProxy *proxy, DBusGProxyCall *call_id, GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	int iCurrentState = 0;
	gboolean bSuccess = dbus_g_proxy_end_call (proxy,
		call_id,
		NULL,
		G_TYPE_INT,
		&iCurrentState,
		G_TYPE_INVALID);
	cd_debug ("got sound state: %d", iCurrentState);
	
	// update the icon.
	if (bSuccess)
	{
		myData.iCurrentState = iCurrentState;
		myData.bIsMute = (iCurrentState == MUTED
		|| iCurrentState == UNAVAILABLE
		|| iCurrentState == BLOCKED);
		myData.iCurrentVolume = _get_volume ();
		
		cd_update_icon ();
	}
	CD_APPLET_LEAVE();
}
static void cd_sound_get_initial_values (GldiModuleInstance *myApplet)
{
	// query the service to display initial values.
	dbus_g_proxy_begin_call (myData.pIndicator->pServiceProxy, "GetSoundState",
		(DBusGProxyCallNotify)_on_got_sound_state,
		myApplet,
		(GDestroyNotify) NULL,
		G_TYPE_INVALID);
}



void update_accessible_desc (double new_value)
{
	cd_debug ("%s (%p)", __func__, myData.volume_widget);
	if (!myData.volume_widget)
		return;
	
	myData.iCurrentVolume = (new_value < 0 ? _get_volume() : new_value);
	cd_update_icon ();
}

void cd_mixer_connect_to_sound_service (void)
{
	myData.pIndicator = cd_indicator_new (myApplet,
		INDICATOR_SOUND_DBUS_NAME,
		INDICATOR_SOUND_SERVICE_DBUS_OBJECT_PATH,
		INDICATOR_SOUND_DBUS_INTERFACE,
		INDICATOR_SOUND_MENU_DBUS_OBJECT_PATH,
		0);	
	myData.pIndicator->on_connect 			= cd_sound_on_connect;
	myData.pIndicator->on_disconnect 		= cd_sound_on_disconnect;
	myData.pIndicator->get_initial_values 	= cd_sound_get_initial_values;
	myData.pIndicator->add_menu_handler 	= cd_sound_add_menu_handler;
}
