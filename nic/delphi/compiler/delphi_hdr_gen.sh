#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Invalid number of arguments"
    echo " $0 <proto-gen-dir>"
    exit -1
fi

cd $1/../..
find gen/proto/ -name "*delphi.hpp" | awk '{print "#include \"" $1 "\""}' > gen/proto/delphi_objects.hpp
