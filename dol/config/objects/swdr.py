#! /usr/bin/python3

import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr

import config.hal.defs          as haldefs
import config.hal.api           as halapi

from config.store               import Store
from infra.common.logging       import cfglogger

class SwDscrObject(base.ConfigObjectBase):
    def __init__(self, ringname, swdreidx, parent):
        super().__init__()
        self.Clone(Store.templates.Get("DESCR_AOL"))
        self.GID("%s_DESC%04d" % (ringname, swdreidx))
        # TODO: Populate the descriptor state from HAL
        return

class SwDscrRingEntryObject(base.ConfigObjectBase):
    def __init__(self, ringname, swdreidx, type, parent):
        super().__init__()
        self.Clone(Store.templates.Get("%s_ENTRY" % type))
        self.GID("%s_ENTRY%04d" % (ringname, swdreidx))
        self.swdscr = SwDscrObject(ringname, swdreidx, parent)
        Store.objects.Add(self.swdscr)
        # TODO: Populate the entry state from HAL
        return

    def Show(self):
        cfglogger.info("SwDscrRingEntry: %s SwDscrObject: %s" % (self.ID(), self.swdscr.ID()))
        return

class SwDscrRingObject(base.ConfigObjectBase):
    def __init__(self, swdr_type, count, parent):
        super().__init__()
        self.Clone(Store.templates.Get(swdr_type))
        self.swdr_type = swdr_type
        self.count = count
        self.swdre_list = []
        self.GID("%s_%s_SWDR" % (parent.upper(), self.swdr_type))
        return

    def Init(self, parent):
        for swdreidx in range(self.count):
            swdre = SwDscrRingEntryObject(self.ID(), swdreidx, self.swdr_type, parent)
            self.swdre_list.append(swdre)
            Store.objects.Add(swdre)
        return

    def Show(self):
        cfglogger.info("SWDSCR List for %s" % self.ID())
        for swdre in self.swdre_list:
            swdre.Show()
        return


class SwDscrRingObjectHelper:
    def __init__(self):
        self.swdr_list = []
        return

    def Configure(self, type):
        cfglogger.info("Configuring %s." % type)
        return

    def Generate(self, type, parent):
        spec = Store.specs.Get(type)
        for swdrt in spec.entries:
            swdr = SwDscrRingObject(swdrt.entry.type, swdrt.entry.count, parent)
            swdr.Init(parent)
            self.swdr_list.append(swdr)
            Store.objects.Add(swdr)
        return

    def Show(self):
        for swdr in self.swdr_list:
            swdr.Show()
        return

    def main(self, type, parent):
        self.Generate(type, parent)
        self.Show()


SwDscrRingHelper = SwDscrRingObjectHelper()
