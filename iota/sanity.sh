#! /bin/bash
export DIR_IOTA=`dirname $0`
export DIR_GO_IOTA=$GOPATH/src/github.com/pensando/sw/iota

cd $DIR_IOTA

function ErrorCheckNExit() {
    if [ "$1" = "0" ];then
        return
    fi

    echo "ERROR: $2 FAILED"
    exit 1
}

make
ErrorCheckNExit $? "make"

cd $DIR_GO_IOTA && mkdir -p /var/iota && go  test -v -p=1 ./svcs/agent/...
ErrorCheckNExit $? "Agent unit tests failed"

infra_sanity="./iota.py  --testbed testbeds/sample.json --testsuite hw_naples_config  --debug --skip-firmware-upgrade --dryrun"

$infra_sanity
ErrorCheckNExit $? $infra_sanity


exit 0
