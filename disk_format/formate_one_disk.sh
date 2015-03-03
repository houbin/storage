#!/bin/sh 
JOVISION=/jovision/mnt
FORMAT_DISK=$1
SYS_DISK="sda"

if [ -z "${FORMAT_DISK}" ];then
    echo "please input the format disk"
    exit 1
fi

#judge which disk had boot partition
get_sys_disk()
{
    ALL_DISKS=$(ls -l /dev/ | awk '{printf("%s\n", $10)}' | grep -E "sd[a-z]+$")
    for disk in $ALL_DISKS
    do
        have_boot=$(parted /dev/$disk print | grep boot)
        if [ -n "${have_boot}" ];then
            SYS_DISK=$disk
        fi
    done
}

get_sys_disk

if [ "${FORMAT_DISK}" = "${SYS_DISK}" ];then
    echo "cannot format system disk"
    exit 1
fi

if [ -d $JOVISION/${FORMAT_DISK} ];then
    if_mount=`mount|grep ${FORMAT_DISK}`
    if [ -n "${if_mount}" ]; then
        umount $JOVISION/${FORMAT_DISK}
    fi
else
    mkdir -p $JOVISION/${FORMAT_DISK}
fi

dd if=/dev/zero of=/dev/${FORMAT_DISK} count=1 bs=512

mkfs.xfs -f /dev/${FORMAT_DISK}
mount -t xfs /dev/${FORMAT_DISK} $JOVISION/${FORMAT_DISK}

SCRIPT_PATH=$(dirname `readlink -f $0`)
cd ${SCRIPT_PATH}

chmod +x ./fallocate_one_disk.sh
./fallocate_one_disk.sh ${FORMAT_DISK}

