#! /usr/bin/python3
import pdb
import copy
import inspect

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.common.utils       as utils

from infra.common.glopts   import GlobalOptions as GlobalOptions
from infra.common.logging  import logger as logger

def ToJsonCovereter(obj):
    dict = {}
    for key, value in inspect.getmembers(obj):
        if  key.startswith("_") or inspect.ismethod(value) or inspect.isfunction(value):
            continue
        if  (type(value) is int or type(value) is str or type(value) is bool):
            dict[key] = value
        elif (type(value).__str__ is not object.__str__):
            dict[key] = str(value)
        elif (type(value) in [list,set]):
            items = []
            for item in value:
                items.append(str(item))
            dict[key] = items
        else:
            dict[key] = ToJsonCovereter(value)
    return dict

class StatusObjectBase(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        return

class ConfigObjectBase(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        return

    def __str__(self):
        return str(self.ID())


    def ToJson(self):
        return ToJsonCovereter(self)
        #Ignoring private attributes and complex objects for now.
        #Function has to be enhanced to support deeper coversions.
        #dict = {}
        #for key, value in inspect.getmembers(self):

    def IsFilterMatch(self, filters):
        logger.verbose("IsFilterMatch(): Object %s" % self.GID())
        if filters == None:
            return True
        for f in filters:
            attr = f[0]
            value = f[1]
            if attr == 'any' and value == None:
                continue
            if attr not in self.__dict__:
                logger.error("Attr:%s not present in %s." %\
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
            logger.verbose("  - %s: object" % attr, fvalue,
                              "filter:", value)
            if fvalue != value:
                return False
        logger.verbose("  - Found Match !!")
        return True

    def Equals(self, other):
        logger.error("Method %s not implemented by class: %s" %
                     (utils.GetFunctionName(), self.__class__))
        assert(0)
        return

    def Copy(self):
        logger.error("Method %s not implemented by class: %s" %
                     (utils.GetFunctionName(), self.__class__))
        assert(0)
        return

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

    def SetupTestcaseConfig(self, obj):
        obj.root = self
        return

    def TearDownTestcaseConfig(self, obj):
        return

    def ShowTestcaseConfig(self, obj):
        logger.info("%s Config object :  %s" % (type(self).__name__, self.GID()))
        return

    def CompareObjectFields(self, other, fields, lgh):
        return utils.CompareObjectFields(self, other, fields, lgh)

    def InFeatureSet(self):
        return GlobalOptions.feature_set in self.meta.feature_set

    def IsRetryEnabled(self):
        return False

class AgentObjectMeta:
    def __init__(self):
        self.Name = None
        self.Tenant = None
        self.Namespace = None
        return

class AgentObjectBase:
    def __init__(self, kind, name, ten):
        self.Kind = kind
        self.meta = AgentObjectMeta()
        self.meta.Name = name
        self.meta.Tenant = ten
        self.meta.Namespace = "default"
        return
