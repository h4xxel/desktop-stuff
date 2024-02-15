#!/bin/sh
nitrogen --restore

#/usr/lib/policykit-1-gnome/polkit-gnome-authentication-agent-1 &
xsettingsd &
dunst &
/usr/lib/mate-polkit/polkit-mate-authentication-agent-1 &
nm-applet &
blueman-applet &
firewall-applet &
fix-touchpad &
udiskie -Ans -f nemo &
setxkbmap -option compose:caps
xmodmap  -e 'keycode 78 = Caps_Lock'
xmodmap -e "clear mod4"
xmodmap -e "clear mod1"
xmodmap -e "add mod4 = Alt_L"
xmodmap -e "add mod1 = Super_L"


#syndaemon -i 0.25 -t -d
#
autorandr -c

xss-lock -- custom-lock &
nextcloud --background &
keepassxc &
#pasystray &

xset s off
xset -dpms
