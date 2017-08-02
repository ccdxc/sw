#!/bin/bash
TEST_FILES=`ls -1 *.p4`
for p4f in $TEST_FILES
do
    echo "Compile $p4f"
    ../capri-ncc.py --asm-out --pd-gen $p4f
    if [ $? != 0 ]
    then
        echo "Error"
        exit
    fi
done
