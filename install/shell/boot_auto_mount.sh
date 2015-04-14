#!/bin/sh

LOG_FILE=/var/log/storage/boot_auto_mount.log

MOUNT_DIR=/jovision/mnt

exec 1>${LOG_FILE}
exec 2>&1

#judge which disk had boot partition
get_sys_disk()
{
    ALL_DISKS=$(ls -l /dev/ | awk '{printf("%s\n", $10)}' | grep -E "sd[a-z]+$")
    for disk in $ALL_DISKS
    do
        have_boot=$(parted /dev/$disk print | grep boot)
        if [ -n "${have_boot}" ];then
            SYS_DISK=$disk
            return 0
        fi
    done
}

get_sys_disk

# discard sys disk
DATA_DISKS=$(ls /dev/ | grep -E "sd[a-z]+$" | grep -v ${SYS_DISK})

echo "formate all disk start"
VALID_DATA_DISKS=""
for disk in $DATA_DISKS
do
    IF_BLOCK=`file /dev/$disk | grep block`
    if [ "${IF_BLOCK}" == "" ];
    then
        echo "$disk is not block device"
        continue
    fi
    echo "$disk is block device"

    if [ ! -d ${MOUNT_DIR}/$disk ];
    then
        mkdir -p ${MOUNT_DIR}/$disk
    fi

    VALID_DATA_DISKS=${VALID_DATA_DISKS}" "$disk

    # try to mount
    mount -t xfs /dev/$disk ${MOUNT_DIR}/$disk

    # check if file of "file_count" exist
    if [ ! -f ${MOUNT_DIR}/$disk/file_count ];
    then
        echo "$disk check failed, and format it"
        /jovision/shell/format_one_disk $disk &
        continue
    fi
    echo "$disk check ok, and don't need to format it"
done
echo "formate all disk end"

echo "wait all valid disk format ok start, valid disk is ${VALID_DATA_DISKS}"
# wait for end format of all disk
for disk in $VALID_DATA_DISKS
do
    while [ ! -f ${MOUNT_DIR}/$disk/file_count ];
    do
        echo "$disk wait failed, sleep 15s"
        sleep 15
        continue
    done

    echo "$disk wait ok"
done
echo "wait all valid disk format ok end"

exit

