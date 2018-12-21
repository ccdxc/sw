#!/bin/sh

if [ -z $ARCH ]
then
    echo "ARCH variable not set. Use x86_64 or aarch64"
    exit 0
else
    echo "Running test for [$ARCH]....."
fi

export LD_LIBRARY_PATH=../../../gen/$ARCH/lib/
export PALAPP=../../../gen/$ARCH/bin/palapp

APP1="HAL"
APP2="NICMGR"

LARGEALLOCATION=40960
SMALLALLOCATION=1023
MINALLOCATION=4096
RANDOMALLOCATION=5412

function move_region_files() {
    if [ $ARCH = "x86_64" ]
    then
	if [ -z $TESTNAME -o -z $NO_CLEAN ] 
	then
	    echo "Cleaning up previous regions if any."
	    rm -rf /root/.palreg*
	else
	    DIR=$TESTNAME-`date +'%s'`
	    printf "\nStoring the region files generated in $DIR"
	    mkdir -p pal_result/$DIR
	    mv /root/.palreg* pal_result/$DIR/
	fi
    fi
}

function run_tests() {
    i=0
    CMDS=("$@")
    for CMD in "${CMDS[@]}";
    do
        printf "\n\n\n+++++++++++++++ [TEST-$i] ++++++++++++++\n"
        echo "CMD : $CMD" 
	$CMD

	if [ $? -ne 0 ]
	then
	    printf "TEST FAILED"
	    move_region_files
	    exit 125
	fi

	((++i))
    done
}

function print_metadata() {
        ../../../gen/$ARCH/bin/palapp -p
}

function run() {
    move_region_files 
    CMDS=("$@")
    run_tests "${CMDS[@]}"
    print_metadata
}

TESTNAME="test_hal_init"
printf "\nTEST for HAL-like init"
# Add all your test cases here
declare -a TEST_CASES_HAL='("$PALAPP -a -n HAL -r lif2qstate_map -s 8192"
                            "$PALAPP -a -n HAL -r ipsec-cb -s 4096 -e -c"
                            "$PALAPP -a -n HAL -r nmdr-rx -s 4096 -c"
                            "$PALAPP -a -n HAL -r nmdr-tx -s 4096 -c"
                            "$PALAPP -a -n HAL -r ipsec-nmdr-rx -s 4096 -e -c"
                            "$PALAPP -a -n HAL -r ipsec-nmdr-tx -s 4096 -e -c"
                            "$PALAPP -g -n HAL -r ipsec-nmdr-tx"
                            "$PALAPP -a -n HAL -r ipsec_cb_barco -s 4096 -c")' 

run "${TEST_CASES_HAL[@]}"

TESTNAME="test_multiple_access"
printf "\nTEST for multiple alloc-and-free of same size region"
declare -a TEST_CASE_MULTI='("$PALAPP -a -n HAL -r lif2qstate_map -s 8192"
                            "$PALAPP -f -n HAL -r lif2qstate_map"
			    "$PALAPP -a -n HAL -r lif2qstate_map1 -s 8192"
                            "$PALAPP -f -n HAL -r lif2qstate_map1"
			    "$PALAPP -a -n HAL -r lif2qstate_map2 -s 8192"
                            "$PALAPP -f -n HAL -r lif2qstate_map2"
			    "$PALAPP -a -n HAL -r lif2qstate_map3 -s 8192"
                            "$PALAPP -f -n HAL -r lif2qstate_map3"
			    "$PALAPP -a -n HAL -r lif2qstate_map4 -s 8192"
                            "$PALAPP -f -n HAL -r lif2qstate_map4"
			    "$PALAPP -a -n HAL -r lif2qstate_map5 -s 8192"
                            "$PALAPP -f -n HAL -r lif2qstate_map5"
			    "$PALAPP -a -n HAL -r lif2qstate_map6 -s 8192"
                            "$PALAPP -f -n HAL -r lif2qstate_map6"
			    "$PALAPP -a -n HAL -r lif2qstate_map7 -s 8192")'

run "${TEST_CASE_MULTI[@]}"

TESTNAME="test_coalesce"
printf "\nTEST for coalesce logic"
declare -a TEST_CASE_COALESCE='("$PALAPP -a -n HAL -r lif2qstate_map -s 8192"
                                "$PALAPP -a -n HAL -r lif2qstate_map1 -s 8192"
                                "$PALAPP -a -n HAL -r lif2qstate_map2 -s 8192"
                                "$PALAPP -a -n HAL -r lif2qstate_map3 -s 8192"
                                "$PALAPP -f -n HAL -r lif2qstate_map"
                                "$PALAPP -f -n HAL -r lif2qstate_map2"
                                "$PALAPP -f -n HAL -r lif2qstate_map3"
                                "$PALAPP -f -n HAL -r lif2qstate_map1")'

run "${TEST_CASE_COALESCE[@]}"

TESTNAME="test_splits"
printf "\nTEST to exercise splits"
declare -a TEST_CASE_SPLIT='("$PALAPP -a -n HAL -r lif2qstate_map -s 12000"
			     "$PALAPP -a -n HAL -r lif2qstate_map -s 12000"
                             "$PALAPP -f -n HAL -r lif2qstate_map"
                             "$PALAPP -a -n HAL -r lif2qstate_map2 -s 4096")'

run "${TEST_CASE_SPLIT[@]}"

TESTNAME="test_access"
printf "\nTEST to exercise access control code"
declare -a TEST_CASE_ACCESS='("$PALAPP -a -n HAL -r lif2qstate_map -s 8192 -e"
			      "$PALAPP -a -n HAL -r ipsec_cb -s 8192"
                              "$PALAPP -n HAL -r lif2qstate_map -d HALWRITEAAAAAAAAAAAA"
                              "$PALAPP -n NICMGR -r ipsec_cb -d NICWRITEDEADBEEF"
                              "$PALAPP -n NICMGR -r lif2qstate_map -d NICWRITEZZZZZZZZZZZZ")'

run "${TEST_CASE_ACCESS[@]}"

TESTNAME="test_access_2"
printf "\nTEST to exercise access control code after free"
declare -a TEST_CASE_ACCES2='("$PALAPP -a -n HAL -r lif2qstate_map -s 8192 -e"
                              "$PALAPP -n HAL -r lif2qstate_map -d HALWRITEAAAAAAAAAAAA"
                              "$PALAPP -n NICMGR -r lif2qstate_map -d NICWRITEDEADBEEF"
			      "$PALAPP -f -n HAL -r lif2qstate_map"
			      "$PALAPP -a -n HAL -r lif2qstate_map -s 8192"
                              "$PALAPP -n NICMGR -r lif2qstate_map -d NICWRITEZZZZZZZZZZZZ")'

run "${TEST_CASE_ACCES2[@]}"
