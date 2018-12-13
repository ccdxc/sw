#! /usr/local/bin/python3
import sys
import argparse
import os
import time
import yaml
import pdb

PASS_FILENAME="pass.count"
FAIL_FILENAME="fail.count"

def __read_count(filename):
    f = open(filename, 'r')
    count = int(f.read().split(':')[1])
    f.close()
    return count

def __sysctl(attr_fail, attr_success):
    os.system("sysctl %s > %s" % (attr_success, PASS_FILENAME))
    os.system("sysctl %s > %s" % (attr_fail, FAIL_FILENAME))
    return (__read_count(FAIL_FILENAME), __read_count(PASS_FILENAME))

def GetCounts():
    return __sysctl('compat.linuxkpi.pencake_fail_cnt',
                    'compat.linuxkpi.pencake_success_cnt')

def GetTestCount():
    with open(GlobalOptions.test) as f:
        testobj = yaml.load(f)
    f.close()
    return len(testobj['tests'])

parser = argparse.ArgumentParser(description='Pensando Storage Offload Test')
parser.add_argument('--cfg', nargs='+', dest='cfg', 
                    help='Config YML Files (1 or more - space separated)')
parser.add_argument('--test', dest='test', help='Test YML File')
parser.add_argument('--wait', dest='wait', default=1, type=int, help='Test Wait Time')
parser.add_argument('--failure-test', dest='failure_test',
                    action='store_true', help='Failure Test')
GlobalOptions = parser.parse_args()

old_failure_count, old_success_count = GetCounts()

ymlstring = ""
for cfgfile in GlobalOptions.cfg:
    ymlstring += "%s " % cfgfile
ymlstring += "%s " % GlobalOptions.test
os.system("cat %s > /dev/pencake" % ymlstring)
time.sleep(GlobalOptions.wait)
new_failure_count, new_success_count = GetCounts()

num_tests = GetTestCount()

print("PenCAKE Test Results:")
print(" - Failure Test      : %s" % str(GlobalOptions.failure_test))
print(" - Number of Tests   : %d" % num_tests)
print(" - Old Failure Count : %d" % old_failure_count)
print(" - Old Success Count : %d" % old_success_count)
print(" - New Failure Count : %d" % new_failure_count)
print(" - New Success Count : %d" % new_success_count)

exitcode = 0
if new_failure_count - old_failure_count != 0:
    if not GlobalOptions.failure_test:
        print("ERROR: New Failures in the Test") 
        exitcode = 1
    else:
        print("PenCAKE test PASSED with expected Failures.")
        exitcode = 0
elif new_success_count - old_success_count < num_tests:
    print("ERROR: NumSuccess < NumTests.") 
    exitcode = 1
elif GlobalOptions.failure_test:
    print("ERROR: Failure test passed. Unexpected.")
    exitcode = 1
else:
    print("PenCAKE test PASSED.")

sys.exit(exitcode)
