#Needed packages
make pkg-config libxft-dev build-essential libpangoxft-1.0-0 libpango1.0-dev libasound2-dev libdbus-1-dev libx11-dev libxinerama-dev xorg consolekit upower libfile-mimeinfo-perl

Install with `sudo apt-get install <list of packages>` on Debian/Ubuntu.

#Dwm files
Move the file dwm.desktop in extras/ to /usr/share/xsessions/
and move the icons.ttf in icon-font/ to /usr/share/fonts/truetype/

'sudo cp extras/dwm.desktop /usr/share/xsessions/'
'sudo cp icon-font/icons.ttf /usr/share/fonts/truetype/'

finally create the folder .dwm in your home with

`mkdir ~/.dwm`

and then copy the files autostart.sh and autostart_blocking.sh to ~/.dwm

`cp extras/autostart* ~/.dwm/`

