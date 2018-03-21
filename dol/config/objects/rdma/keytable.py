#! /usr/bin/python3
import pdb
import time

import infra.common.objects     as objects
import model_sim.src.model_wrap as model_wrap
from infra.common.logging       import logger
from scapy.all import *
from infra.common.glopts import GlobalOptions

class RdmaKeyTableEntry(Packet):
    name = "RdmaKeyTableEntry"
    fields_desc = [
        ByteField("user_key", 0),
        BitField("state", 0, 4),
        BitField("type", 0, 4),
        ByteField("acc_ctrl", 0),
        ByteField("log_page_size", 0),
        IntField("len", 0),
        LongField("base_va", 0),
        IntField("pt_base", 0),
        IntField("pd", 0),
        BitField("rsvd", 0, 19),
        BitField("override_lif_vld", 0, 1),
        BitField("override_lif", 0, 12),
        ByteField("flags", 0),
        X3BytesField("qp", 0),
    ]

class RdmaKeyTableEntryObject(object):
    def __init__(self, lif, key):
        self.size = len(RdmaKeyTableEntry())
        self.addr = lif.rdma_kt_base_addr + key * self.size
        self.Read()

    def Write(self):
        if (GlobalOptions.dryrun): return
        logger.info("Writing KeyTableEntry @0x%x size: %d" % (self.addr, self.size))
        model_wrap.write_mem_pcie(self.addr, bytes(self.data), len(self.data))
        self.Read()

    def WriteWithDelay(self):
        if (GlobalOptions.dryrun): return
        logger.info("Writing KeyTableEntry @0x%x size: %d with delay" % (self.addr, self.size))
        model_wrap.write_mem_pcie(self.addr, bytes(self.data), len(self.data))
        if GlobalOptions.rtl:
            time.sleep(10)
        self.Read()

    def Read(self):
        if (GlobalOptions.dryrun):
            data = bytes(self.size)
            self.data = RdmaKeyTableEntry(data)
            return
        self.data = RdmaKeyTableEntry(model_wrap.read_mem(self.addr, self.size))
        logger.info("Read KeyTableEntry @ 0x%x size: %d: " % (self.addr, self.size))
        logger.ShowScapyObject(self.data)

    def Show(self, lgh = logger):
        lgh.ShowScapyObject(self.data) 


