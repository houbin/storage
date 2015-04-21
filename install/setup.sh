#!/bin/sh

JOVISION_SHELL_DIR=/jovision/shell
DISK_MOUNT_DIR=/jovision/mnt

LIBSTORAGE_SO=libstorage.so
LIBNT88_SO=libnt88.so
JCVR_EXE=JCVR

mkdir -p /jovision/mnt

SCRIPT_PATH=$(dirname `readlink -f $0`)
cd ${SCRIPT_PATH}

# clear old files
\rm -f /etc/rc.d/init.d/record_storage
\rm -rf ${JOVISION_SHELL_DIR}

\rm -f /jovision/storage.conf

\rm -f /lib/${LIBNT88_SO}
\rm -f /lib/${LIBSTORAGE_SO}
\rm -f /usr/bin/${JCVR_EXE}

# copy new files
\cp -rf shell ${JOVISION_SHELL_DIR}
\cp -f record_storage /etc/rc.d/init.d/
\cp -f storage.conf /jovision/

\cp -f ${LIBNT88_SO} /lib/
\cp -f ${LIBSTORAGE_SO} /lib/
\cp -f ${JCVR_EXE} /usr/bin/

chmod +x ${JOVISION_SHELL_DIR}/*
chmod +x /etc/rc.d/init.d/record_storage
chmod +x /lib/${LIBSTORAGE_SO}
chmod +x /lib/${LIBNT88_SO}
chmod +x /usr/bin/${JCVR_EXE}

chkconfig --add record_storage
\mkdir -p /var/log/storage/

