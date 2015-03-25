#!/bin/sh

JOVISION_SHELL_DIR=/jovision/shell
DISK_FORMAT_DIR=/jovision/shell/disk_format
DISK_MOUNT_DIR=/jovision/mnt

mkdir -p /jovision/shell/disk_format
mkdir -p /jovision/mnt

SCRIPT_PATH=$(dirname `readlink -f $0`)
cd ${SCRIPT_PATH}/shell

\cp -f fallocate_one_disk.sh ${DISK_FORMAT_DIR}/
\cp -f format_one_disk.sh ${DISK_FORMAT_DIR}/
\cp -f auto_mount.sh ${JOVISION_SHELL_DIR}/
\cp -f storage_auto_mount /etc/rc.d/init.d/

chmod +x ${DISK_FORMAT_DIR}/fallocate_one_disk.sh
chmod +x ${DISK_FORMAT_DIR}/format_one_disk.sh
chmod +x ${JOVISION_SHELL_DIR}/auto_mount.sh
chmod +x /etc/rc.d/init.d/storage_auto_mount

chkconfig --add storage_auto_mount

