#bin/sh
# umount all mount points that belongs to our
# umount /jovision/*
MOUNT_PARENT_DIR=/jovision

if [ -f ${MOUNT_PARENT_DIR} ];then
    echo "${MOUNT_PARENT_DIR} is a file"
    exit 1
elif [ ! -d ${MOUNT_PARENT_DIR} ]; then
    mkdir /jovision/
else
    umount ${MOUNT_PARENT_DIR}/*
    echo "umount ${MOUNT_PARENT_DIR}/* ok"
fi
exit 0
