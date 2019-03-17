#!/bin/sh
for i in 1 2 3 4 5
    do
        wget -o status/download.$i -O results/result.$i http://srv13:8080/job/Hourly_Sanity_FreeBSD/$i/console
        echo "Results for Run: http://srv13:8080/job/Hourly_Sanity_FreeBSD/$i/console"
        echo "------------------"
        sed -n '/TestBundle Summary for TestSuite/,/Overall Run Summary/p' results/result.$i | awk '{print "bundle:"$1" failures:"$3" errors:"$5" result:"$9}' | grep -E "(Failure|Error)" | awk '{ printf "%-20s %10s %10s %10s\n", $1, $2, $3, $4 }'
        echo "------------------"
done
