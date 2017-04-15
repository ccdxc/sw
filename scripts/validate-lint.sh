#!/bin/bash

lint_error=$(golint $1 | grep -v "vendor\|scripts\|bi\|docs\|Godeps" | grep -v ".pb.go\|_mock.go" | tee /dev/stderr)

if [ "$lint_error" != "" ]; then
	exit 1
fi

exit 0
