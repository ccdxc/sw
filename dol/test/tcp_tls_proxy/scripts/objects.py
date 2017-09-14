#! /usr/bin/python3
import sys
import pdb
import random
import ipaddress
import copy

from collections import OrderedDict


class FrameworkObject(object):
    __readonly = False
    __locked = False

    def __init__(self):
        self.__gid = None
        return

    def ID(self, gid=None):
        return self.GID(gid)

    def GID(self, gid=None):
        if gid is not None:
            self.__gid = gid
        return self.__gid

    def __setattr__(self, key, val):
        if self.__readonly:
            raise TypeError("Cannot modify a readonly object.")
        if self.__locked and key not in self.__dict__:
            raise TypeError("Cannot add new attribute to class %s" % self)
        self.__dict__[key] = val

    def LockAttributes(self):
        self.__locked = True
        return

    def SetReadOnly(self):
        self.__readonly = True
        return

    def SetReadWrite(self):
        self.__readonly = False
        return

    def Clone(self, template):
        #assert(isinstance(template, FrameworkTemplateObject))
        old_gid = self.__gid
        self.__dict__.update(copy.deepcopy(template.__dict__))

        # Restore the original GID.
        self.__gid == old_gid
        self.template = template
        # self.LockAttributes()
        return

    def show(self):
        for k in self.__dict__:
            v = self.__dict__[k]
            if isinstance(v, FrameworkObject):
                v.show()
            elif isinstance(v, list):
                for e in v:
                    if isinstance(e, FrameworkObject):
                        e.show()
            elif isinstance(v, dict):
                for key, value in v.items():
                    if isinstance(value, FrameworkObject):
                        value.show()
        return


