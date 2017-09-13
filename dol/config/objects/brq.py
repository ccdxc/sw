
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr

import config.hal.defs          as haldefs
import config.hal.api           as halapi

import types_pb2            as types_pb2

from config.store               import Store
from infra.common.logging       import cfglogger

import wring_pb2            as wring_pb2
import wring_pb2_grpc           as wring_pb2_grpc

class BRQEntryObject(base.ConfigObjectBase):
    def __init__(self, brq_ring_name, brq_ring_type, brq_entry_defn, entry_idx):
        super().__init__()
        self.Clone(Store.templates.Get(brq_entry_defn))
        self.ring_type = brq_ring_type
        self.entry_idx = entry_idx
        self.GID("%s_ENTRY%04d" % (brq_ring_name, entry_idx))
        return
    def Sync(self):
        ring_entries = []
        ring_entries.append(self) 
        halapi.GetRingEntries(ring_entries)
        return
    def Configure(self):
        self.Sync()
        return
    def PrepareHALRequestSpec(self, reqspec):
        reqspec.type = self.ring_type
        reqspec.index = self.entry_idx
        #cfglogger.info("PrepareHALRequestSpec Entry: %s, type: %d, index: %d" % (self.ID() , reqspec.type , reqspec.index ))
        return
    def ProcessHALResponse(self, req_spec, resp_spec):
        #cfglogger.info("Entry : %s : RI: %d T: %d I:%d" % (self.ID(), resp_spec.spec.key_or_handle.wring_id, resp_spec.spec.type, resp_spec.index))
        if (resp_spec.spec.type != req_spec.type):
            assert(0)
        if (resp_spec.index != req_spec.index):
            assert(0)
        #cfglogger.info("Field set %s" % resp_spec.WhichOneof("WRingSlotInfo"))
            
        if (resp_spec.HasField("barco_gcm_desc")):
            self.ilist_addr = resp_spec.barco_gcm_desc.ilist_addr
            self.olist_addr = resp_spec.barco_gcm_desc.olist_addr
            self.command = resp_spec.barco_gcm_desc.command
            self.key_desc_index = resp_spec.barco_gcm_desc.key_desc_index
            self.iv_addr = resp_spec.barco_gcm_desc.iv_addr
            self.status_addr = resp_spec.barco_gcm_desc.status_addr
            self.doorbell_addr = resp_spec.barco_gcm_desc.doorbell_addr
            self.doorbell_data = resp_spec.barco_gcm_desc.doorbell_data
            #cfglogger.info("Entry(%s): ila: %x. ola: %x, cmd: %x, kdi: %x, ia: %x, sa: %x, da: %x, dd: %x" %
            #                (self.ID(), self.ilist_addr, self.olist_addr, self.command, self.key_desc_index,
            #                self.iv_addr, self.status_addr, self.doorbell_addr, self.doorbell_data))
        else:
            assert(0)
        return
    def Show(self):
        cfglogger.info("Entry: %s" % self.ID())
        return



class BRQObject(base.ConfigObjectBase):
    def __init__(self, brq_instance):
        super().__init__()
        self.Clone(Store.templates.Get(brq_instance.defn))
        self.GID("%s" % brq_instance.name)
        self.ring_entries = []
        self.count = brq_instance.count
        self.type = brq_instance.type
        for ring_entry_idx in range(brq_instance.count):
            ring_entry = BRQEntryObject(self.ID(), brq_instance.type, brq_instance.entry_defn, ring_entry_idx)
            self.ring_entries.append(ring_entry)
        Store.objects.SetAll(self.ring_entries)
        return
    def Sync(self):
        rings = []
        rings.append(self)
        halapi.GetRingMeta(rings)
        halapi.GetRingEntries(self.ring_entries)
        return
    def Configure(self):
        self.Sync()
        return
    def Show(self):
        cfglogger.info("Ring: %s" % (self.ID()))
        for ring_entry in self.ring_entries:
            ring_entry.Show()
        return
    def PrepareHALRequestSpec(self, reqspec):
        reqspec.type = self.type
        return
    def ProcessHALResponse(self, req_spec, resp_spec):
        #cfglogger.info("Entry : %s : RI: %d T: %d I:%d" % (self.ID(), resp_spec.spec.key_or_handle.wring_id, resp_spec.spec.type, resp_spec.index))
        self.pi = resp_spec.pi
        self.ci = resp_spec.ci
        #cfglogger.info("Entry : %s : pi %s ci %s" % (self.ID(), self.pi, self.ci))
        return
        



class BRQObjectHelper:
    def __init__(self):
        self.brq_list = []
        return

    def Sync(self):
        for brq in self.brq_list:
            brq.Sync()
        return

    def Generate(self):
        spec = Store.specs.Get("BRQ_DEF")
        for brq_instance in spec.entries:
            brq = BRQObject(brq_instance.entry)
            self.brq_list.append(brq)
        Store.objects.SetAll(self.brq_list)
        return

    def Show(self):
        for brq in self.brq_list:
            brq.Show()
        return

    def main(self):
        self.Generate()
        self.Sync()
        self.Show()


BRQHelper = BRQObjectHelper()
