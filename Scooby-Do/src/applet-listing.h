
#ifndef __APPLET_LISTING__
#define  __APPLET_LISTING__


#include <cairo-dock.h>
#include "applet-struct.h"


void cd_do_free_entry (CDEntry *pEntry);
void cd_do_backup_entry (CDEntry *pInEntry, CDEntry *pFromEntry);

CDListing *cd_do_create_listing (void);
void cd_do_destroy_listing (CDListing *pListing);


gboolean cd_do_update_listing_notification (gpointer pUserData, CDListing *pListing, gboolean *bContinueAnimation);
gboolean cd_do_render_listing_notification (gpointer pUserData, CDListing *pListing, cairo_t *pCairoContext);

void cd_do_show_listing (CDEntry *pEntries, int iNbEntries);

void cd_do_hide_listing (void);


void cd_do_select_prev_next_entry_in_listing (gboolean bNext);
void cd_do_select_prev_next_page_in_listing (gboolean bNext);
void cd_do_select_last_first_entry_in_listing (gboolean bLast);
void cd_do_select_nth_entry_in_listing (int iNumEntry);


void cd_do_set_status (const gchar *cStatus);
void cd_do_set_status_printf (const gchar *cStatusFormat, ...);


#endif
