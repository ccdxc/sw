#! /usr/bin/python3
import pdb
import copy

import infra.common.defs as defs
import infra.common.objects as objects
import infra.common.utils as utils


class FactoryObjectBase(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        return

    def SetLogger(self, logger):
        self.logger = logger
        return

    def __str__(self):
        return str(self.GID())

    def Init(self):
        self.logger.error("Method %s not implemented by class: %s" %
                          (utils.GetFunctionName(), self.__class__))
        assert(0)
        return

    def Read(self):
        self.logger.error("Method %s not implemented by class: %s" %
                          (utils.GetFunctionName(), self.__class__))
        assert(0)
        return

    def Write(self):
        self.logger.error("Method %s not implemented by class: %s" %
                          (utils.GetFunctionName(), self.__class__))
        assert(0)
        return
