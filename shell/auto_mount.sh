#!/bin/sh

MOUNT_DIR=/jovision/mnt

ALL_DISKS=$(ls /dev/ | grep -E "sd[a-z]+$")
for disk in $ALL_DISKS
do
    have_boot=$(parted /dev/$disk print | grep boot)
    if [ -n "${have_boot}" ];then
        continue
    fi

    if [ ! -d ${MOUNT_DIR}/$disk ];
    then
        mkdir -p ${MOUNT_DIR}/$disk
    fi

    mount -t xfs /dev/$disk ${MOUNT_DIR}/$disk
done

