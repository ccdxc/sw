#!/bin/sh

COREMGR=/sw/bazel-bin/nic/sysmgr/coremgr/src/coremgr

# 8 MB of random data
dd if=/dev/urandom of=rand.dat bs=4096 count=2048
if [ $? -ne 0 ]
then
    exit $?
fi

timeout 60s $COREMGR -P ./cores --pid 999 --executable rand -M 1 < rand.dat
if [ $? -ne 0 ]
then
    exit $?
fi

if [ ! -e ./cores ]
then
    echo "'./cores' not created"
fi

timeout 60s $COREMGR -P ./cores --pid 999 --executable rand -M 1 < rand.dat
if [ $? -ne 0 ]
then
    exit $?
fi

CORE_COUNT=`ls cores/core* | wc -l`
if [ ${CORE_COUNT} -ne 1 ]
then
    echo "Too many/few cores ${CORE_COUNT}"
    exit -1
fi

for f in `ls cores/core*`
do
    gunzip $f
    cmp ${f%.gz} rand.dat
    if [ $? -ne 0 ]
    then
        echo "Decompressed file doesn't match rand.dat"
        exit $?
    fi
done

rm -rf cores
rm -rf rand.dat

exit 0
