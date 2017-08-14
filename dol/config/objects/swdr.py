#! /usr/bin

import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr

import config.hal.defs          as haldefs
import config.hal.api           as halapi

from config.store               import Store
from infra.common.logging       import cfglogger


class SwDscrAolObject(base.ConfigObjectBase):
    def __init__(self, ringname, swdreidx, parent = None):
        super().__init__()
        self.Clone(Store.templates.Get("DESCR_AOL_VIA_REF"))
        self.GID("%s_DESC%04d" % (ringname, swdreidx))
        return
    def Show(self):
        cfglogger.info("SwDscrAol: %s" % (self.ID()))
        return
    def PrepareHALRequestSpec(self, reqspec):
        # FIXME, this should really be an index into the ring
        reqspec.descr_aol_handle = 543210
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.DescAddr   = resp_spec.descr_aol_handle
        self.Addr1      = resp_spec.Address1
        self.Offset1    = resp_spec.Offset1
        self.Len1       = resp_spec.Length1
        self.Addr2      = resp_spec.Address2
        self.Offset2    = resp_spec.Offset2
        self.Len2       = resp_spec.Length2
        self.Addr3      = resp_spec.Address3
        self.Offset3    = resp_spec.Offset3
        self.Len3       = resp_spec.Length3
        cfglogger.info("Received response for handle: %016d" % self.DescAddr)
        cfglogger.info("A:%016d O:%08d L:%08d" % (self.Addr1, self.Offset1, self.Len1))
        cfglogger.info("A:%016d O:%08d L:%08d" % (self.Addr2, self.Offset2, self.Len2))
        cfglogger.info("A:%016d O:%08d L:%08d" % (self.Addr3, self.Offset3, self.Len3))
        return


class SwDscrAolObjectHelper:
    def __init__(self):
        return
    def Generate(self, ringname, swdreidx, parent = None):
        return SwDscrAolObject(ringname, swdreidx, parent)

HelperDB = {}
HelperDB["DESCR_AOL_VIA_REF"] = SwDscrAolObjectHelper()

class SwDscrRingEntryObject(base.ConfigObjectBase):
    def __init__(self, ringname, swdreidx, type, parent):
        super().__init__()
        self.Clone(Store.templates.Get(type))
        self.index = swdreidx
        self.GID("%s_ENTRY%04d" % (ringname, swdreidx))
        # TODO: Populate the entry state from HAL into handle
        self.handle = 0
        SwDscrObjectHelper = HelperDB[type]
        self.swdscr = SwDscrObjectHelper.Generate(ringname, swdreidx, parent)
        Store.objects.Add(self.swdscr)
        return

    def Show(self):
        cfglogger.info("SwDscrRingEntry: %s SwDscrObject: %s" % (self.ID(), self.swdscr.ID()))
        return

    def GetRingEntryObjectHandle(self):
        return self.handle

    def GetDscrObject(self):
        return self.swdscr

class SwDscrRingObject(base.ConfigObjectBase):
    def __init__(self, ring_def, parent = None):
        super().__init__()
        self.Clone(Store.templates.Get("RING"))
        self.swdr_name = ring_def.name
        self.swdr_type = ring_def.type
        self.count = ring_def.count 
        self.swdre_list = []
        self.GID("%s_%s_SWDR" % (parent.upper(), self.swdr_type))
        return

    def Init(self, parent):
        for swdreidx in range(self.count):
            RingEntryHelper = HelperDB[self.swdr_type]
            swdre = RingEntryHelper.Generate(self.ID(), swdreidx, parent)
            self.swdre_list.append(swdre)
            Store.objects.Add(swdre)
        return

    def Show(self):
        cfglogger.info("SWDSCR List for %s" % self.ID())
        for swdre in self.swdre_list:
            swdre.Show()
        return

    def GetEntry(self, index):
        if (index < 0) or (index >= self.count):
            assert(0)
        return self.swdre_list[index]

    def Configure(self):
        halapi.GetDscrAolObjectState(self.swdre_list)


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
            swdr = SwDscrRingObject(swdrt.entry, parent)
            swdr.Init(parent)
            swdr.Configure()
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
