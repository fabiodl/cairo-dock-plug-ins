#!/bin/bash
#Auteur : necropotame
#Contact : adrien.pilleboue@gmail.com
#Version : 2008-03-12

dbus-binding-tool --mode=glib-server --prefix=cd_dbus_callback ./src/dbus_introspectable.xml > ./src/applet-dbus-spec.h

dbus-binding-tool --mode=glib-server --prefix=cd_dbus_applet ./src/dbus_applet_introspectable.xml > ./src/applet-dbus-applet-spec.h
