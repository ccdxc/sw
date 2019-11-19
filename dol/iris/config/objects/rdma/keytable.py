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
        XByteField("acc_ctrl", 0),
        ByteField("log_page_size", 0),
        LongField("len", 0),
        LongField("base_va", 0),
        IntField("pt_base", 0),
        IntField("pd", 0),
        BitField("host_addr", 0, 1),
        BitField("override_lif_vld", 0, 1),
        BitField("override_lif", 0, 12),
        BitField("is_phy_addr", 0, 1),
        BitField("rsvd1", 0, 17),
        XByteField("flags", 0),
        X3BytesField("qp", 0),
        IntField("mr_l_key", 0),
        IntField("mr_cookie", 0),
        IntField("num_pt_entries_rsvd", 0),
        LongField("phy_base_addr", 0),
        BitField("rsvd2", 0, 64)
    ]

class RdmaKeyTableEntryObject(object):
    def __init__(self, lif, key):
        self.size = len(RdmaKeyTableEntry())
        self.addr = lif.rdma_kt_base_addr + (key >> 8) * self.size
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
        # On RTL, write is asyncronous and taking long time to complete
        # Wait until the write succeed otherwise tests may fail in RTL (timing)
        # Read(synchrouns) in loop and wait until the read data is same as what was inteded to write
        wdata = self.data[0:len(self.data)]
        if GlobalOptions.rtl:
            count = 1
            while True:
                self.Read()
                rdata = self.data[0:len(self.data)]
                if rdata == wdata:
                    break
                # Read data is not same as we wrote, so sleep for 1 sec and try again
                time.sleep(1)
                count = count + 1
            logger.info("KeyTableEntry Write @0x%x size: %d completed after %d secs" % (self.addr, self.size, count))
        else:
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


