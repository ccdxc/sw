#! /bin/bash
cd `dirname $0`
DOL=$PWD
NIC=$PWD/../nic/
MOCKMODEL=${DOL}/mockmodel
LOG_FILE=${DOL}/ut_run.log
declare -a ut_files=("test_trigger_with_mock_model.py" "test_packet.py",
 "test_dol_main.py")

export PYTHONPATH=${DOL}:${NIC}
export WS_TOP=${DOL}/../
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
	cd ${DOL}/unit_test && python3 -m unittest discover -v --pattern="$module"  2>&1 | tee  ${LOG_FILE}
	out=${PIPESTATUS[0]}
	if [ $out -ne 0 ];then
            exit $out
        fi

done

exit 0
