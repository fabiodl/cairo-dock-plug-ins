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

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <glib.h>

#include "applet-impulse.h"
#include "Impulse.h"

void _im_start (void)
{
	cd_debug ("Impulse: start im");
	im_start();
}

void _im_stop (void)
{
	cd_debug ("Impulse: stop im");
	//im_stop(); // FIXME => if stopped, the client is not stopped and im_getSnapshot(IM_FFT) give always the same thing...
}

static void _get_icons_list_without_separators (CDSharedMemory *pSharedMemory)
{
	if (pSharedMemory->pDock == NULL)
		return ;

	pSharedMemory->bIsUpdatingIconsList = TRUE;

	pSharedMemory->pIconsList = NULL;
	GList *ic;
	Icon *pIcon;
	for (ic = pSharedMemory->pDock->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		// cd_debug ("Impulse: icon name=%s", pIcon->cName);
		if (! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
			pSharedMemory->pIconsList = g_list_append (pSharedMemory->pIconsList, pIcon);
	}
	g_list_free (ic);
	pSharedMemory->bIsUpdatingIconsList = FALSE;
	cd_debug ("Impulse: updated icons list: %d", g_list_length(pSharedMemory->pIconsList));
}

static gboolean _animate_the_dock (CDSharedMemory *pSharedMemory)
{
	CD_APPLET_ENTER;
	// cd_debug ("Impulse: in");
	if (pSharedMemory->bIsUpdatingIconsList || pSharedMemory->pIconsList == NULL)
		CD_APPLET_LEAVE (TRUE);

	guint iIcons = 256 / g_list_length(pSharedMemory->pIconsList); // number of icons (without separators)

	double *array = im_getSnapshot(IM_FFT);
	int i;
	double l = 0.0;
	GList *ic = pSharedMemory->pIconsList;
	Icon *pIcon;
	for (i = 0; ic != NULL; i++) // i < 256
	{
		l += array[i]; // a sum for the average
		if (i != 0 && (i % iIcons) == 0)
		{
			pIcon = ic->data;
			// cd_debug ("Impulse: Average: i=%d, l=%f ; I=%d ; l/I=%f ; %s", i, l, iIcons, l/iIcons, pIcon->cName);
			if ((l/iIcons) > pSharedMemory->fMinValueToAnim) // animation
			{
				//cd_debug ("Impulse: animation on this icon=%s", pIcon->cName);
				cairo_dock_request_icon_animation (pIcon, pSharedMemory->pDock, pSharedMemory->cIconAnimation, pSharedMemory->iNbAnimations);
			}
			else if (pSharedMemory->bStopAnimations)
				cairo_dock_stop_icon_animation (pIcon);
			l = 0.0;
			ic = ic->next;
		}
	}
	//cd_debug ("Impulse: out");
	g_list_free (ic);
	CD_APPLET_LEAVE (TRUE);
}


/*void cd_impulse_start_animations (void)
{
	myData.iSidAnimate = g_timeout_add (myConfig.iLoopTime, (GSourceFunc) _animate_the_dock, NULL);
}*/

void cd_impulse_stop_animations (void)
{
	if (myData.pTask != NULL)
	{
		cairo_dock_discard_task (myData.pTask);
		myData.pTask = NULL;
	}
	if (myData.bPulseLaunched)
		_im_stop();
	// myData.bPulseLaunched = FALSE; //FIXME
}

static void _free_shared_memory (CDSharedMemory *pSharedMemory)
{
	g_free (pSharedMemory->cIconAnimation);
	g_list_free (pSharedMemory->pIconsList);
	g_free (pSharedMemory);
}

void cd_impulse_launch_task (void) //(CairoDockModuleInstance *myApplet)
{
	// PulseAudio Server
	if (! myData.bPulseLaunched)
	{
		_im_start(); // FIXME => if already started and stopped, it will crash... because not correctly stopped...
		myData.bPulseLaunched = TRUE;
	}

	// if a task is already launching
	if (myData.pTask != NULL)
	{
		cairo_dock_discard_task (myData.pTask);
		myData.pTask = NULL;
	}
	
	myData.pSharedMemory = g_new0 (CDSharedMemory, 1);
	myData.pSharedMemory->pIconsList = NULL; // without separators
	myData.pSharedMemory->bIsUpdatingIconsList = TRUE; // without separators
	myData.pSharedMemory->cIconAnimation = g_strdup (myConfig.cIconAnimation);
	myData.pSharedMemory->iNbAnimations = myConfig.iNbAnimations;
	myData.pSharedMemory->fMinValueToAnim = myConfig.fMinValueToAnim;
	myData.pSharedMemory->bStopAnimations = myConfig.bStopAnimations;
	myData.pSharedMemory->pDock = myConfig.pDock;
	// pSharedMemory->pApplet = myApplet;
	
	myData.pTask = cairo_dock_new_task_full (1,// TODO (SECOND) myConfig.iLoopTime,
		// (CairoDockGetDataAsyncFunc) _get_icons_list_without_separators,
		NULL,
		(CairoDockUpdateSyncFunc) _animate_the_dock,
		(GFreeFunc) _free_shared_memory,
		myData.pSharedMemory);
	_get_icons_list_without_separators (myData.pSharedMemory);
	cairo_dock_launch_task (myData.pTask);
}

gboolean cd_impulse_on_icon_changed (gpointer pUserData, Icon *pIcon, CairoDock *pDock)
{
	// launched and something has changed in the right dock
	//cd_debug ("Impulse: update needed? %d | %d", pDock, myConfig.pDock);
	if (myData.pTask != NULL && pDock == myConfig.pDock)
	{
		_get_icons_list_without_separators (myData.pSharedMemory);
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
