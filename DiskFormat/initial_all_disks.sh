#!/bin/sh 
SCRIPT_PATH=$(dirname `readlink -f $0`)

get_confirm()
{
	echo "Please select disks from the list(all for all disks, no for exit):"
	while true
	do 
		read x
		case "$x" in
			all | All | ALL )
			return 1;;

			n | N | no | NO )
			return 0;;

			* )
				input_disks=$x
				
				find=0
				#check input disks's legality
				for disk in ${input_disks}
				do
					have_disk=$(echo ${spare_disks} | grep -w $disk)
					if [ -z "${have_disk}" ];then
					  echo "Please input disks from the list(all for all disks, no for exit):"
						find=0
						break
					fi
					
					find=1	
				done
				
				if [ $find -eq 1 ];then
					spare_disks=$x
					return 1
				fi
				;;
		esac
	done	
}

echo ${SCRIPT_PATH}
cd ${SCRIPT_PATH}

chmod +x ./umount_all_points.sh
./umount_all_points.sh
echo "umount all points ok"

#list all disks
disks=`ls -l /dev/ | awk '{printf("%s\n", $10)}' | grep -E "sd[a-z]+$"`
echo "DISKS: "${disks}

spare_disks=""
#check if disk have partitions or lvm
for disk in $disks
do     
	have_partition=$(ls -l /dev | awk '{printf("%s\n", $10)}' | grep -E "${disk}[0-9]+$")
	if [ -n "${have_partition}" ];then
		echo "${disk} have partition, so it can't be initialled"
		continue
	fi
	
	have_lvm=$(pvdisplay | grep ${disk})
	if [ -n "${have_lvm}" ];then
		echo "${disk} have lvm, so it can't be initialled"
		continue
	fi
	
	spare_disks=${spare_disks}" "${disk}
done

if [ -z "${spare_disks}" ];then
	echo "have no spare disks, exit"
	exit
fi

echo "the list of spare disks is: ${spare_disks}"
if get_confirm
then
	echo "bye"
	exit
fi
	
chmod +x ${SCRIPT_PATH}
for disk in ${spare_disks}
do
	./mount_one_disk.sh $disk
done

echo
echo "Initial all spare disk ok"
