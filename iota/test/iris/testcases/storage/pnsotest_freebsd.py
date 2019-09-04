#! /usr/local/bin/python3
import sys
import argparse
import os
import time
import yaml
import pdb
import json

PASS_FILENAME="pass.count"
FAIL_FILENAME="fail.count"

class Dict2Object(object):
    def __init__(self, d):
        for a, b in d.items():
            if isinstance(b, (list, tuple)):
                setattr(self, a, [Dict2Object(x) if isinstance(x, dict) else x for x in b])
            else:
                setattr(self, a, Dict2Object(b) if isinstance(b, dict) else b)

class JsonObjectList(object):
    def __init__(self, speclist):
        self.objs = []
        for spec in speclist:
            obj = Dict2Object(spec)
            self.objs.append(obj)
        return
        
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
parser.add_argument('--timeout', dest='timeout', default=60, type=int, help='Test timeout')
parser.add_argument('--failure-test', dest='failure_test',
                    action='store_true', help='Failure Test')
parser.add_argument('--perf-test', dest='perf_test',
                    action='store_true', help='Performance Test')
GlobalOptions = parser.parse_args()
GlobalOptions.retry_wait = 0.2

old_failure_count, old_success_count = GetCounts()
os.system("dmesg -c > /dev/null")
ymlstring = ""
for cfgfile in GlobalOptions.cfg:
    ymlstring += "%s " % cfgfile
ymlstring += "%s " % GlobalOptions.test
os.system("cat %s > /dev/pencake" % ymlstring)
num_tests = GetTestCount()

for retry in range(int(GlobalOptions.timeout / 0.2)):
    time.sleep(GlobalOptions.retry_wait)
    new_failure_count, new_success_count = GetCounts()
    change_count = (new_failure_count - old_failure_count) + (new_success_count - old_success_count)
    if change_count >= num_tests: break

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

if GlobalOptions.perf_test:
    os.system("dmesg | grep -v \"PenCAKE Testcase\" | jq -s \'.\' > /tmp/result.json")
    with open('/tmp/result.json', 'r') as f:
        jsonlist = JsonObjectList(json.load(f))
    f.close()

    def __to_gbps(value):
        return (value * 8) / (1024 * 1024 * 1024)

    for tid in range(num_tests):
        obj = jsonlist.objs[tid]
        name = obj.tests[0].test.name
        input_gbps = __to_gbps(obj.tests[0].test.stats.in_bytes_per_sec) 
        max_input_gbps = __to_gbps(obj.tests[0].test.stats.max_in_bytes_per_sec) 
        output_gbps = __to_gbps(obj.tests[0].test.stats.out_bytes_per_sec) 
        max_output_gbps = __to_gbps(obj.tests[0].test.stats.max_out_bytes_per_sec)
        print("%s Input=%.02fGbps MaxInput=%.02fGbps Output=%.02fGbps MaxOutput=%.02fGbps" %\
              (name, input_gbps, max_input_gbps, output_gbps, max_output_gbps))

sys.exit(exitcode)
