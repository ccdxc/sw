#! /bin/bash
DOL=${WS_TOP}/sw/dol
NIC=${WS_TOP}/sw/nic/
MOCKMODEL=${DOL}/mockmodel
LOG_FILE=${DOL}/ut_run.log
declare -a ut_files=("test_trigger_with_mock_model.py" "test_packet.py")

export PYTHONPATH=${DOL}:${NIC}
export WS_TOP=${WS_TOP}/sw
export MODEL_SOCK_PATH=${WS_TOP}/nic

#Build Mockmodel
cd ${MOCKMODEL} && make > ${LOG_FILE}  2>&1
out=$?
if [ $out -ne 0 ];then
    exit $out
fi

#Run all the unittest modules.
for module in "${ut_files[@]}"
do
	cd ${DOL}/unit_test && python3 -m unittest discover -v --pattern="$module"  > ${LOG_FILE}  2>&1
	out=$?
	if [ $out -ne 0 ];then
            exit $out
        fi

done

exit 0
