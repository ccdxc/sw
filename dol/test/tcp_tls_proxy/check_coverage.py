#! /usr/bin/python
import os
import pdb
import glob

total = 0
print("%-50s %-10s %-10s %-10s %-20s" % ('Module', 'multiplier', 'count', 'tests', 'test-name'))
for filename in glob.glob('*.py'):
    if filename == 'check_coverage.py': continue 
    if filename == 'tcp_tls_proxy.py': continue
    if filename == 'tcp_proxy.py': continue
#for filename in os.listdir(os.getcwd()):
   # do your stuff
    module = []
    temp_filename = filename.replace(".py", "")
    with open('../../modules.list', 'r') as f:
         multiplier = 0
         for line in f.readlines():
             if 'name' in line:
                 name = line.split('name    : ')
             if temp_filename in line and 'module' in line: 
                 if temp_filename in line and 'pkt_to_serq_atomic_stats' in line and temp_filename != 'pkt_to_serq_atomic_stats':
                     continue
                 multiplier = multiplier + 1
                 module.append(name[1])
    count = 0
    with open(filename, 'r') as t:
        for line in t.readlines():
            if 'return False' in line:
                if '#' not in line:
                    count = count + 1
    print("%-50s %-10d %-10d %-10d %-20s" % (filename, multiplier, count, multiplier*count, "".join(module[0])))
    for item in module[1:]:
        print('%-83s %-20s' % ("",  str(item).translate(None, "[]'n\\")))
    total = total+multiplier*count
print("")
print("%-60s total tests %d" % ("",total))

