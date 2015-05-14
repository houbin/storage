#!/bin/bash

JOVISION_MOUNT_DIR=/jovision/mnt
JOVISION_SHELL_DIR=/jovision/shell
JOVISION_BIN_DIR=/jovision/bin
JOVISION_LIB_DIR=/jovision/lib

LIBSTORAGE_SO=libstorage.so
JCVR_EXE=JCVR

SCRIPT_PATH=$(dirname `readlink -f $0`)
cd ${SCRIPT_PATH}

# clear old files
\rm -f /etc/rc.d/init.d/record_storage
\rm -rf ${JOVISION_SHELL_DIR}
\rm -rf ${JOVISION_BIN_DIR}
\rm -rf ${JOVISION_LIB_DIR}

# mkdir
\mkdir -p ${JOVISION_MOUNT_DIR}
\mkdir -p ${JOVISION_SHELL_DIR}
\mkdir -p ${JOVISION_BIN_DIR}
\mkdir -p ${JOVISION_LIB_DIR}
\mkdir -p /var/log/storage/

# copy new files
\cp -rf shell /jovision/
\cp -f record_storage /etc/init.d/
\cp -f storage.conf /jovision/

\cp -f ${LIBSTORAGE_SO} ${JOVISION_LIB_DIR}
\cp -f ${JCVR_EXE} ${JOVISION_BIN_DIR}

chmod +x ${JOVISION_SHELL_DIR}/*
chmod +x ${JOVISION_BIN_DIR}/*
chmod +x ${JOVISION_LIB_DIR}/*
chmod +x /etc/init.d/record_storage

chkconfig --add record_storage

# add LD_LIBRARY_PATH
if_has_jovision_path=$(echo $LD_LIBRARY_PATH | grep ${JOVISION_LIB_DIR})
if [ -z "${if_has_jovision_path}" ];
then
    echo "export LD_LIBRARY_PATH=${JOVISION_LIB_DIR}:LD_LIBRARY_PATH" >> /etc/bashrc
    source /etc/bashrc
fi
