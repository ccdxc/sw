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
        ByteField("rp_min_dec_fac", 0),
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

    def Init(self, spec):
        self.np_incp_802p_prio = spec.np_incp_802p_prio if hasattr(spec, "np_incp_802p_prio") else 0
        self.np_cnp_dscp = spec.np_cnp_dscp if hasattr(spec, "np_cnp_dscp") else 0
        self.rp_initial_alpha_value  = spec.rp_initial_alpha_value if hasattr(spec, "rp_initial_alpha_value") else 0
        self.rp_dce_tcp_g = spec.rp_dce_tcp_g if hasattr(spec, "rp_dce_tcp_g") else 0
        self.rp_dce_tcp_rtt = spec.rp_dce_tcp_rtt if hasattr(spec, "rp_dce_tcp_rtt") else 0
        self.rp_rate_reduce_monitor_period = spec.rp_rate_reduce_monitor_period if hasattr(spec, "rp_rate_reduce_monitor_period") else 0
        self.rp_rate_to_set_on_first_cnp = spec.rp_rate_to_set_on_first_cnp if hasattr(spec, "rp_rate_to_set_on_first_cnp") else 0
        self.rp_min_rate = spec.rp_min_rate if hasattr(spec, "rp_min_rate") else 0
        self.rp_gd = spec.rp_gd if hasattr(spec, "rp_gd") else 0
        self.rp_min_dec_fac = spec.rp_min_dec_fac if hasattr(spec, "rp_min_dec_fac") else 0
        self.rp_clamp_flags = spec.rp_clamp_flags if hasattr(spec, "rp_clamp_flags") else 0
        self.rp_threshold = spec.rp_threshold if hasattr(spec, "rp_threshold") else 0
        self.rp_time_reset = spec.rp_time_reset if hasattr(spec, "rp_time_reset") else 0
        self.rp_byte_reset = spec.rp_byte_reset if hasattr(spec, "rp_byte_reset") else 0
        self.rp_ai_rate = spec.rp_ai_rate if hasattr(spec, "rp_ai_rate") else 0
        self.rp_hai_rate = spec.rp_hai_rate if hasattr(spec, "rp_hai_rate") else 0

        self.data = RdmaDcqcnProfile(np_incp_802p_prio = self.np_incp_802p_prio,
                        np_cnp_dscp = self.np_cnp_dscp, rp_initial_alpha_value = self.rp_initial_alpha_value,
                        rp_dce_tcp_g = self.rp_dce_tcp_g, rp_dce_tcp_rtt = self.rp_dce_tcp_rtt,
                        rp_rate_reduce_monitor_period = self.rp_rate_reduce_monitor_period,
                        rp_rate_to_set_on_first_cnp = self.rp_rate_to_set_on_first_cnp,
                        rp_min_rate = self.rp_min_rate, rp_gd = self.rp_gd,
                        rp_min_dec_fac = self.rp_min_dec_fac, rp_clamp_flags = self.rp_clamp_flags,
                        rp_threshold = self.rp_threshold, rp_time_reset = self.rp_time_reset,
                        rp_byte_reset = self.rp_byte_reset, rp_ai_rate = self.rp_ai_rate,
                        rp_hai_rate = self.rp_hai_rate)

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

class RdmaDcqcnProfileObjectHelper:
    def __init__(self):
        self.dcqcn_objects = []

    def Generate(self, lif, spec):
        self.lif = lif
        dcqcn_spec = spec.default_profile
        dcqcn = RdmaDcqcnProfileObject(lif, dcqcn_spec.id)
        dcqcn.Init(dcqcn_spec)
        self.dcqcn_objects.append(dcqcn)

    def Configure(self):
        logger.info("Configuring %d dcqcn configs. " % (len(self.dcqcn_objects)))

        if (GlobalOptions.dryrun): return

        # Avoid using adminAPI as that path is already tested in a test
        for dcqcn in self.dcqcn_objects:
            dcqcn.Write()
