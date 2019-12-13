#!/bin/sh

NOW=`date +"%Y%m%d%H%M%S"`
TOP=/obfl/sysmgr-dumps
LOCATION=${TOP}/debug-${NOW}-$1
MAX_DAYS="1"
MAX_RECORDS="11"

mkdir -p ${LOCATION}

# Check CPU load
top -b -n 1 -d 1 > ${LOCATION}/top_start.txt
top -b -n 1 -d 1 -m >> ${LOCATION}/top_start.txt

# Copy everything under `/log`
cp -a /var/log ${LOCATION}/log

# One final CPU load check
top -b -n 1 -d 1 > ${LOCATION}/top_end.txt
top -b -n 1 -d 1 -m >> ${LOCATION}/top_end.txt

# Delete old logs
find ${TOP} -mtime +${MAX_DAYS} -type d -prune -exec rm -rf {} \;

# Also don't keep any more than MAX_RECORDS
for f in `ls -t ${TOP} | tail -n +${MAX_RECORDS}`; do rm -rf ${TOP}/$f; done
