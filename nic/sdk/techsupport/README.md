# TechSupport
TechSupport binary is used by sysmgr for collecting system wide techsupport.
It runs the list of commands supplied in json file,
compresses the output on the fly & writes it to the specified location.

## How to run

### On x86

 /sw/nic/build/x86_64/apulu/bin/techsupport -c /sw/nic/conf/apulu/techsupport.json -d /sw/nic/ -o techsupport.gz

### On HW

 /nic/bin/techsupport.bin -c /nic/conf/techsupport.json -d /data/ -o techsupport.gz

