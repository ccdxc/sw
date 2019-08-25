#!/bin/sh

NOW=`date +"%Y%m%d%H%M%S"`
TEMP_LOCATION=/data/debug-${NOW}-$1
FINAL_LOCATION=/data/lastlog

mkdir -p ${TEMP_LOCATION}

# Check CPU load
top -b -n 1 -d 1 > ${TEMP_LOCATION}/top_start.txt

# Copy everything under `/log`
cp -a /var/log ${TEMP_LOCATION}/log

# One final CPU load check
top -b -n 1 -d 1 > ${TEMP_LOCATION}/top_end.txt

mv ${FINAL_LOCATION} ${FINAL_LOCATION}.old
rm -rf ${FINAL_LOCATION}.old
mv ${TEMP_LOCATION} ${FINAL_LOCATION}
# in case `mv` fails
rm -rf ${TEMP_LOCATION}
