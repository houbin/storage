#!/bin/sh

#mount one disk
#usage: $0 sdb

MOUNT_DISK=$1
MOUNT_PARENT_DIR=/jovision/

if [ "${MOUNT_DISK}" == "" ];then
	echo "no input disk"
	exit -1
fi

HAVE_MOUNT_DIR=`ls -l ${MOUNT_PARENT_DIR} | grep ${MOUNT_DISK}`
if [ -z "${HAVE_MOUNT_DIR}" ];then
	mkdir ${MOUNT_PARENT_DIR}/${MOUNT_DISK}
fi

mkfs.xfs -f /dev/${MOUNT_DISK}
mount -t xfs /dev/${MOUNT_DISK} ${MOUNT_PARENT_DIR}/${MOUNT_DISK}

