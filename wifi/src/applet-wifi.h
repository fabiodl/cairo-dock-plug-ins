#ifndef __APPLET_WIFI__
#define  __APPLET_WIFI__

#include <cairo-dock.h>

gboolean cd_wifi_timer (gpointer data);

void cd_wifi_get_data (void);

void cd_wifi_launch_measure (void);
gboolean cd_wifi_getStrength(void);

/*gboolean cd_wifi(gpointer data);
gboolean cd_wifi_getStrength(void);
float pourcent(float x, float y);
//void cd_wifi_init(gchar *origine);
void cd_wifi_wait(void);*/

#endif
