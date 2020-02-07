#!/usr/bin/env bash


echo "Checking for cores....."
for core_file in core.*; do
    [ -f "$core_file" ] || continue
    echo "********************CORE BEGIN************************" 
    exec_file="$(strings $core_file  | grep ^/ | tail -1)"
    exec_file=`file $core_file  | grep -o -P '(?<=execfn:).*(?=,)' | tr -d "\'"`
    echo "Exec file : $exec_file"
    echo "Core file : $core_file"
    gdb -batch -ex bt $exec_file $core_file
    echo "*********************CORE END************************" 
done
