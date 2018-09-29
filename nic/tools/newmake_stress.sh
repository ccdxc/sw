#! /bin/bash

NEWMAKE="/bin/make -j -f Makefile.nrmake"
X86_IRIS_MAKE="${NEWMAKE} PIPELINE=iris"
X86_GFT_MAKE="${NEWMAKE} PIPELINE=gft"
X86_AP2_MAKE="${NEWMAKE} PIPELINE=ap2"
rm -f newmakestress.log



for iter in `seq 1 $1`
do
    echo "Starting Iteration: $iter"
    ${X86_IRIS_MAKE} clean 2>&1 >> newmakestress.log || exit 1
    ${X86_GFT_MAKE} clean 2>&1 >> newmakestress.log || exit 1
    ${X86_AP2_MAKE} clean 2>&1 >> newmakestress.log || exit 1
    ${X86_IRIS_MAKE} 2>&1 >> newmakestress.log || exit 1
    ${X86_GFT_MAKE} 2>&1 >> newmakestress.log || exit 1
    ${X86_AP2_MAKE} 2>&1 >> newmakestress.log || exit 1
done
