#!/usr/bin/python
import subprocess
import sys
import yaml
if len(sys.argv) < 2:
    print("USAGE: {0} <base dir to search from>\n".format(sys.argv[0]))
    sys.exit(1)
baseDir = sys.argv[1]
ftsearch = ["*.yml","*.testcase","*.testbundle","*.testsuite"]
for name in ftsearch:
    files = subprocess.check_output('find {0} -type f -name "{1}"'.format(baseDir,name), shell=True)
    files = [f for f in files.split('\n') if len(f)>3]
    for f in files:
        print("checking yaml file {0}".format(f))
        yaml.safe_load(open(f,'r'))

