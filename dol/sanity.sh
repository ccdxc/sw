#! /bin/bash

DOL=${WS_TOP}/hack/dol
NIC=${WS_TOP}/hack/saratk/nic/
OBJ=${WS_TOP}/hack/saratk/nic/obj

CAPRI_MOCK_MODE=1 LD_LIBRARY_PATH=$NIC/obj $NIC/obj/hal_stub -c hal.json 2>&1 > hal.log &
HALPID=$!
echo "Started HAL @ PID=$HALPID"

$DOL/mockmodel/mockmodel &
MOCKMODELPID=$!
echo "Started MockModel @PID=$MOCKMODELPID"

sleep 30
$DOL/main.py -m dol_test_modules.list
status=$?

echo "Stopping HAL @PID=$HALPID"
kill -9 $HALPID
echo "Stopping MOCKMODEL @PID=$MOCKMODELPID"
kill -9 $MOCKMODELPID

exit $status
