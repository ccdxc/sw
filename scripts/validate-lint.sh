#!/bin/bash

for pkg in $@
do
    echo "linting " $pkg
    lint_error=$(golint $pkg | grep -v "vendor\|scripts\|bi\|docs\|Godeps" | grep -v ".pb.go\|_mock.go" | tee /dev/stderr)

    if [ "$lint_error" != "" ]; then
    	exit 1
    fi
done

exit 0
