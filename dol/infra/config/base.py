#! /usr/bin/python3
import pdb
import copy

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.common.utils       as utils

from infra.common.logging  import cfglogger as cfglogger

class ConfigObjectBase(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        return

    def __str__(self):
        return self.ID()

    def IsFilterMatch(self, filters):
        cfglogger.verbose("IsFilterMatch(): Object %s" % self.GID())
        for f in filters:
            attr = f[0]
            value = f[1]
            if attr == 'any' and value == None:
                continue
            if attr not in self.__dict__:
                cfglogger.error("Attr:%s not present in %s." %\
                                (attr, self.__class__))
                assert(0)
                return False

            fvalue = self.__dict__[attr]
            if isinstance(fvalue, objects.FrameworkFieldObject):
                fvalue = fvalue.get()
            
            if value.isdigit(): value = int(value)
            if value == 'None': value = None
            if value == 'True': value = True
            if value == 'False': value = False
            cfglogger.verbose("  - %s: object" % attr, fvalue,
                              "filter:", value)
            if fvalue != value:
                return False
        cfglogger.verbose("  - Found Match !!")
        return True

    def PrepareHALRequestSpec(self, reqspec):
        logger.error("Method %s not implemented by class: %s" %\
                     (utils.GetFunctionName(), self.__class__))
        assert(0)
        return

    def ProcessHALResponse(self, msgspec, response_spec):
        logger.error("Method %s not implemented by class: %s" %\
                     (utils.GetFunctionName(), self.__class__))
        assert(0)
        return

    # Methods for RING objects.
    def Post(self):
        logger.error("Method %s not implemented by class: %s" %\
                     (utils.GetFunctionName(), self.__class__))
        assert(0)
        return

    def Init(self):
        logger.error("Method %s not implemented by class: %s" %\
                     (utils.GetFunctionName(), self.__class__))
        assert(0)
        return

    def Write(self):
        logger.error("Method %s not implemented by class: %s" %\
                     (utils.GetFunctionName(), self.__class__))
        assert(0)
        return
        
    def Read(self):
        logger.error("Method %s not implemented by class: %s" %\
                     (utils.GetFunctionName(), self.__class__))
        assert(0)
        return
