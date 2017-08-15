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

'''
AOL Type descriptor object support
'''
class SwDscrAolObject(base.ConfigObjectBase):
    def __init__(self, ringname, swdreidx):
        super().__init__()
        self.idx = swdreidx
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
        cfglogger.info("[%s]Received response for handle: %016d" % (self.ID(), self.DescAddr))
        cfglogger.info("A:%016d O:%08d L:%08d" % (self.Addr1, self.Offset1, self.Len1))
        cfglogger.info("A:%016d O:%08d L:%08d" % (self.Addr2, self.Offset2, self.Len2))
        cfglogger.info("A:%016d O:%08d L:%08d" % (self.Addr3, self.Offset3, self.Len3))
        return
    def GetHandle(self):
        return self.DescAddr

class SwDscrAolObjectHelper:
    def __init__(self):
        return
    def Generate(self, ringname, swdreidx):
        return SwDscrAolObject(ringname, swdreidx)
    def Configure(self, swdre_list):
        #halapi.GetDscrAolObjectState(swdre_list)
        return

'''
Page Type object support
'''
class SwDscrPageObject(base.ConfigObjectBase):
    def __init__(self, ringname, swdreidx):
        super().__init__()
        self.idx = swdreidx
        self.Clone(Store.templates.Get("PAGE_VIA_REF"))
        self.GID("%s_PAGE%04d" % (ringname, swdreidx))
        return
    def Show(self):
        cfglogger.info("SwPage: %s" % (self.ID()))
        return
    def PrepareHALRequestSpec(self, reqspec):
        # FIXME, query address of the page
        return
    def ProcessHALResponse(self, req_spec, resp_spec):
        return

class SwPageObjectHelper:
    def __init__(self):
        return
    def Generate(self, ringname, swdreidx):
        return SwDscrPageObject(ringname, swdreidx)
    def Configure(self, swdre_list):
        # TODO: integrate the HAL API
        #halapi.GetDscrAolObjectState(swdre_list)
        return

HelperDB = {}
HelperDB["DESCR_AOL_VIA_REF"] = SwDscrAolObjectHelper()
HelperDB["PAGE_VIA_REF"] = SwPageObjectHelper()


class SwDscrRingEntry(base.ConfigObjectBase):
    def __init__(self, ringname, entryidx, entrytype, ringidx = None):
        super().__init__()
        self.handle = None
        self.idx = entryidx
        self.type = entrytype
        if (ringidx != None):
            self.ringidx = ringidx
        else:
            self.ringidx = None
        self.GID("%s_ENTRY%04d" % (ringname, entryidx))
        return
    def Show(self):
        cfglogger.info("Entry : %s" % (self.ID()))
        return
    def PrepareHALRequestSpec(self, reqspec):
        if (self.ringidx != None):
            reqspec.key_or_handle.wring_id = self.ringidx
        reqspec.type = self.type
        reqspec.index = self.idx
        return
    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("Entry : %s : RI: %d T: %d I:%d" % (self.ID(), resp_spec.spec.key_or_handle.wring_id, resp_spec.spec.type, resp_spec.index))
        if (resp_spec.spec.type != req_spec.type):
            assert(0)
        if (resp_spec.index != req_spec.index):
            assert(0)

        self.handle = resp_spec.value
        cfglogger.info("Entry : %s : Handle: %016x" % (self.ID(), self.handle))
        return


class SwDscrRingObject(base.ConfigObjectBase):
    def __init__(self, ring_def, parent = None, ringidx = None):
        super().__init__()
        self.Clone(Store.templates.Get("RING"))
        self.swdr_name = ring_def.name
        self.haltype = ring_def.haltype
        self.swdr_type = ring_def.type
        self.count = ring_def.count 
        self.ringentries = []
        self.swdre_list = []
        if (parent != None):
            self.GID("%s_%s" % (parent.upper(), self.swdr_name))
        else:
            self.GID("%s" % (self.swdr_name))

        if (ringidx != None):
            self.ringidx = ringidx
        else:
            self.ringidx = None

        return

    def Init(self):
        for swdreidx in range(self.count):
            # Generic ring entry information
            ringentry = SwDscrRingEntry(self.ID(), swdreidx, self.haltype, self.ringidx)
            self.ringentries.append(ringentry)
            Store.objects.Add(ringentry)

            # Ring entry specific information
            RingEntryHelper = HelperDB[self.swdr_type]
            swdre = RingEntryHelper.Generate(self.ID(), swdreidx)
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
        # Configure generic ring entries
        halapi.GetRingEntries(self.ringentries)

        # Configure entry specific information
        RingEntryHelper = HelperDB[self.swdr_type]
        RingEntryHelper.Configure(self.swdre_list)
        return


class SwDscrRingObjectHelper:
    def __init__(self):
        self.swdr_list = []
        return

    def Configure(self):
        for swdr in self.swdr_list:
            swdr.Configure()
        return

    def Generate(self, type, parent = None, ringidx = None):
        spec = Store.specs.Get(type)
        for swdrt in spec.entries:
            swdr = SwDscrRingObject(swdrt.entry, parent, ringidx)
            swdr.Init()
            self.swdr_list.append(swdr)
            Store.objects.Add(swdr)
        return

    def Show(self):
        for swdr in self.swdr_list:
            swdr.Show()
        return

    def main(self, type, parent = None, ringidx = None):
        self.Generate(type, parent, ringidx)
        self.Configure()
        self.Show()


SwDscrRingHelper = SwDscrRingObjectHelper()
