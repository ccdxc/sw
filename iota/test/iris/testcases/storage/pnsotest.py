#! /usr/bin/python3
import sys
import argparse
import os
import time
import yaml

def __read_count(filename):
    with open(filename) as f:
        count = int(f.readline())
    f.close()
    return count

def GetFailureCount():
    return __read_count('/sys/module/pencake/status/fail')

def GetSuccessCount():
    return __read_count('/sys/module/pencake/status/success')

def GetTestCount():
    with open(GlobalOptions.test) as f:
        testobj = yaml.load(f)
    f.close()
    return len(testobj['tests'])

parser = argparse.ArgumentParser(description='Pensando Storage Offload Test')
parser.add_argument('--cfg', nargs='+', dest='cfg', 
                    help='Config YML Files (1 or more - space separated)')
parser.add_argument('--test', dest='test', help='Test YML File')
parser.add_argument('--wait', dest='wait', type=int, help='Test Wait Time')
parser.add_argument('--failure-test', dest='failure_test',
                    action='store_true', help='Failure Test')
GlobalOptions = parser.parse_args()

old_success_count = GetSuccessCount()
old_failure_count = GetFailureCount()

num_cfgs = len(GlobalOptions.cfg)
for cfgidx in range(num_cfgs):
    os.system("cp -v %s /sys/module/pencake/cfg/%d" % (GlobalOptions.cfg[cfgidx], cfgidx))
os.system("cp -v %s /sys/module/pencake/cfg/%d" % (GlobalOptions.test, num_cfgs + 1))
os.system("echo start > /sys/module/pencake/cfg/ctl")

time.sleep(GlobalOptions.wait)
new_success_count = GetSuccessCount()
new_failure_count = GetFailureCount()

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
elif new_success_count - old_success_count != num_tests:
    print("ERROR: NumSuccess != NumTests.") 
    exitcode = 1
elif GlobalOptions.failure_test:
    print("ERROR: Failure test passed. Unexpected.")
    exitcode = 1
else:
    print("PenCAKE test PASSED.")

sys.exit(exitcode)
