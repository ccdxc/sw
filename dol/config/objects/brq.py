
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr

import config.hal.defs          as haldefs
import config.hal.api           as halapi

import types_pb2            as types_pb2

from config.store               import Store
from infra.common.logging       import logger

import wring_pb2            as wring_pb2
#import wring_pb2_grpc           as wring_pb2_grpc

class BRQEntryObject(base.ConfigObjectBase):
    def __init__(self, brq_ring_name, brq_ring_type, brq_entry_defn, entry_idx):
        super().__init__()
        self.Clone(Store.templates.Get(brq_entry_defn))
        self.ring_type = brq_ring_type
        self.entry_idx = entry_idx
        self.entry_defn = brq_entry_defn
        self.GID("%s_ENTRY%04d" % (brq_ring_name, entry_idx))
        return
    def Sync(self):
        ring_entries = []
        ring_entries.append(self)
        if (self.entry_defn == "BRQ_GCM_ENTRY"):
            halapi.GetRingEntries(ring_entries)
        else:
            halapi.GetBarcoRingEntries(ring_entries)
        return
    def Configure(self):
        self.Sync()
        return
    def PrepareHALRequestSpec(self, reqspec):
        if (self.entry_defn == "BRQ_GCM_ENTRY"):
            reqspec.type = self.ring_type
            reqspec.index = self.entry_idx
            #logger.info("PrepareHALRequestSpec Entry: %s, type: %d, index: %d" % (self.ID() , reqspec.type , reqspec.index ))
        else:
            reqspec.ring_type = self.ring_type
            reqspec.slot_index = self.entry_idx
            #logger.info("PrepareHALRequestSpec Entry: %s, type: %d, index: %d" % (self.ID() , reqspec.ring_type , reqspec.slot_index ))
        return
    def ProcessHALResponse(self, req_spec, resp_spec):
        if (self.entry_defn == "BRQ_GCM_ENTRY"):
            #logger.info("Entry : %s : RI: %d T: %d I:%d" % (self.ID(), resp_spec.spec.key_or_handle.wring_id, resp_spec.spec.type, resp_spec.index))
            if (resp_spec.spec.type != req_spec.type):
                assert(0)
                if (resp_spec.index != req_spec.index):
                    assert(0)
                    logger.info("Field set %s" % resp_spec.WhichOneof("WRingSlotInfo"))

            if (resp_spec.HasField("barco_gcm_desc")):
                self.ilist_addr = resp_spec.barco_gcm_desc.ilist_addr
                self.olist_addr = resp_spec.barco_gcm_desc.olist_addr
                self.command = resp_spec.barco_gcm_desc.command
                self.key_desc_index = resp_spec.barco_gcm_desc.key_desc_index
                self.iv_addr = resp_spec.barco_gcm_desc.iv_addr
                self.status_addr = resp_spec.barco_gcm_desc.status_addr
                self.doorbell_addr = resp_spec.barco_gcm_desc.doorbell_addr
                self.doorbell_data = resp_spec.barco_gcm_desc.doorbell_data
                self.salt = resp_spec.barco_gcm_desc.salt
                self.explicit_iv = resp_spec.barco_gcm_desc.explicit_iv
                self.barco_status = resp_spec.barco_gcm_desc.barco_status
                self.header_size = resp_spec.barco_gcm_desc.header_size
                logger.info("Entry(%s): ila: %x. ola: %x, cmd: %x, kdi: %x, ia: %x, sa: %x, da: %x, dd: %x" %
                                (self.ID(), self.ilist_addr, self.olist_addr, self.command, self.key_desc_index,
                                self.iv_addr, self.status_addr, self.doorbell_addr, self.doorbell_data))
            else:
                assert(0)
        else:
            logger.info("Entry : %s : T: %d I:%d" % (self.ID(), resp_spec.ring_type, resp_spec.slot_index))
            if (resp_spec.ring_type != req_spec.ring_type):
                assert(0)
            if (resp_spec.slot_index != req_spec.slot_index):
                assert(0)
            logger.info("Field set %s" % resp_spec.WhichOneof("ReqDescrMsg"))

            if (resp_spec.HasField("symm_req_descr")):
                self.ilist_addr = resp_spec.symm_req_descr.ilist_addr
                self.olist_addr = resp_spec.symm_req_descr.olist_addr
                self.command = resp_spec.symm_req_descr.command
                self.key_desc_index = resp_spec.symm_req_descr.key_desc_index
                self.second_key_desc_index = resp_spec.symm_req_descr.second_key_desc_index
                self.iv_addr = resp_spec.symm_req_descr.iv_addr
                self.status_addr = resp_spec.symm_req_descr.status_addr
                self.doorbell_addr = resp_spec.symm_req_descr.doorbell_addr
                self.doorbell_data = resp_spec.symm_req_descr.doorbell_data
                self.salt = resp_spec.symm_req_descr.salt
                self.explicit_iv = resp_spec.symm_req_descr.explicit_iv
                self.barco_status = resp_spec.symm_req_descr.barco_status
                self.header_size = resp_spec.symm_req_descr.header_size
                logger.info("Entry(%s): ila: %x. ola: %x, cmd: %x, kdi: %x, skdi: %x, ia: %x, sa: %x, da: %x, dd: %x" %
                                (self.ID(), self.ilist_addr, self.olist_addr, self.command, self.key_desc_index, self.second_key_desc_index,
                                self.iv_addr, self.status_addr, self.doorbell_addr, self.doorbell_data))
            else:
                assert(0)
        return
    def Show(self):
        logger.info("Entry: %s" % self.ID())
        return



