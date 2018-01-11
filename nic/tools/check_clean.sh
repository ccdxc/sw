#!/bin/bash

dir=$1
if [ -d $dir ]; then
    if [ -z "$(ls -A $dir)" ]; then
        exit 0
    else
        echo "ERROR: $dir directory is not empty after clean. Please clean properly"
        exit 1
    fi
else
    exit 0
fi
