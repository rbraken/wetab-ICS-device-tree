#!/system/bin/sh
#cp /system/bin/su /system/bin/su.bk
cp /system/xbin/special_su /system/bin/su
chmod 4755 /system/bin/su

# disable cursor blinking
echo -e '\033[?17;0;0c' > /dev/tty0
# Set some nice mixer defaults

alsa_amixer set Master 100
alsa_amixer set Headphone 100
alsa_amixer set Speaker 100
alsa_amixer set 'Mic Boost' 2
alsa_amixer set 'Capture' 25
alsa_amixer set 'Capture' cap

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
