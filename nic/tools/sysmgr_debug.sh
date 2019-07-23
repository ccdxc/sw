#!/bin/sh

NOW=`date +"%Y%m%d%H%M%S"`
LOG_LOCATION=/data/debug-${NOW}-$1/

mkdir -p ${LOG_LOCATION}

# Check CPU load
top -b -n 1 -d 1 > ${LOG_LOCATION}/top_start.txt

# Copy everything under `/log` to `/data/debug-${NOW}/log`
cp -a /var/log ${LOG_LOCATION}/log

# One final CPU load check
top -b -n 1 -d 1 > ${LOG_LOCATION}/top_end.txt
