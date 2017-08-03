#! /bin/bash

DOL=${WS_TOP}/hack/dol
NIC=${WS_TOP}/hack/saratk/nic/
OBJ=${WS_TOP}/hack/saratk/nic/obj
sudo LD_LIBRARY_PATH=$OBJ $OBJ/hal -c $NIC/conf/hal.json > hal.log 2>&1 &
HALPID=$!
echo "Started HAL @ PID=$HALPID

$DOL/main.py -C
status=$?

echo "Stopping HAL @PID=$HALPID"
kill -9 $HALPID

exit $status
