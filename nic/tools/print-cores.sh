#!/bin/bash

echo "Checking for cores....."
for core_file in core.*; do
    [ -f "$core_file" ] || continue
    echo "********************CORE BEGIN************************" 
    exec_file="$(strings $core_file  | grep ^/ | tail -1)"
    echo "Exec file : $exec_file"
    echo "Core file : $core_file"
    gdb -batch -ex bt $exec_file $core_file
    echo "*********************CORE END************************" 
done
