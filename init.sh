!/system/bin/sh
chmod 4755 /system/xbin/su

# disable cursor blinking
[ "$(getprop system_init.startsurfaceflinger)" = "0" ] && echo -e '\033[?17;0;0c' > /dev/tty0

#Audio Config
alsa_amixer set Master 100
alsa_amixer set Headphone 100
alsa_amixer set Speaker 100
alsa_amixer set 'Mic Boost' 2
alsa_amixer set 'Capture' 75
alsa_amixer set 'Capture' cap

#FIX 3G Permissions

chmod 0777 /dev/ttyUSB0
chmod 0777 /dev/ttyUSB1
chmod 0777 /dev/ttyUSB2
chmod 0777 /dev/ttyUSB3

#FIX resume
#echo radio-interface > /sys/power/wake_unlock
#echo PowerManagerService > /sys/power/wake_unlock

# import cmdline variables
for c in `cat /proc/cmdline`; do
       case $c in
               *=*)
                       eval $c
                       ;;
       esac
done

[ -z "$SDCARD" -o "$SDCARD" = "internal" ] && start sdcard
# for hardware acceleration of video
/system/bin/chmod 0666 /dev/crystalhd
