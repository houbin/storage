#!/bin/sh
#umount partitions in a disk
LOG_PATH=./format_partition.log
exec 1>>${LOG_PATH} 2>>1
 
FORMAT_DISK=$1

if [ "${FORMAT_DISK}" == "" ];then
    echo "$0: no input disk"
    exit -1
fi

ls -l /dev/${FORMAT_DISK}
if [ $? -ne 0 ];then 
    echo "$0: input disk does't exist"
    exit -1
fi

PARTITION_NUMBER_LIST=`parted /dev/${FORMAT_DISK} print | grep -A128 Number | grep -v Number | grep -v '^$' | awk '{printf("%d\n", $1)}'`

for PARTITION_NUMBER in ${PARTITION_NUMBER_LIST}; 
do
    parted /dev/${FORMAT_DISK} << END
    rm ${PARTITION_NUMBER}
    quit
END
done
exit 0
