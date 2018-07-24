#!/bin/bash
A=$(git status  --porcelain=v1 --ignore-submodules=all | egrep -v '^[MAD] ' | grep -v gitmodules)
if [ -n "$A" ] ; then
    echo locally modified files are $A
    echo *** Failing compilation ***
    echo Local changes are
    git diff --ignore-submodules=all
	exit 1
fi
echo No uncomitted locally modified files
exit 0
