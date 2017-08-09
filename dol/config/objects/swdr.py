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

class NMDObject(base.ConfigObjectBase):
    def __init__(self, ringname, nmdidx):
        super().__init__()
        self.Clone(Store.templates.Get('NMD'))
        self.GID("%s_DESC%04d" % (ringname, nmdidx))
        # TODO: Populate the descriptor state from HAL
        return

class NMDREntryObject(base.ConfigObjectBase):
    def __init__(self, ringname, nmdidx):
        super().__init__()
        self.Clone(Store.templates.Get('NMDR_ENTRY'))
        self.GID("%s_ENTRY%04d" % (ringname, nmdidx))
        self.nmd = NMDObject(ringname, nmdidx)
        Store.objects.Add(self.nmd)
        # TODO: Populate the entry state from HAL
        return

    def Show(self):
        cfglogger.info("NMDREntry: %s NMDObject: %s" % (self.ID(), self.nmd.ID()))
        return

class NMDRObject(base.ConfigObjectBase):
    def __init__(self, nmdr_type, count):
        super().__init__()
        self.Clone(Store.templates.Get('NMDR'))
        self.nmdr_type = nmdr_type
        self.count = count
        self.nmde_list = []
        self.GID("%01sNMDR" % self.nmdr_type)
        return

    def Init(self):
        for nmdidx in range(self.count):
            nmde = NMDREntryObject(self.ID(), nmdidx)
            self.nmde_list.append(nmde)
        Store.objects.SetAll(self.nmde_list)
        return

    def Show(self):
        cfglogger.info("NMD List for %s" % self.ID())
        for nmde in self.nmde_list:
            nmde.Show()
        return


class NMDRObjectHelper:
    def __init__(self):
        self.nmdr_list = []
        return

    def Configure(self):
        cfglogger.info("Configuring NMDR.")
        return

    def Generate(self):
        spec = Store.specs.Get('NMDR')
        for nmdrt in spec.types:
            nmdr = NMDRObject(nmdrt.entry.type, nmdrt.entry.count)
            nmdr.Init()
            self.nmdr_list.append(nmdr)
        Store.objects.SetAll(self.nmdr_list)
        return

    def Show(self):
        for nmdr in self.nmdr_list:
            nmdr.Show()
        return

    def main(self):
        self.Generate()
        self.Configure()
        self.Show()


NMDRHelper = NMDRObjectHelper()
