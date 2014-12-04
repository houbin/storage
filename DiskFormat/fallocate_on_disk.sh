#!/bin/sh
# uint M

mount_dir=/jovision

disk=$1

disk_size=`parted /dev/$disk unit MiB print | grep -A2 Number | grep -v Number | grep -v '^$' | sed "s/MiB//g" | awk '{printf("%d\n", $4)}'`

disk_use_size=$(echo "${disk_size} - 10000" | bc)
echo "$disk use size is ${disk_use_size}"

file_count=$(echo "${disk_use_size} / 256" | bc)
echo ${file_count}

i=0
while [ $i -lt ${file_count} ];
do
	file_name=$(printf 'record%05d' $i)
	echo $file_name

	fallocate -l 256m ${mount_dir}/${disk}/${file_name}
	i=$(($i + 1))

done





