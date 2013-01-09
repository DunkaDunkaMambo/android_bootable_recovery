#!/sbin/sh

if [ ! -e /emmc/clockworkmod/bootanimation_backup.zip ]
then
    mv /system/media/bootanimation.zip /emmc/clockworkmod/bootanimation_backup.zip
else
    rm -rf /system/media/bootanimation.zip
fi

cp -a $1 /system/media/bootanimation.zip
chmod 644 /system/media/bootanimation.zip

return 0
