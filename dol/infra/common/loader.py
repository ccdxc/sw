#! /usr/bin/python3
import pdb
import os
import importlib

from infra.common.glopts import GlobalOptions

def ImportModule(pkg, modname, pipeline = None):
    imp_path = "%s.%s" % ( pkg, modname)
    if pipeline:
        imp_path = pipeline + "." + imp_path
    return importlib.import_module(imp_path)

def GetModuleAttr(pkg, modname, attr):
    mod = ImportModule(pkg, modname)
    return getattr(mod, attr)

def CreateObjectInstance(pkg, modname, class_name):
    obj_class = GetModuleAttr(pkg, modname, class_name)
    return obj_class()