class BRQObject(base.ConfigObjectBase):
    def __init__(self, brq_instance):
        super().__init__()
        self.Clone(Store.templates.Get(brq_instance.defn))
        self.GID("%s" % brq_instance.name)
        self.ring_entries = []
        self.count = brq_instance.count
        self.type = brq_instance.type
        self.defn = brq_instance.defn
        for ring_entry_idx in range(brq_instance.count):
            ring_entry = BRQEntryObject(self.ID(), brq_instance.type, brq_instance.entry_defn, ring_entry_idx)
            self.ring_entries.append(ring_entry)
        Store.objects.SetAll(self.ring_entries)
        return
    def Sync(self):
        rings = []
        rings.append(self)
        if (self.defn == "BRQ_GCM"):
            halapi.GetRingMeta(rings)
            halapi.GetRingEntries(self.ring_entries)
        else:
            halapi.GetBarcoRingMeta(rings)
            halapi.GetBarcoRingEntries(self.ring_entries)
        return
    def Configure(self):
        self.Sync()
        return
    def GetMeta(self):
        # Get ring meta state
        lst = []
        lst.append(self) 
        if (self.defn == "BRQ_GCM"):
              halapi.GetRingMeta(lst)
        else:
            halapi.GetBarcoRingMeta(lst)
        return

    def GetRingEntries(self, indices):
        # Get generic ring entries for given slot indices
        ringentries = []
        for i in indices:
            ringentries.append(self.ring_entries[i])
        if (self.defn == "BRQ_GCM"):
            halapi.GetRingEntries(ringentries)
        else:
            halapi.GetBarcoRingEntries(ringentries)
        return
    def Show(self):
        logger.info("Ring: %s" % (self.ID()))
        for ring_entry in self.ring_entries:
            ring_entry.Show()
        return
    def PrepareHALRequestSpec(self, reqspec):
        if (self.defn == "BRQ_GCM"):
            self.pi = resp_spec.spec.pi
            self.ci = resp_spec.spec.ci
            reqspec.type = self.type
        else:
            #self.pi = resp_spec.pi
            #self.ci = resp_spec.ci
            reqspec.ring_type = self.type
        return
    def ProcessHALResponse(self, req_spec, resp_spec):
        if (self.defn != "BRQ_GCM"):
            self.pi = resp_spec.pi
            self.ci = resp_spec.ci
        return
    def PrepareHALGetRequestSpec(self, reqspec):
        if (self.defn == "BRQ_GCM"):
            reqspec.type = self.type
        else:
            reqspec.ring_type = self.type
        return
    def ProcessHALGetResponse(self, req_spec, resp_spec):
        if (self.defn == "BRQ_GCM"):
            #logger.info("Entry : %s : RI: %d T: %d I:%d" % (self.ID(), resp_spec.spec.key_or_handle.wring_id, resp_spec.spec.type, resp_spec.index))
            self.pi = resp_spec.spec.pi
            self.ci = resp_spec.spec.ci
            logger.info("Entry : %s : pi %s ci %s" % (self.ID(), self.pi, self.ci))
        else:
            logger.info("Entry : %s : T: %d I:%d" % (self.ID(), resp_spec.ring_type, resp_spec.slot_index))
            self.pi = resp_spec.pi
            self.ci = resp_spec.ci
            logger.info("Entry : %s : pi %s ci %s" % (self.ID(), self.pi, self.ci))
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
