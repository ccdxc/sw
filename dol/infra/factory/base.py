#! /usr/bin/python3
import pdb

import infra.common.defs as defs
import infra.common.objects as objects
import infra.common.utils as utils

class FactoryObjectBase(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        self.rbytes     = [] # Raw bytes of this object after bin packing.
        self.address    = None
        self.logger     = None
        self.spec       = None
        return

    def Logger(self, logger = None):
        if logger:
            self.logger = logger
        return self.logger

    def Address(self, address = None):
        if address:
            self.address = address
        return self.address

    def __str__(self):
        return str(self.GID())

    def Init(self, spec):
        self.spec = spec
        return

    def CopyConstructor(self, obj):
        return

    def Copy(self):
        cpobj = self.__class__()
        cpobj.CopyConstructor(self)
        return cpobj

    # Returns an object of same type after reading from HW.
    def Read(self, mkcopy = True):
        if mkcopy:
            read_obj = self.Copy()
            read_obj.Read(mkcopy = False)
            return read_obj
        
        # Call the model_wrap mem_read API
        return self

    def Write(self):
        # Call the model_wrap mem_write API
        return

