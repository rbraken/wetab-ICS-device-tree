#!/system/bin/sh
#cp /system/bin/su /system/bin/su.bk
#cp /system/xbin/special_su /system/bin/su
#chmod 4755 /system/bin/su

# disable cursor blinking
echo -e '\033[?17;0;0c' > /dev/tty0
