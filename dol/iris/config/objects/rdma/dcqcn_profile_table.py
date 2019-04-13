#! /usr/bin/python3
import pdb
import time

import infra.common.objects     as objects
import model_sim.src.model_wrap as model_wrap
from infra.common.logging       import logger
from scapy.all import *
from infra.common.glopts import GlobalOptions

class RdmaDcqcnProfile(Packet):
    name = "RdmaDcqcnProfile"
    fields_desc = [
        BitField("pad", 0, 64),
        ByteField("np_incp_802p_prio", 0),
        ByteField("np_cnp_dscp", 0),
        BitField("np_rsvd", 0, 48),
        ShortField("rp_initial_alpha_value", 0),
        ShortField("rp_dce_tcp_g", 0),
        IntField("rp_dce_tcp_rtt", 0),
        IntField("rp_rate_reduce_monitor_period", 0),
        IntField("rp_rate_to_set_on_first_cnp", 0),
        IntField("rp_min_rate", 0),
        ByteField("rp_gd", 0),
        ByteField("rp_dec_fac", 0),
        BitField("rp_clamp_flags", 0, 8),
        ByteField("rp_threshold", 0),
        IntField("rp_time_reset", 0),
        IntField("rp_byte_reset", 0),
        IntField("rp_ai_rate", 0),
        IntField("rp_hai_rate", 0),
        BitField("rp_rsvd", 0, 64),
    ]

class RdmaDcqcnProfileObject(object):
    def __init__(self, lif, key):
        self.size = len(RdmaDcqcnProfile())
        self.addr = lif.rdma_dcqcn_profile_base_addr + key * self.size
        self.Read()

    def Write(self):
        if (GlobalOptions.dryrun): return
        logger.info("Writing RdmaDcqcnProfile @0x%x size: %d" % (self.addr, self.size))
        model_wrap.write_mem_pcie(self.addr, bytes(self.data), len(self.data))
        self.Read()

    def WriteWithDelay(self):
        if (GlobalOptions.dryrun): return
        logger.info("Writing RdmaDcqcnProfile @0x%x size: %d with delay" % (self.addr, self.size))
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
            logger.info("RdmaDcqcnProfile Write @0x%x size: %d completed after %d secs" % (self.addr, self.size, count))
        else:
            self.Read()

    def Read(self):
        if (GlobalOptions.dryrun):
            data = bytes(self.size)
            self.data = RdmaDcqcnProfile(data)
            return
        self.data = RdmaDcqcnProfile(model_wrap.read_mem(self.addr, self.size))
        logger.info("Read RdmaDcqcnProfile @ 0x%x size: %d: " % (self.addr, self.size))
        logger.ShowScapyObject(self.data)

    def Show(self, lgh = logger):
        lgh.ShowScapyObject(self.data) 
