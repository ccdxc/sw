#! /bin/bash

OBJ=${WS_TOP}/hack/saratk/nic/obj
GEN_TEST_RESULTS_DIR=${WS_TOP}/hack/saratk/nic/gen/test_results
VAL_DIR=${WS_TOP}/hack/saratk/nic/tools/valgrind
HAL_CONFIG_PATH=${WS_TOP}/hack/saratk/nic/conf

OPTIND=1
valgrind=0
while getopts ":v" opt; do
    case $opt in
        v)
            valgrind=1
            ;;
    esac
done

if [[ $valgrind -eq 1 ]]; then
    echo "Testing valgrind LIF: "
    sudo LD_LIBRARY_PATH=$OBJ WS_TOP=${WS_TOP}/ valgrind --tool=memcheck --leak-check=yes --xml=yes --xml-file=$VAL_DIR/lif_val.xml $OBJ/lif_test
else
    echo "Testing LIF: "
    ASIC_MOCK_MODE=1 LD_LIBRARY_PATH=$OBJ WS_TOP=${WS_TOP}/ HAL_CONFIG_PATH=${WS_TOP}/hack/saratk/nic/conf $OBJ/lif_test --gtest_output="xml:$GEN_TEST_RESULTS_DIR/lif_test.xml"
    echo "Testing UplinkIf: "
    ASIC_MOCK_MODE=1 LD_LIBRARY_PATH=$OBJ WS_TOP=${WS_TOP}/ HAL_CONFIG_PATH=${WS_TOP}/hack/saratk/nic/conf $OBJ/uplinkif_test --gtest_output="xml:$GEN_TEST_RESULTS_DIR/uplinkif_test.xml"
fi

shift $((OPTIND-1))
# [ "$1" = "--" ] && shift

# sudo LD_LIBRARY_PATH=./obj WS_TOP=/mnt/ws/work/hal_hack/ ./obj/lif_test

# $DOL/main.py --config-only
# status=$?
#
# echo "Stopping HAL @PID=$HALPID"
# sudo kill -9 $HALPID
#
# exit $status
