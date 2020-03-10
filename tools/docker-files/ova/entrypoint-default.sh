#!/bin/bash

set -x
set -e
umask 000
echo | ./VMware-ovftool-4.3.0-13981069-lin.x86_64.bundle --eulas-agreed
# Build the OVA and download it using buildit tool
build_id=`/t/buildit b -i /t/pen-install.iso pensando/e2e-images/venice/venice.yaml | grep 'Build ID: ' | uniq | sed 's/Build ID: //'`
# Directory to keep the output of the build
if [ $# -eq 1 -a "$1" == "apulu" ]; then
    echo "building ova and qcow2 for apulu venice"
    dir="output-apulu"
else
    echo "building ova and qcow2 for iris venice"
    dir="output"
fi
mkdir $dir
./buildit i p -o $dir/venice.ova $build_id
# Untar the OVA so we can modify the .ovf file and repackage it
chmod -R 777 $dir
file_list=`tar -C $dir -xvf $dir/venice.ova | xargs`
rm -f $dir/venice.ova
# We need to do two things
# 1. For OVF, insert the OVA properties and repackage the OVA
# 2. For vmdk, create qcow2
for file in $file_list; do
    if [[ $file == *".ovf" ]]; then
        # Run python script to insert the OVA properties and repackage the OVA
        python venice_ova_insert_ova_properties.py $dir/$file
    elif [[ $file == *".vmdk" ]]; then
        # Run qemu-img convert here
	qemu-img convert -f vmdk -O qcow2 $dir/$file $dir/venice.qcow2
    fi
done
chmod -R 777 $dir 
for file in $file_list; do
    if [[ $file != "." ]]; then
        rm -f $dir/$file
    fi
done
