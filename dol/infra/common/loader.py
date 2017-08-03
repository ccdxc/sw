#! /usr/bin/python3
import pdb
import os
import importlib

def ImportModule(pkg, modname):
    imp_path = "%s.%s" % (pkg, modname)
    return importlib.import_module(imp_path)

def GetModuleAttr(pkg, modname, attr):
    mod = ImportModule(pkg, modname)
    return getattr(mod, attr)

def CreateObjectInstance(pkg, modname, class_name):
    obj_class = GetModuleAttr(pkg, modname, class_name)
    return obj_class()
