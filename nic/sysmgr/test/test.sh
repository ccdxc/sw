#!/bin/sh

bazel test //nic/sysmgr/... --jobs 1 --cache_test_results=no --test_output=all
RET_1=$?

timeout 60s /sw/bazel-bin/nic/sysmgr/src/sysmgr_test
RET_2=$?

cat *.log

echo "bazel test result = $RET_1"
echo "sysmgr_test result = $RET_2"
exit `expr "$RET_1" + "$RET_2"`
