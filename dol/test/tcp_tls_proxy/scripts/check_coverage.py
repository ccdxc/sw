#! /usr/bin/python3
import os
import pdb
import glob
import sys

total = 0

paths = [
    '/dol',
    '/nic/gen/protobuf/',
    '/dol/third_party/',
    '/nic'
]
ws_top = os.path.dirname(sys.argv[0]) + '/../../../../'
ws_top = os.path.abspath(ws_top)
sys.path.insert(0, ws_top)
os.environ['WS_TOP'] = ws_top
os.environ['MODEL_SOCK_PATH'] = ws_top + '/nic/'
for path in paths:
    fullpath = ws_top + path
    print("Adding Path: %s" % fullpath)
    sys.path.insert(0, fullpath)

import infra.engine.modmgr as modmgr
ModuleDatabaseObj = modmgr.ModuleDatabase('modules.list')

ProxyModulesDB = {}

for filename in glob.glob('../*.py'):
    count = 0
    with open(filename, 'r') as t:
        for line in t.readlines():
            if 'return False' in line:
                if '#' not in line:
                    count = count + 1
    filename = filename.replace(".py", "")
    filename = filename.replace("../", "")
    ProxyModulesDB[filename] = count

class Result:
    def __init__(self, module):
        self.namelist   = []
        self.module     = module
        self.multiplier = 0
        self.count = 0
        self.total = 0
        return

ResultDB = {} 
module = ModuleDatabaseObj.getfirst()
while module != None:
    if module.parsedata.enable == True and module.parsedata.ignore == False:
        if module.parsedata.module in ProxyModulesDB.keys():
            insert = False
            if module.parsedata.module in ResultDB.keys():
                obj = ResultDB[module.parsedata.module]
            else:
                obj = Result(module.parsedata.module)
                insert = True
            obj.namelist.append(module.parsedata.name)
            obj.count = ProxyModulesDB[module.parsedata.module]
            obj.multiplier = obj.multiplier+1
            obj.total = obj.count * obj.multiplier
            if insert:
                ResultDB[module.parsedata.module] = obj         
    module = ModuleDatabaseObj.getnext()

print("%-40s %-10s %-10s %-10s %-20s" % ('Module', 'multiplier', 'count', 'tests', 'test-name'))
for key, value in ResultDB.items():
    total = total+value.total
    print("%-40s %-10d %-10d %-10d %-20s" % (value.module, value.multiplier, value.count, value.total, "".join(value.namelist[0])))
    for item in value.namelist[1:]:
        print('%-73s %-20s' % ("",  item))
print("")
print("%-50s total tests %d" % ("",total))

