#!/bin/bash

DISABLE_COVERAGE="apigen"

coverage=$(go test -cover -tags test -p 1 $@ | grep "coverage:" | grep -v $DISABLE_COVERAGE | awk -Fcoverage: '{print $2 " " $1 }' | awk -F% '($1 < 75.0 && $1 > 0.0) {print "Not enough coverage, expected 75.0%, found  " $0 "|" }')
if [ "$coverage" != "" ]; then
	echo $coverage | awk -v RS='|' '{print $0}'
	exit 1
fi

echo "================== Coverage test SUCCESSFUL =============="

exit 0
