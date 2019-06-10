#!/bin/sh

if [ "$1" == "remove-config" ]; then
    dir_list="/sysconfig/config0/ /sysconfig/config1/ /update/ /data/"
elif [ "$1" == "factory-default" ]; then
    dir_list="/sysconfig/config0/ /sysconfig/config1/ /update/ /data/ /obfl/"
else
    echo "Invalid arg"
    echo "usage: clear_nic_config.sh"
    echo "action:"
    echo "remove-config"
    echo "factory-default"
    exit
fi

for dir in $dir_list; do
    echo "Removing content from $dir"
    cd $dir && find . -name lost+found -prune -o -exec rm -rf '{}' ';'
done

#sync the disk
sync
