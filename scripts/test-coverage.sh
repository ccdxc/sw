#!/bin/bash

coverage=$(go test -cover -tags test $@ | grep "coverage:" | awk -Fcoverage: '{print $2 " " $1 }' | awk -F% '($1 < 75.0 && $1 > 0.0) {print "Not enough coverage, expected 75.0%, found  " $0 }')
if [ "$coverage" != "" ]; then
	echo $coverage
	exit 1
fi

exit 0
