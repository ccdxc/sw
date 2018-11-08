#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Invalid number of arguments"
    echo " $0 <proto-gen-dir>"
    exit -1
fi

cd $1
ls *delphi.hpp | awk '{print "#include \"gen/proto/" $1 "\""}' > delphi_objects.hpp
