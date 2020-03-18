#! /bin/bash

export NICDIR=$(pwd)

make -C ${NICDIR}/apollo/test/athena_app/ clean
make -C ${NICDIR}/apollo/test/athena_app/

result=$?
if [ $result -ne 0 ]
then
	echo "athena_app build failed" && exit 1
fi

echo "athena_app build success"
exit 0
