#!/bin/bash

#Replace absoulte links to relative link
if [ ! $# -eq 1 ]
then
    echo "Invalid number of arguments"
    exit 1
fi

symfiles=$(find $1 -type l)
symfiles=(${symfiles// / })
for i in "${symfiles[@]}";
do
 dirname=$(dirname $i)
 rel_path=$(realpath --relative-to=$dirname $i)
 ln -sf $rel_path $i
done


