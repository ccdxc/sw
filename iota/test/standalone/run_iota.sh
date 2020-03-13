#!/bin/bash

cd /sw/iota
cat /warmd.json

./iota.py --testbed /warmd.json $@
ret=$?

cp -v testsuite_*_results.json /testcase_result_export

exit $ret
