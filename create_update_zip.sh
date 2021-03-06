#!/sbin/sh

# Create directory for uncompressed files to live before being zipped
mkdir -p $1/uncompressed

# get the kernel
dd if=/dev/block/mmcblk0p15 of=$1/uncompressed/boot.img

# copy meta files to pwd
cp -a /emmc/clockworkmod/zips/files/META-INF/ $1/uncompressed
if [ ! -e /system/xbin/busybox ]
then
   cp /emmc/clockworkmod/zips/files/updater-script.no_busybox $1/compressed/META-INF/com/google/android/updater-script
fi

# copy /system files to pwd
cp -a /system $1/uncompressed

# go to pwd
cd $1/uncompressed

# create custom rom
zip -r ../yourcustombackup.zip *

# go back
cd ..

# delete uncompressed
rm -rf uncompressed