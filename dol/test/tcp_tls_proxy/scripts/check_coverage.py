#! /usr/bin/python3
import os
import pdb
import glob
import ruamel.yaml as yaml
import collections
import objects as objects

total = 0

class ModuleListParser:
    def __init__(self, module_list_file):
        self.module_list_file = module_list_file
        self.parsed_module_list = []
        self.parse()
        return

    def __parse(self, ydata, depth):
        if isinstance(ydata, dict):
            return self.__parse_dict(ydata, depth)
        elif isinstance(ydata, list):
            return self.__parse_list(ydata, depth)
        elif ydata == 'None':
            return None
        return ydata

    def __parse_dict(self, ydata, depth):
        obj = objects.FrameworkObject()
        for key in ydata:
            obj.__dict__[key] = self.__parse(ydata[key], depth + 1)
        return obj

    # List of objects or List of values.
    def __parse_list(self, ydata, depth):
        objlist = []
        for elem in ydata:
            obj = self.__parse(elem, depth + 1)
            objlist.append(obj)
        return objlist

    def parse(self):
        with open(self.module_list_file, 'r') as f:
            ydata = yaml.load(f, Loader=yaml.RoundTripLoader)
            genobj = self.__parse(ydata, 0)

        for elem in genobj.modules:
            self.parsed_module_list.append(elem.module)
            elem.show()
        return


class Module:
    def __init__(self, parsedata):
        self.parsedata = parsedata

        self.name       = parsedata.name
        self.package    = parsedata.package
        self.module     = parsedata.module
        self.spec       = parsedata.spec
        self.path       = self.package.replace(".", "/")
        self.ignore     = parsedata.ignore
        return

class ModuleDatabase:
    def __init__(self, module_list_file):
        self.db = []
        self.parser = ModuleListParser(module_list_file)
        self.__add_all()
        return

    def __is_test_match(self, name):
        return True

    def __is_module_match(self, module):
        return True

    def __add(self, pmod):
        pmod.enable = True

        if pmod.enable == False:
            return

        if 'ignore' not in pmod.__dict__:
            pmod.ignore = False

        if 'iterate' not in pmod.__dict__:
            pmod.iterate = [ None ]

        if not self.__is_test_match(pmod.name):
            return

        if not self.__is_module_match(pmod.module):
            return

        module = Module(pmod)
        self.db.append(module)
        return

    def __add_all(self):
        for pmod in self.parser.parsed_module_list:
            self.__add(pmod)
        return

    def getnext(self):
        if self.iter == len(self.db):
            return None
        mod = self.db[self.iter]
        self.iter += 1
        return mod

    def getfirst(self):
        self.iter = 0
        return self.getnext()

ModuleDatabaseObj = ModuleDatabase('../../../modules.list')

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

