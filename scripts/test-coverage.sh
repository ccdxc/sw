#!/bin/bash

DISABLE_COVERAGE="apigen"

# run go test -cover
coverage=$(go test -cover -tags test -p 1 $@)

# check if any tests failed
testFail=$(echo "$coverage" | grep "FAIL")
if [ "$testFail" != "" ]; then
	echo "$coverage"
	echo "======================= Test Failed ====================="
	echo "$testFail"
	exit 1
fi

# Check coverage of all packages
coverFail=$(echo "$coverage" | grep "coverage:" | grep -v $DISABLE_COVERAGE | awk -Fcoverage: '{print $2 " " $1 }' | awk -F% '($1 < 75.0 && $1 > 0.0) {print "Not enough coverage, expected 75.0%, found  " $0 }')
if [ "$coverFail" != "" ]; then
	echo "$coverage"
	echo "======================= Failed packages ====================="
	echo "$coverFail"
	exit 1
fi

echo "================== Coverage test SUCCESSFUL =============="

exit 0
