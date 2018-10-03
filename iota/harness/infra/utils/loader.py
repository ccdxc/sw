#! /usr/bin/python3
import pdb
import os
import importlib

from iota.harness.infra.utils.logger import Logger

import iota.harness.infra.types as types

def Import(modname, packages = []):
    for pkg in packages:
        imp_path = "%s.%s" % (pkg, modname)
        module = importlib.import_module(imp_path)
        if module:
            return module
    Logger.error("Failed to import module: %s in packages: " % modname, packages)
    assert(0)
    return None

def RunCallback(module, attr, required, args):
    cb = getattr(module, attr, None)
    if required:
        assert(cb)
    if not cb: return types.status.SUCCESS
    return cb(args)

'''
UNUSED
def CreateObjectInstance(pkg, modname, class_name):
    obj_class = GetModuleAttr(pkg, modname, class_name)
    return obj_class()
'''
