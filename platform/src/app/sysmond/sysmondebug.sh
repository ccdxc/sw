#!/bin/sh

NOW=`date +"%Y-%m-%d-%H:%M:%S"`
SYSMODDUMP_LOCATION=/obfl/sysmonddumps

mkdir -p ${SYSMODDUMP_LOCATION}

# Check the current processes
top -b -n 1 -d 1 > ${SYSMODDUMP_LOCATION}/${NOW}.txt

top -m -b -n 1 -d 1 >> ${SYSMODDUMP_LOCATION}/${NOW}.txt

cat /proc/meminfo >> ${SYSMODDUMP_LOCATION}/${NOW}.txt

# Also don't keep any more than MAX_RECORDS
for f in `ls -t ${SYSMODDUMP_LOCATION} | tail -n +11`; do rm -rf ${SYSMODDUMP_LOCATION}/$f; done
