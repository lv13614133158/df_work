#!/bin/sh    
# IDPS stop script 

APP="IDPS"
MONITOR_SCRIPT="IDPS_start.sh"

echo "killall $MONITOR_SCRIPT"
killall $MONITOR_SCRIPT

echo "killall $APP"
killall $APP
