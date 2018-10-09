#!/bin/sh

rm -f *.log

bazel test //nic/sysmgr/... --jobs 1 --cache_test_results=no --test_output=all
RET_1=$?

pushd /usr/src/github.com/pensando/sw/nic/sysmgr/goexample && go build && popd

timeout 60s /sw/bazel-bin/nic/sysmgr/src/sysmgr_test
RET_2=$?

cat *.log

echo "bazel test result = $RET_1"
echo "sysmgr_test result = $RET_2"
exit `expr "$RET_1" + "$RET_2"`
